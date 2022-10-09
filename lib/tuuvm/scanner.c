#include "tuuvm/scanner.h"
#include "tuuvm/arrayList.h"
#include "tuuvm/sourceCode.h"
#include "tuuvm/sourcePosition.h"
#include "tuuvm/token.h"

typedef struct tuuvm_scannerState_s
{
    tuuvm_tuple_t sourceCode;
    const uint8_t *text;
    size_t size;
    size_t position;
} tuuvm_scannerState_t;

static int tuuvm_scanner_lookAt(tuuvm_scannerState_t *state, size_t offset)
{
    return state->position + offset < state->size ? state->text[state->position + offset] : -1;
}

static void tuuvm_scanner_emitTokenForStateRange(tuuvm_context_t *context, tuuvm_scannerState_t *startState, tuuvm_scannerState_t *endState, tuuvm_tokenKind_t kind, tuuvm_tuple_t outTokenList)
{
    tuuvm_tuple_t sourcePosition = tuuvm_sourcePosition_createWithIndices(context, startState->sourceCode, startState->position, endState->position);
    tuuvm_tuple_t token = tuuvm_token_createWithKind(context, kind, sourcePosition, TUUVM_NULL_TUPLE);
    tuuvm_arrayList_add(context, outTokenList, token);
}

static void tuuvm_scanner_skipWhite(tuuvm_scannerState_t *state)
{
    while(state->position < state->size && state->text[state->position] <= ' ')
        ++state->position;
}

static void tuuvm_scanner_skipSingleLineComment(tuuvm_scannerState_t *state)
{
    state->position += 2;
    while(state->position < state->size)
    {
        uint8_t c = state->text[state->position];
        ++state->position;
        if(c == '\n' || c == '\r')
            break;
    }
}

static bool tuuvm_scanner_skipMultiLineComment(tuuvm_context_t *context, tuuvm_scannerState_t *state, tuuvm_tuple_t outTokenList)
{
    if(tuuvm_scanner_lookAt(state, 0) != '#' && tuuvm_scanner_lookAt(state, 1) != '*')
        return true;

    tuuvm_scannerState_t startState = *state;
    state->position += 2;

    while(state->position < state->size)
    {
        if(tuuvm_scanner_lookAt(state, 0) == '*' && tuuvm_scanner_lookAt(state, 1) == '#')
        {
            state->position += 2;
            return true;
        }

        ++state->position;
    }

    tuuvm_scanner_emitTokenForStateRange(context, &startState, state, TUUVM_TOKEN_KIND_ERROR, outTokenList);
    return false;
}

static bool tuuvm_scanner_skipWhiteAndComments(tuuvm_context_t *context, tuuvm_scannerState_t *state, tuuvm_tuple_t outTokenList)
{
    bool hasSeenComment = false;

    do
    {
        hasSeenComment = false;
        tuuvm_scanner_skipWhite(state);

        // Skip the two types of different comments.
        if(tuuvm_scanner_lookAt(state, 0) == '#')
        {
            int commentCharType = tuuvm_scanner_lookAt(state, 1);
            if(commentCharType == '#')
            {
                tuuvm_scanner_skipSingleLineComment(state);
                hasSeenComment = true;
            }
            else if(commentCharType == '*')
            {
                if(!tuuvm_scanner_skipMultiLineComment(context, state, outTokenList))
                    return false;
                hasSeenComment = true;
            }
        }
    } while(hasSeenComment);

    return true;
}

static inline bool tuuvm_scanner_isIdentifierStart(int c)
{
    return ('A' <= c && c <= 'Z') ||
        ('a' <= c && c <= 'z') ||
        '_' == c;
}

static inline bool tuuvm_scanner_isDigit(int c)
{
    return '0' <= c && c <= '9';
}

static inline bool tuuvm_scanner_isSign(int c)
{
    return '+' == c || c == '-';
}

static inline bool tuuvm_scanner_isIdentifierMiddle(int c)
{
    return tuuvm_scanner_isIdentifierStart(c) || tuuvm_scanner_isDigit(c);
}

static bool tuuvm_scanner_advanceKeyword(tuuvm_scannerState_t *state)
{
    if(!tuuvm_scanner_isIdentifierStart(tuuvm_scanner_lookAt(state, 0)))
        return false;

    tuuvm_scannerState_t endState = *state;
    while(tuuvm_scanner_isIdentifierMiddle(tuuvm_scanner_lookAt(&endState, 0)))
        ++endState.position;

    if(tuuvm_scanner_lookAt(&endState, 0) == ':')
    {
        ++endState.position;
        *state = endState;
        return true;
    }

    return false;
}

static bool tuuvm_scanner_scanNextTokenInto(tuuvm_context_t *context, tuuvm_scannerState_t *state, tuuvm_tuple_t outTokenList)
{
    // Skip the whitespaces and the comments preceeding the token.
    if(!tuuvm_scanner_skipWhiteAndComments(context, state, outTokenList))
        return false;

    // Fetch the first token character.
    int c = tuuvm_scanner_lookAt(state, 0);

    // Is this the end?
    if(c < 0)
        return false;

    // Make a copy of the scanner initial state.
    tuuvm_scannerState_t tokenStartState = *state;

    // Is this an identifier?
    if(tuuvm_scanner_isIdentifierStart(c))
    {
        ++state->position;
        while(tuuvm_scanner_isIdentifierMiddle(tuuvm_scanner_lookAt(state, 0)))
            ++state->position;

        if(':' == tuuvm_scanner_lookAt(state, 0))
        {
            ++state->position;

            bool isMultikeyword = false;
            while(tuuvm_scanner_advanceKeyword(state))
                isMultikeyword = true;

            tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, isMultikeyword ? TUUVM_TOKEN_KIND_MULTI_KEYWORD : TUUVM_TOKEN_KIND_KEYWORD, outTokenList);
            return true;
        }

        tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_IDENTIFIER, outTokenList);
        return true;
    }

    // Is this a number?
    if(tuuvm_scanner_isDigit(c) ||
        tuuvm_scanner_isSign(c) && tuuvm_scanner_isDigit(tuuvm_scanner_lookAt(state, 1)))
    {
        ++state->position;
        while(tuuvm_scanner_isDigit(tuuvm_scanner_lookAt(state, 0)))
            ++state->position;

        // Look for a float
        if(tuuvm_scanner_lookAt(state, 0) == '.')
        {
            if(tuuvm_scanner_isDigit(tuuvm_scanner_lookAt(state, 1)) ||
                tuuvm_scanner_isSign(tuuvm_scanner_lookAt(state, 1)) && tuuvm_scanner_isDigit(tuuvm_scanner_lookAt(state, 2)))
            {
                ++state->position; // Dot
                if(tuuvm_scanner_isSign(tuuvm_scanner_lookAt(state, 1)))
                    ++state->position; // sign
                ++state->position; // First decimal

                // Remaining fractional part
                while(tuuvm_scanner_isDigit(tuuvm_scanner_lookAt(state, 0)))
                    ++state->position;

                tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_FLOAT, outTokenList);
                return true;
            }
        }

        tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_INTEGER, outTokenList);
        return true;
    }

    if(c == '#')
    {
        int c1 = tuuvm_scanner_lookAt(state, 1);

        // Identifier symbols
        if(tuuvm_scanner_isIdentifierStart(c1))
        {
            state->position += 2;
            while(tuuvm_scanner_isIdentifierMiddle(tuuvm_scanner_lookAt(state, 0)))
                ++state->position;

            // Accept keyword symbols.
            if(':' == tuuvm_scanner_lookAt(state, 0))
            {
                ++state->position;
                while(tuuvm_scanner_advanceKeyword(state))
                    ;
            }

            tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_SYMBOL, outTokenList);
            return true;
        }

        switch(c1)
        {
        case '[':
            state->position += 2;
            tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_BYTE_ARRAY_START, outTokenList);
            return true;
        case '{':
            state->position += 2;
            tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_DICTIONARY_START, outTokenList);
            return true;
        case '(':
            state->position += 2;
            tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_LITERAL_ARRAY_START, outTokenList);
            return true;
        default:
            break;
        }
    }

    switch(c)
    {
    case '(':
        ++state->position;
        tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_LPARENT, outTokenList);
        return true;
    case ')':
        ++state->position;
        tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_RPARENT, outTokenList);
        return true;
    case '[':
        ++state->position;
        tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_LBRACKET, outTokenList);
        return true;
    case ']':
        ++state->position;
        tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_RBRACKET, outTokenList);
        return true;
    case '{':
        ++state->position;
        tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_LCBRACKET, outTokenList);
        return true;
    case '}':
        ++state->position;
        tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_RCBRACKET, outTokenList);
        return true;

    case ':':
        ++state->position;
        if(':' == tuuvm_scanner_lookAt(state, 0))
        {
            ++state->position;
            tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_COLON_COLON, outTokenList);
            return true;
        }

        tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_COLON, outTokenList);
        return true;

    case '|':
        ++state->position;
        tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_BAR, outTokenList);
        return true;

    case '`':
        {
            switch(tuuvm_scanner_lookAt(state, 1))
            {
            case '\'':
                state->position += 2;
                tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_QUOTE, outTokenList);
                return true;
            case '`':
                state->position += 2;
                tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_QUASI_QUOTE, outTokenList);
                return true;
            case ',':
                state->position += 2;
                tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_QUASI_UNQUOTE, outTokenList);
                return true;
            case '@':
                state->position += 2;
                tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_SPLICE, outTokenList);
                return true;
            default:
                break;
            }
        }
        break;
    default:
        break;
    }

    return false;
}

TUUVM_API tuuvm_tuple_t tuuvm_scanner_scan(tuuvm_context_t *context, tuuvm_tuple_t sourceCode)
{
    tuuvm_tuple_t tokenList = tuuvm_arrayList_create(context);

    if(tuuvm_tuple_isNonNullPointer(sourceCode))
    {
        tuuvm_sourceCode_t *sourceCodeObject = (tuuvm_sourceCode_t*)sourceCode;
        if(tuuvm_tuple_isBytes(sourceCodeObject->text))
        {
            tuuvm_scannerState_t scannerState = {
                .sourceCode = sourceCode,
                .text = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(sourceCodeObject->text)->bytes,
                .size = tuuvm_tuple_getSizeInBytes(sourceCodeObject->text),
                .position = 0
            };

            while(scannerState.position < scannerState.size)
            {
                if(!tuuvm_scanner_scanNextTokenInto(context, &scannerState, tokenList))
                    break;
            }
        }
    }

    return tuuvm_arrayList_asArraySlice(context, tokenList);
}

TUUVM_API tuuvm_tuple_t tuuvm_scanner_scanCString(tuuvm_context_t *context, const char *sourceCodeText, const char *sourceCodeName)
{
    return tuuvm_scanner_scan(context, tuuvm_sourceCode_createWithCStrings(context, sourceCodeText, sourceCodeName));
}
