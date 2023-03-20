#include "tuuvm/scanner.h"
#include "tuuvm/arrayList.h"
#include "tuuvm/assert.h"
#include "tuuvm/gc.h"
#include "tuuvm/integer.h"
#include "tuuvm/float.h"
#include "tuuvm/sourceCode.h"
#include "tuuvm/sourcePosition.h"
#include "tuuvm/string.h"
#include "tuuvm/token.h"
#include <stdlib.h>
#include <string.h>

typedef struct tuuvm_scannerState_s
{
    tuuvm_tuple_t sourceCode;
    const uint8_t *text;
    size_t size;
    size_t position;
} tuuvm_scannerState_t;

typedef tuuvm_tuple_t (*tuuvm_scanner_tokenConversionFunction_t)(tuuvm_context_t *context, size_t stringSize, const uint8_t *string);

static inline bool tuuvm_scanner_isOperatorCharacter(int c)
{
    switch(c)
    {
    case '+':
    case '-':
    case '/':
    case '\\':
    case '*':
    case '~':
    case '<':
    case '>':
    case '=':
    case '@':
    case '%':
    case '|':
    case '&':
    case '?':
    case '!':
    case '^':
        return true;
    default:
        return false;
    }
}

static tuuvm_tuple_t tuuvm_scanner_tokenAsSymbol(tuuvm_context_t *context, size_t stringSize, const uint8_t *string)
{
    return tuuvm_symbol_internWithString(context, stringSize, (const char*)string);
}

static tuuvm_tuple_t tuuvm_scanner_tokenAsSymbolWithoutPrefix(tuuvm_context_t *context, size_t stringSize, const uint8_t *string)
{
    return tuuvm_symbol_internWithString(context, stringSize - 1, (const char*)(string + 1));
}

static tuuvm_tuple_t tuuvm_scanner_tokenAsInteger(tuuvm_context_t *context, size_t stringSize, const uint8_t *string)
{
    return tuuvm_integer_parseString(context, stringSize, (const char*)string);
}

static tuuvm_tuple_t tuuvm_scanner_tokenAsFloat(tuuvm_context_t *context, size_t stringSize, const uint8_t *string)
{
    return tuuvm_float64_parseString(context, stringSize, (const char*)string);
}

static tuuvm_tuple_t tuuvm_scanner_tokenAsCharacter(tuuvm_context_t *context, size_t stringSize, const uint8_t *string)
{
    TUUVM_ASSERT(stringSize >= 2);
    if(stringSize == 2)
    {
        return tuuvm_tuple_char32_encode(context, 0);
    }
    else if(stringSize >= 4 && string[1] == '\\')
    {
        switch(string[2])
        {
        case 't': return tuuvm_tuple_char32_encode(context, '\t');
        case 'r': return tuuvm_tuple_char32_encode(context, '\r');
        case 'n': return tuuvm_tuple_char32_encode(context, '\n');
        default: return tuuvm_tuple_char32_encode(context, string[2]);
        }
    }
    else
    {
        return tuuvm_tuple_char32_encode(context, string[1]);
    }
}

static size_t countSizeRequiredForString(size_t stringSize, const uint8_t *string)
{
    size_t result = 0;
    for(size_t i = 0; i < stringSize; ++i)
    {
        uint8_t c = string[i];
        if(c == '\\')
            ++i;
        ++result;
    }

    return result;
}

static tuuvm_tuple_t tuuvm_scanner_tokenAsString(tuuvm_context_t *context, size_t stringSize, const uint8_t *string)
{
    TUUVM_ASSERT(stringSize >= 2);

    size_t requiredStringSize = countSizeRequiredForString(stringSize - 2, string + 1);
    tuuvm_tuple_t allocatedString = tuuvm_string_createEmptyWithSize(context, requiredStringSize);
    uint8_t *destination = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(allocatedString)->bytes;
    for(size_t i = 1; i < stringSize - 1; ++i)
    {
        uint8_t c = string[i];
        if(c == '\\')
        {
            ++i;
            c = string[i];
            switch(c)
            {
            case 't':
                *destination++ = '\t';
                break;
            case 'r':
                *destination++ = '\r';
                break;
            case 'n':
                *destination++ = '\n';
                break;
            default:
                break;
            }
        }
        else
        {
            *destination++ = c;
        }
    }

    return allocatedString;
}

static tuuvm_tuple_t tuuvm_scanner_tokenAsSymbolString(tuuvm_context_t *context, size_t stringSize, const uint8_t *string)
{
    TUUVM_ASSERT(stringSize >= 3);

    size_t requiredStringSize = countSizeRequiredForString(stringSize - 3, string + 2);
    tuuvm_tuple_t allocatedString = tuuvm_string_createEmptyWithSize(context, requiredStringSize);
    uint8_t *destination = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(allocatedString)->bytes;
    for(size_t i = 2; i < stringSize - 1; ++i)
    {
        uint8_t c = string[i];
        if(c == '\\')
        {
            ++i;
            c = string[i];
            switch(c)
            {
            case 't':
                *destination++ = '\t';
                break;
            case 'r':
                *destination++ = '\r';
                break;
            case 'n':
                *destination++ = '\n';
                break;
            default:
                break;
            }
        }
        else
        {
            *destination++ = c;
        }
    }

    return tuuvm_symbol_internFromTuple(context, allocatedString);
}

static tuuvm_tuple_t tuuvm_scanner_incompleteMultilineCommentError(tuuvm_context_t *context, size_t stringSize, const uint8_t *string)
{
    (void)stringSize;
    (void)string;
    return tuuvm_symbol_internWithCString(context, "Incomplete multiline comment.");
}

static tuuvm_tuple_t tuuvm_scanner_unrecognizedTokenError(tuuvm_context_t *context, size_t stringSize, const uint8_t *string)
{
    (void)stringSize;
    (void)string;
    return tuuvm_symbol_internWithCString(context, "Unrecognized token.");
}

static tuuvm_tuple_t tuuvm_scanner_incompleteCharacterError(tuuvm_context_t *context, size_t stringSize, const uint8_t *string)
{
    (void)stringSize;
    (void)string;
    return tuuvm_symbol_internWithCString(context, "Incomplete character.");
}

static tuuvm_tuple_t tuuvm_scanner_incompleteStringError(tuuvm_context_t *context, size_t stringSize, const uint8_t *string)
{
    (void)stringSize;
    (void)string;
    return tuuvm_symbol_internWithCString(context, "Incomplete string.");
}

static int tuuvm_scanner_lookAt(tuuvm_scannerState_t *state, size_t offset)
{
    return state->position + offset < state->size ? state->text[state->position + offset] : -1;
}

static void tuuvm_scanner_emitTokenForStateRange(tuuvm_context_t *context, tuuvm_scannerState_t *startState, tuuvm_scannerState_t *endState, tuuvm_tokenKind_t kind, tuuvm_scanner_tokenConversionFunction_t tokenConversionFunction, tuuvm_tuple_t outTokenList)
{
    tuuvm_tuple_t sourcePosition = tuuvm_sourcePosition_createWithIndices(context, startState->sourceCode, startState->position, endState->position);
    tuuvm_tuple_t value = tokenConversionFunction(context, endState->position - startState->position, startState->text + startState->position);
    tuuvm_tuple_t token = tuuvm_token_createWithKind(context, kind, sourcePosition, value);
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

    tuuvm_scanner_emitTokenForStateRange(context, &startState, state, TUUVM_TOKEN_KIND_ERROR, tuuvm_scanner_incompleteMultilineCommentError, outTokenList);
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

static inline bool tuuvm_scanner_isAlpha(int c)
{
    return ('A' <= c && c <= 'Z') ||
        ('a' <= c && c <= 'z');
}

static inline bool tuuvm_scanner_isDigit(int c)
{
    return '0' <= c && c <= '9';
}

static inline bool tuuvm_scanner_isDigitOrUnderscore(int c)
{
    return ('0' <= c && c <= '9') || c == '_';
}

static inline bool tuuvm_scanner_isAlphanumericOrUnderscore(int c)
{
    return tuuvm_scanner_isDigit(c) || tuuvm_scanner_isAlpha(c) || c == '_';
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

        // Chop the scope resolutions.
        while(':' == tuuvm_scanner_lookAt(state, 0) &&
            ':' == tuuvm_scanner_lookAt(state, 1) &&
            tuuvm_scanner_isIdentifierStart(tuuvm_scanner_lookAt(state, 2)))
        {
            state->position += 3;
            while(tuuvm_scanner_isIdentifierMiddle(tuuvm_scanner_lookAt(state, 0)))
                ++state->position;
        }

        // Operator with scope.
        if(':' == tuuvm_scanner_lookAt(state, 0) &&
            ':' == tuuvm_scanner_lookAt(state, 1) &&
            tuuvm_scanner_isOperatorCharacter(tuuvm_scanner_lookAt(state, 2)))
        {
            state->position += 3;
            while(tuuvm_scanner_isOperatorCharacter(tuuvm_scanner_lookAt(state, 0)))
                ++state->position;
            tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_OPERATOR, tuuvm_scanner_tokenAsSymbol, outTokenList);
            return true;
        }

        if(':' == tuuvm_scanner_lookAt(state, 0))
        {
            ++state->position;

            bool isMultikeyword = false;
            while(tuuvm_scanner_advanceKeyword(state))
                isMultikeyword = true;

            tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, isMultikeyword ? TUUVM_TOKEN_KIND_MULTI_KEYWORD : TUUVM_TOKEN_KIND_KEYWORD, tuuvm_scanner_tokenAsSymbol, outTokenList);
            return true;
        }

        tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_IDENTIFIER, tuuvm_scanner_tokenAsSymbol, outTokenList);
        return true;
    }

    // Is this a number?
    if(tuuvm_scanner_isDigit(c) ||
        (tuuvm_scanner_isSign(c) && tuuvm_scanner_isDigit(tuuvm_scanner_lookAt(state, 1))))
    {
        ++state->position;
        while(tuuvm_scanner_isDigitOrUnderscore(tuuvm_scanner_lookAt(state, 0)))
            ++state->position;

        // Did we parse the radix?
        if(tuuvm_scanner_lookAt(state, 0) == 'r' || tuuvm_scanner_lookAt(state, 0) == 'R')
        {
            ++state->position;
            while(tuuvm_scanner_isAlphanumericOrUnderscore(tuuvm_scanner_lookAt(state, 0)))
                ++state->position;

            tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_INTEGER, tuuvm_scanner_tokenAsInteger, outTokenList);
            return true;
        }

        // Look for a float
        if(tuuvm_scanner_lookAt(state, 0) == '.')
        {
            if(tuuvm_scanner_isDigit(tuuvm_scanner_lookAt(state, 1)) ||
                (tuuvm_scanner_isSign(tuuvm_scanner_lookAt(state, 1)) && tuuvm_scanner_isDigit(tuuvm_scanner_lookAt(state, 2))))
            {
                ++state->position; // Dot
                if(tuuvm_scanner_isSign(tuuvm_scanner_lookAt(state, 1)))
                    ++state->position; // sign
                ++state->position; // First decimal

                // Remaining fractional part
                while(tuuvm_scanner_isDigitOrUnderscore(tuuvm_scanner_lookAt(state, 0)))
                    ++state->position;

                // Exponent.
                if(tuuvm_scanner_lookAt(state, 0) == 'e' || tuuvm_scanner_lookAt(state, 0) == 'E')
                {
                    ++state->position;
                    if(tuuvm_scanner_lookAt(state, 0) == '+' || tuuvm_scanner_lookAt(state, 0) == '-')
                        --state->position;

                    while(tuuvm_scanner_isDigitOrUnderscore(tuuvm_scanner_lookAt(state, 0)))
                        ++state->position;
                }

                tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_FLOAT, tuuvm_scanner_tokenAsFloat, outTokenList);
                return true;
            }
        }

        tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_INTEGER, tuuvm_scanner_tokenAsInteger, outTokenList);
        return true;
    }

    // Strings
    if(c == '"')
    {
        ++state->position;
        while((c = tuuvm_scanner_lookAt(state, 0)) != '"' && c >= 0)
        {
            if(c == '\\')
            {
                ++state->position;
                if(tuuvm_scanner_lookAt(state, 0) < 0)
                {
                    tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_ERROR, tuuvm_scanner_incompleteStringError, outTokenList);
                    return true;
                }
            }
            else
            {
                ++state->position;
            }
        }

        if(tuuvm_scanner_lookAt(state, 0) != '"')
        {
            tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_ERROR, tuuvm_scanner_incompleteStringError, outTokenList);
            return true;
        }

        ++state->position;
        tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_STRING, tuuvm_scanner_tokenAsString, outTokenList);
        return true;
    }

    // Characters
    if(c == '\'')
    {
        ++state->position;
        while((c = tuuvm_scanner_lookAt(state, 0)) != '\'' && c >= 0)
        {
            if(c == '\\')
            {
                ++state->position;
                if(tuuvm_scanner_lookAt(state, 0) < 0)
                {
                    tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_ERROR, tuuvm_scanner_incompleteCharacterError, outTokenList);
                    return true;
                }
            }
            else
            {
                ++state->position;
            }
        }

        if(tuuvm_scanner_lookAt(state, 0) != '\'')
        {
            tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_ERROR, tuuvm_scanner_incompleteCharacterError, outTokenList);
            return true;
        }

        ++state->position;
        tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_CHARACTER, tuuvm_scanner_tokenAsCharacter, outTokenList);
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

            // Chop the scope resolutions.
            while(':' == tuuvm_scanner_lookAt(state, 0) &&
                ':' == tuuvm_scanner_lookAt(state, 1) &&
                tuuvm_scanner_isIdentifierStart(tuuvm_scanner_lookAt(state, 2)))
            {
                state->position += 3;
                while(tuuvm_scanner_isIdentifierMiddle(tuuvm_scanner_lookAt(state, 0)))
                    ++state->position;
            }

            // Accept keyword symbols.
            if(':' == tuuvm_scanner_lookAt(state, 0))
            {
                ++state->position;
                while(tuuvm_scanner_advanceKeyword(state))
                    ;
            }

            tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_SYMBOL, tuuvm_scanner_tokenAsSymbolWithoutPrefix, outTokenList);
            return true;
        }

        // Operators
        if(tuuvm_scanner_isOperatorCharacter(c1))
        {
            state->position += 2;
            while(tuuvm_scanner_isOperatorCharacter(tuuvm_scanner_lookAt(state, 0)))
                ++state->position;
            tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_SYMBOL, tuuvm_scanner_tokenAsSymbolWithoutPrefix, outTokenList);
            return true;
        }

        // StringSymbol string
        if(c1 == '"')
        {
            state->position += 2;
            while((c = tuuvm_scanner_lookAt(state, 0)) != '"' && c >= 0)
            {
                if(c == '\\')
                {
                    ++state->position;
                    if(tuuvm_scanner_lookAt(state, 0) < 0)
                    {
                        tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_ERROR, tuuvm_scanner_incompleteStringError, outTokenList);
                        return true;
                    }
                }
                else
                {
                    ++state->position;
                }
            }

            if(tuuvm_scanner_lookAt(state, 0) != '"')
            {
                tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_ERROR, tuuvm_scanner_incompleteStringError, outTokenList);
                return true;
            }

            ++state->position;
            tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_SYMBOL, tuuvm_scanner_tokenAsSymbolString, outTokenList);
            return true;
        }

        switch(c1)
        {
        case '[':
            state->position += 2;
            tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_BYTE_ARRAY_START, tuuvm_scanner_tokenAsSymbol, outTokenList);
            return true;
        case '{':
            state->position += 2;
            tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_DICTIONARY_START, tuuvm_scanner_tokenAsSymbol, outTokenList);
            return true;
        case '(':
            state->position += 2;
            tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_LITERAL_ARRAY_START, tuuvm_scanner_tokenAsSymbol, outTokenList);
            return true;
        default:
            break;
        }
    }

    switch(c)
    {
    case '(':
        ++state->position;
        tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_LPARENT, tuuvm_scanner_tokenAsSymbol, outTokenList);
        return true;
    case ')':
        ++state->position;
        tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_RPARENT, tuuvm_scanner_tokenAsSymbol, outTokenList);
        return true;
    case '[':
        ++state->position;
        tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_LBRACKET, tuuvm_scanner_tokenAsSymbol, outTokenList);
        return true;
    case ']':
        ++state->position;
        tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_RBRACKET, tuuvm_scanner_tokenAsSymbol, outTokenList);
        return true;
    case '{':
        ++state->position;
        tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_LCBRACKET, tuuvm_scanner_tokenAsSymbol, outTokenList);
        return true;
    case '}':
        ++state->position;
        tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_RCBRACKET, tuuvm_scanner_tokenAsSymbol, outTokenList);
        return true;

    case ';':
        ++state->position;
        tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_SEMICOLON, tuuvm_scanner_tokenAsSymbol, outTokenList);
        return true;

    case ',':
        ++state->position;
        tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_COMMA, tuuvm_scanner_tokenAsSymbol, outTokenList);
        return true;

    case '.':
        ++state->position;
        if('.' == tuuvm_scanner_lookAt(state, 0) && '.' == tuuvm_scanner_lookAt(state, 1))
        {
            state->position += 2;
            tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_ELLIPSIS, tuuvm_scanner_tokenAsSymbol, outTokenList);
            return true;
        }

        tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_DOT, tuuvm_scanner_tokenAsSymbol, outTokenList);
        return true;
    case ':':
        ++state->position;
        if(':' == tuuvm_scanner_lookAt(state, 0))
        {
            ++state->position;
            tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_COLON_COLON, tuuvm_scanner_tokenAsSymbol, outTokenList);
            return true;
        }
        else if('=' == tuuvm_scanner_lookAt(state, 0))
        {
            ++state->position;
            tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_ASSIGNMENT, tuuvm_scanner_tokenAsSymbol, outTokenList);
            return true;
        }

        tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_COLON, tuuvm_scanner_tokenAsSymbol, outTokenList);
        return true;

    case '`':
        {
            switch(tuuvm_scanner_lookAt(state, 1))
            {
            case '\'':
                state->position += 2;
                tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_QUOTE, tuuvm_scanner_tokenAsSymbol, outTokenList);
                return true;
            case '`':
                state->position += 2;
                tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_QUASI_QUOTE, tuuvm_scanner_tokenAsSymbol, outTokenList);
                return true;
            case ',':
                state->position += 2;
                tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_QUASI_UNQUOTE, tuuvm_scanner_tokenAsSymbol, outTokenList);
                return true;
            case '@':
                state->position += 2;
                tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_SPLICE, tuuvm_scanner_tokenAsSymbol, outTokenList);
                return true;
            default:
                break;
            }
        }
        break;
    case '|':
        ++state->position;
        if(tuuvm_scanner_isOperatorCharacter(tuuvm_scanner_lookAt(state, 0)))
        {
            while(tuuvm_scanner_isOperatorCharacter(tuuvm_scanner_lookAt(state, 0)))
                ++state->position;
            tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_OPERATOR, tuuvm_scanner_tokenAsSymbol, outTokenList);
            return true;
        }
        tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_BAR, tuuvm_scanner_tokenAsSymbol, outTokenList);
        return true;
    default:
        // Binary operators
        if(tuuvm_scanner_isOperatorCharacter(tuuvm_scanner_lookAt(state, 0)))
        {
            while(tuuvm_scanner_isOperatorCharacter(tuuvm_scanner_lookAt(state, 0)))
                ++state->position;
            size_t operatorSize = state->position - tokenStartState.position;
            if(operatorSize == 1)
            {
                if(c == '<')
                {
                    tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_LESS_THAN, tuuvm_scanner_tokenAsSymbol, outTokenList);
                    return true;                    
                }
                else if(c == '>')
                {
                    tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_GREATER_THAN, tuuvm_scanner_tokenAsSymbol, outTokenList);
                    return true;                    
                }
                else if(c == '*')
                {
                    tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_STAR, tuuvm_scanner_tokenAsSymbol, outTokenList);
                    return true;                    
                }
            }
            
            tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_OPERATOR, tuuvm_scanner_tokenAsSymbol, outTokenList);
            return true;
        }
        break;
    }

    ++state->position;
    tuuvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, TUUVM_TOKEN_KIND_ERROR, tuuvm_scanner_unrecognizedTokenError, outTokenList);
    return true;
}

TUUVM_API tuuvm_tuple_t tuuvm_scanner_scan(tuuvm_context_t *context, tuuvm_tuple_t sourceCode)
{
    struct {
        tuuvm_tuple_t tokenList;
        tuuvm_tuple_t result;
    } gcFrame = {0};

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    tuuvm_gc_lock(context);

    gcFrame.tokenList = tuuvm_arrayList_create(context);

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
                if(!tuuvm_scanner_scanNextTokenInto(context, &scannerState, gcFrame.tokenList))
                    break;
            }
        }
    }

    gcFrame.result = tuuvm_arrayList_asArraySlice(context, gcFrame.tokenList);
    tuuvm_gc_unlock(context);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

TUUVM_API tuuvm_tuple_t tuuvm_scanner_scanCString(tuuvm_context_t *context, const char *sourceCodeText, const char *sourceCodeName, const char *languageName)
{
    return tuuvm_scanner_scan(context, tuuvm_sourceCode_createWithCStrings(context, sourceCodeText, "", sourceCodeName, languageName));
}
