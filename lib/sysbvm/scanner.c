#include "sysbvm/scanner.h"
#include "sysbvm/arrayList.h"
#include "sysbvm/assert.h"
#include "sysbvm/gc.h"
#include "sysbvm/integer.h"
#include "sysbvm/float.h"
#include "sysbvm/sourceCode.h"
#include "sysbvm/sourcePosition.h"
#include "sysbvm/string.h"
#include "sysbvm/token.h"
#include <stdlib.h>
#include <string.h>

typedef struct sysbvm_scannerState_s
{
    sysbvm_tuple_t sourceCode;
    const uint8_t *text;
    size_t size;
    size_t position;
} sysbvm_scannerState_t;

typedef sysbvm_tuple_t (*sysbvm_scanner_tokenConversionFunction_t)(sysbvm_context_t *context, size_t stringSize, const uint8_t *string);

static inline bool sysbvm_scanner_isOperatorCharacter(int c)
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

static sysbvm_tuple_t sysbvm_scanner_tokenAsSymbol(sysbvm_context_t *context, size_t stringSize, const uint8_t *string)
{
    return sysbvm_symbol_internWithString(context, stringSize, (const char*)string);
}

static sysbvm_tuple_t sysbvm_scanner_tokenAsSymbolWithoutPrefix(sysbvm_context_t *context, size_t stringSize, const uint8_t *string)
{
    return sysbvm_symbol_internWithString(context, stringSize - 1, (const char*)(string + 1));
}

static sysbvm_tuple_t sysbvm_scanner_tokenAsInteger(sysbvm_context_t *context, size_t stringSize, const uint8_t *string)
{
    return sysbvm_integer_parseString(context, stringSize, (const char*)string);
}

static sysbvm_tuple_t sysbvm_scanner_tokenAsFloat(sysbvm_context_t *context, size_t stringSize, const uint8_t *string)
{
    return sysbvm_float64_parseString(context, stringSize, (const char*)string);
}

static sysbvm_tuple_t sysbvm_scanner_tokenAsCharacter(sysbvm_context_t *context, size_t stringSize, const uint8_t *string)
{
    SYSBVM_ASSERT(stringSize >= 2);
    if(stringSize == 2)
    {
        return sysbvm_tuple_char32_encode(context, 0);
    }
    else if(stringSize >= 4 && string[1] == '\\')
    {
        switch(string[2])
        {
        case 't': return sysbvm_tuple_char32_encode(context, '\t');
        case 'r': return sysbvm_tuple_char32_encode(context, '\r');
        case 'n': return sysbvm_tuple_char32_encode(context, '\n');
        default: return sysbvm_tuple_char32_encode(context, string[2]);
        }
    }
    else
    {
        return sysbvm_tuple_char32_encode(context, string[1]);
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

static sysbvm_tuple_t sysbvm_scanner_tokenAsString(sysbvm_context_t *context, size_t stringSize, const uint8_t *string)
{
    SYSBVM_ASSERT(stringSize >= 2);

    size_t requiredStringSize = countSizeRequiredForString(stringSize - 2, string + 1);
    sysbvm_tuple_t allocatedString = sysbvm_string_createEmptyWithSize(context, requiredStringSize);
    uint8_t *destination = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(allocatedString)->bytes;
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
                *destination++ = c;
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

static sysbvm_tuple_t sysbvm_scanner_tokenAsSymbolString(sysbvm_context_t *context, size_t stringSize, const uint8_t *string)
{
    SYSBVM_ASSERT(stringSize >= 3);

    size_t requiredStringSize = countSizeRequiredForString(stringSize - 3, string + 2);
    sysbvm_tuple_t allocatedString = sysbvm_string_createEmptyWithSize(context, requiredStringSize);
    uint8_t *destination = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(allocatedString)->bytes;
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
                *destination++ = c;
                break;
            }
        }
        else
        {
            *destination++ = c;
        }
    }

    return sysbvm_symbol_internFromTuple(context, allocatedString);
}

static sysbvm_tuple_t sysbvm_scanner_incompleteMultilineCommentError(sysbvm_context_t *context, size_t stringSize, const uint8_t *string)
{
    (void)stringSize;
    (void)string;
    return sysbvm_symbol_internWithCString(context, "Incomplete multiline comment.");
}

static sysbvm_tuple_t sysbvm_scanner_unrecognizedTokenError(sysbvm_context_t *context, size_t stringSize, const uint8_t *string)
{
    (void)stringSize;
    (void)string;
    return sysbvm_symbol_internWithCString(context, "Unrecognized token.");
}

static sysbvm_tuple_t sysbvm_scanner_incompleteCharacterError(sysbvm_context_t *context, size_t stringSize, const uint8_t *string)
{
    (void)stringSize;
    (void)string;
    return sysbvm_symbol_internWithCString(context, "Incomplete character.");
}

static sysbvm_tuple_t sysbvm_scanner_incompleteStringError(sysbvm_context_t *context, size_t stringSize, const uint8_t *string)
{
    (void)stringSize;
    (void)string;
    return sysbvm_symbol_internWithCString(context, "Incomplete string.");
}

static int sysbvm_scanner_lookAt(sysbvm_scannerState_t *state, size_t offset)
{
    return state->position + offset < state->size ? state->text[state->position + offset] : -1;
}

static void sysbvm_scanner_emitTokenForStateRange(sysbvm_context_t *context, sysbvm_scannerState_t *startState, sysbvm_scannerState_t *endState, sysbvm_tokenKind_t kind, sysbvm_scanner_tokenConversionFunction_t tokenConversionFunction, sysbvm_tuple_t outTokenList)
{
    sysbvm_tuple_t sourcePosition = sysbvm_sourcePosition_createWithIndices(context, startState->sourceCode, startState->position, endState->position);
    sysbvm_tuple_t value = tokenConversionFunction(context, endState->position - startState->position, startState->text + startState->position);
    sysbvm_tuple_t token = sysbvm_token_createWithKind(context, kind, sourcePosition, value);
    sysbvm_arrayList_add(context, outTokenList, token);
}

static void sysbvm_scanner_skipWhite(sysbvm_scannerState_t *state)
{
    while(state->position < state->size && state->text[state->position] <= ' ')
        ++state->position;
}

static void sysbvm_scanner_skipSingleLineComment(sysbvm_scannerState_t *state)
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

static bool sysbvm_scanner_skipMultiLineComment(sysbvm_context_t *context, sysbvm_scannerState_t *state, sysbvm_tuple_t outTokenList)
{
    if(sysbvm_scanner_lookAt(state, 0) != '#' && sysbvm_scanner_lookAt(state, 1) != '*')
        return true;

    sysbvm_scannerState_t startState = *state;
    state->position += 2;

    while(state->position < state->size)
    {
        if(sysbvm_scanner_lookAt(state, 0) == '*' && sysbvm_scanner_lookAt(state, 1) == '#')
        {
            state->position += 2;
            return true;
        }

        ++state->position;
    }

    sysbvm_scanner_emitTokenForStateRange(context, &startState, state, SYSBVM_TOKEN_KIND_ERROR, sysbvm_scanner_incompleteMultilineCommentError, outTokenList);
    return false;
}

static bool sysbvm_scanner_skipWhiteAndComments(sysbvm_context_t *context, sysbvm_scannerState_t *state, sysbvm_tuple_t outTokenList)
{
    bool hasSeenComment = false;

    do
    {
        hasSeenComment = false;
        sysbvm_scanner_skipWhite(state);

        // Skip the two types of different comments.
        if(sysbvm_scanner_lookAt(state, 0) == '#')
        {
            int commentCharType = sysbvm_scanner_lookAt(state, 1);
            if(commentCharType == '#')
            {
                sysbvm_scanner_skipSingleLineComment(state);
                hasSeenComment = true;
            }
            else if(commentCharType == '*')
            {
                if(!sysbvm_scanner_skipMultiLineComment(context, state, outTokenList))
                    return false;
                hasSeenComment = true;
            }
        }
    } while(hasSeenComment);

    return true;
}

static inline bool sysbvm_scanner_isIdentifierStart(int c)
{
    return ('A' <= c && c <= 'Z') ||
        ('a' <= c && c <= 'z') ||
        '_' == c;
}

static inline bool sysbvm_scanner_isAlpha(int c)
{
    return ('A' <= c && c <= 'Z') ||
        ('a' <= c && c <= 'z');
}

static inline bool sysbvm_scanner_isDigit(int c)
{
    return '0' <= c && c <= '9';
}

static inline bool sysbvm_scanner_isDigitOrUnderscore(int c)
{
    return ('0' <= c && c <= '9') || c == '_';
}

static inline bool sysbvm_scanner_isAlphanumericOrUnderscore(int c)
{
    return sysbvm_scanner_isDigit(c) || sysbvm_scanner_isAlpha(c) || c == '_';
}

static inline bool sysbvm_scanner_isSign(int c)
{
    return '+' == c || c == '-';
}

static inline bool sysbvm_scanner_isIdentifierMiddle(int c)
{
    return sysbvm_scanner_isIdentifierStart(c) || sysbvm_scanner_isDigit(c);
}

static bool sysbvm_scanner_advanceKeyword(sysbvm_scannerState_t *state)
{
    if(!sysbvm_scanner_isIdentifierStart(sysbvm_scanner_lookAt(state, 0)))
        return false;

    sysbvm_scannerState_t endState = *state;
    while(sysbvm_scanner_isIdentifierMiddle(sysbvm_scanner_lookAt(&endState, 0)))
        ++endState.position;

    if(sysbvm_scanner_lookAt(&endState, 0) == ':')
    {
        ++endState.position;
        *state = endState;
        return true;
    }

    return false;
}

static bool sysbvm_scanner_scanNextTokenInto(sysbvm_context_t *context, sysbvm_scannerState_t *state, sysbvm_tuple_t outTokenList)
{
    // Skip the whitespaces and the comments preceeding the token.
    if(!sysbvm_scanner_skipWhiteAndComments(context, state, outTokenList))
        return false;

    // Fetch the first token character.
    int c = sysbvm_scanner_lookAt(state, 0);

    // Is this the end?
    if(c < 0)
        return false;

    // Make a copy of the scanner initial state.
    sysbvm_scannerState_t tokenStartState = *state;

    // Is this an identifier?
    if(sysbvm_scanner_isIdentifierStart(c))
    {
        ++state->position;
        while(sysbvm_scanner_isIdentifierMiddle(sysbvm_scanner_lookAt(state, 0)))
            ++state->position;

        // Chop the scope resolutions.
        while(':' == sysbvm_scanner_lookAt(state, 0) &&
            ':' == sysbvm_scanner_lookAt(state, 1) &&
            sysbvm_scanner_isIdentifierStart(sysbvm_scanner_lookAt(state, 2)))
        {
            state->position += 3;
            while(sysbvm_scanner_isIdentifierMiddle(sysbvm_scanner_lookAt(state, 0)))
                ++state->position;
        }

        // Operator with scope.
        if(':' == sysbvm_scanner_lookAt(state, 0) &&
            ':' == sysbvm_scanner_lookAt(state, 1) &&
            sysbvm_scanner_isOperatorCharacter(sysbvm_scanner_lookAt(state, 2)))
        {
            state->position += 3;
            while(sysbvm_scanner_isOperatorCharacter(sysbvm_scanner_lookAt(state, 0)))
                ++state->position;
            sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_OPERATOR, sysbvm_scanner_tokenAsSymbol, outTokenList);
            return true;
        }

        if(':' == sysbvm_scanner_lookAt(state, 0))
        {
            ++state->position;

            bool isMultikeyword = false;
            while(sysbvm_scanner_advanceKeyword(state))
                isMultikeyword = true;

            sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, isMultikeyword ? SYSBVM_TOKEN_KIND_MULTI_KEYWORD : SYSBVM_TOKEN_KIND_KEYWORD, sysbvm_scanner_tokenAsSymbol, outTokenList);
            return true;
        }

        sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_IDENTIFIER, sysbvm_scanner_tokenAsSymbol, outTokenList);
        return true;
    }

    // Is this a number?
    if(sysbvm_scanner_isDigit(c) ||
        (sysbvm_scanner_isSign(c) && sysbvm_scanner_isDigit(sysbvm_scanner_lookAt(state, 1))))
    {
        ++state->position;
        while(sysbvm_scanner_isDigitOrUnderscore(sysbvm_scanner_lookAt(state, 0)))
            ++state->position;

        // Did we parse the radix?
        if(sysbvm_scanner_lookAt(state, 0) == 'r' || sysbvm_scanner_lookAt(state, 0) == 'R')
        {
            ++state->position;
            while(sysbvm_scanner_isAlphanumericOrUnderscore(sysbvm_scanner_lookAt(state, 0)))
                ++state->position;

            sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_INTEGER, sysbvm_scanner_tokenAsInteger, outTokenList);
            return true;
        }

        // Look for a float
        if(sysbvm_scanner_lookAt(state, 0) == '.')
        {
            if(sysbvm_scanner_isDigit(sysbvm_scanner_lookAt(state, 1)) ||
                (sysbvm_scanner_isSign(sysbvm_scanner_lookAt(state, 1)) && sysbvm_scanner_isDigit(sysbvm_scanner_lookAt(state, 2))))
            {
                ++state->position; // Dot
                if(sysbvm_scanner_isSign(sysbvm_scanner_lookAt(state, 1)))
                    ++state->position; // sign
                ++state->position; // First decimal

                // Remaining fractional part
                while(sysbvm_scanner_isDigitOrUnderscore(sysbvm_scanner_lookAt(state, 0)))
                    ++state->position;

                // Exponent.
                if(sysbvm_scanner_lookAt(state, 0) == 'e' || sysbvm_scanner_lookAt(state, 0) == 'E')
                {
                    ++state->position;
                    if(sysbvm_scanner_lookAt(state, 0) == '+' || sysbvm_scanner_lookAt(state, 0) == '-')
                        --state->position;

                    while(sysbvm_scanner_isDigitOrUnderscore(sysbvm_scanner_lookAt(state, 0)))
                        ++state->position;
                }

                sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_FLOAT, sysbvm_scanner_tokenAsFloat, outTokenList);
                return true;
            }
        }

        sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_INTEGER, sysbvm_scanner_tokenAsInteger, outTokenList);
        return true;
    }

    // Strings
    if(c == '"')
    {
        ++state->position;
        while((c = sysbvm_scanner_lookAt(state, 0)) != '"' && c >= 0)
        {
            if(c == '\\')
            {
                ++state->position;
                if(sysbvm_scanner_lookAt(state, 0) < 0)
                {
                    sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_ERROR, sysbvm_scanner_incompleteStringError, outTokenList);
                    return true;
                }
            }
            ++state->position;
        }

        if(sysbvm_scanner_lookAt(state, 0) != '"')
        {
            sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_ERROR, sysbvm_scanner_incompleteStringError, outTokenList);
            return true;
        }

        ++state->position;
        sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_STRING, sysbvm_scanner_tokenAsString, outTokenList);
        return true;
    }

    // Characters
    if(c == '\'')
    {
        ++state->position;
        while((c = sysbvm_scanner_lookAt(state, 0)) != '\'' && c >= 0)
        {
            if(c == '\\')
            {
                ++state->position;
                if(sysbvm_scanner_lookAt(state, 0) < 0)
                {
                    sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_ERROR, sysbvm_scanner_incompleteCharacterError, outTokenList);
                    return true;
                }
            }
            else
            {
                ++state->position;
            }
        }

        if(sysbvm_scanner_lookAt(state, 0) != '\'')
        {
            sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_ERROR, sysbvm_scanner_incompleteCharacterError, outTokenList);
            return true;
        }

        ++state->position;
        sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_CHARACTER, sysbvm_scanner_tokenAsCharacter, outTokenList);
        return true;
    }

    if(c == '#')
    {
        int c1 = sysbvm_scanner_lookAt(state, 1);

        // Identifier symbols
        if(sysbvm_scanner_isIdentifierStart(c1))
        {
            state->position += 2;
            while(sysbvm_scanner_isIdentifierMiddle(sysbvm_scanner_lookAt(state, 0)))
                ++state->position;

            // Chop the scope resolutions.
            while(':' == sysbvm_scanner_lookAt(state, 0) &&
                ':' == sysbvm_scanner_lookAt(state, 1) &&
                sysbvm_scanner_isIdentifierStart(sysbvm_scanner_lookAt(state, 2)))
            {
                state->position += 3;
                while(sysbvm_scanner_isIdentifierMiddle(sysbvm_scanner_lookAt(state, 0)))
                    ++state->position;
            }

            // Accept keyword symbols.
            if(':' == sysbvm_scanner_lookAt(state, 0))
            {
                ++state->position;
                while(sysbvm_scanner_advanceKeyword(state))
                    ;
            }

            sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_SYMBOL, sysbvm_scanner_tokenAsSymbolWithoutPrefix, outTokenList);
            return true;
        }

        // Operators
        if(sysbvm_scanner_isOperatorCharacter(c1))
        {
            state->position += 2;
            while(sysbvm_scanner_isOperatorCharacter(sysbvm_scanner_lookAt(state, 0)))
                ++state->position;
            sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_SYMBOL, sysbvm_scanner_tokenAsSymbolWithoutPrefix, outTokenList);
            return true;
        }

        // StringSymbol string
        if(c1 == '"')
        {
            state->position += 2;
            while((c = sysbvm_scanner_lookAt(state, 0)) != '"' && c >= 0)
            {
                if(c == '\\')
                {
                    ++state->position;
                    if(sysbvm_scanner_lookAt(state, 0) < 0)
                    {
                        sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_ERROR, sysbvm_scanner_incompleteStringError, outTokenList);
                        return true;
                    }
                }

                ++state->position;
            }

            if(sysbvm_scanner_lookAt(state, 0) != '"')
            {
                sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_ERROR, sysbvm_scanner_incompleteStringError, outTokenList);
                return true;
            }

            ++state->position;
            sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_SYMBOL, sysbvm_scanner_tokenAsSymbolString, outTokenList);
            return true;
        }

        switch(c1)
        {
        case '[':
            state->position += 2;
            sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_BYTE_ARRAY_START, sysbvm_scanner_tokenAsSymbol, outTokenList);
            return true;
        case '{':
            state->position += 2;
            sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_DICTIONARY_START, sysbvm_scanner_tokenAsSymbol, outTokenList);
            return true;
        case '(':
            state->position += 2;
            sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_LITERAL_ARRAY_START, sysbvm_scanner_tokenAsSymbol, outTokenList);
            return true;
        default:
            break;
        }
    }

    switch(c)
    {
    case '(':
        ++state->position;
        sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_LPARENT, sysbvm_scanner_tokenAsSymbol, outTokenList);
        return true;
    case ')':
        ++state->position;
        sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_RPARENT, sysbvm_scanner_tokenAsSymbol, outTokenList);
        return true;
    case '[':
        ++state->position;
        sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_LBRACKET, sysbvm_scanner_tokenAsSymbol, outTokenList);
        return true;
    case ']':
        ++state->position;
        sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_RBRACKET, sysbvm_scanner_tokenAsSymbol, outTokenList);
        return true;
    case '{':
        ++state->position;
        sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_LCBRACKET, sysbvm_scanner_tokenAsSymbol, outTokenList);
        return true;
    case '}':
        ++state->position;
        sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_RCBRACKET, sysbvm_scanner_tokenAsSymbol, outTokenList);
        return true;

    case ';':
        ++state->position;
        sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_SEMICOLON, sysbvm_scanner_tokenAsSymbol, outTokenList);
        return true;

    case ',':
        ++state->position;
        sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_COMMA, sysbvm_scanner_tokenAsSymbol, outTokenList);
        return true;

    case '.':
        ++state->position;
        if('.' == sysbvm_scanner_lookAt(state, 0) && '.' == sysbvm_scanner_lookAt(state, 1))
        {
            state->position += 2;
            sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_ELLIPSIS, sysbvm_scanner_tokenAsSymbol, outTokenList);
            return true;
        }

        sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_DOT, sysbvm_scanner_tokenAsSymbol, outTokenList);
        return true;
    case ':':
        ++state->position;
        if(':' == sysbvm_scanner_lookAt(state, 0))
        {
            ++state->position;
            sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_COLON_COLON, sysbvm_scanner_tokenAsSymbol, outTokenList);
            return true;
        }
        else if('=' == sysbvm_scanner_lookAt(state, 0))
        {
            ++state->position;
            sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_ASSIGNMENT, sysbvm_scanner_tokenAsSymbol, outTokenList);
            return true;
        }

        sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_COLON, sysbvm_scanner_tokenAsSymbol, outTokenList);
        return true;

    case '`':
        {
            switch(sysbvm_scanner_lookAt(state, 1))
            {
            case '\'':
                state->position += 2;
                sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_QUOTE, sysbvm_scanner_tokenAsSymbol, outTokenList);
                return true;
            case '`':
                state->position += 2;
                sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_QUASI_QUOTE, sysbvm_scanner_tokenAsSymbol, outTokenList);
                return true;
            case ',':
                state->position += 2;
                sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_QUASI_UNQUOTE, sysbvm_scanner_tokenAsSymbol, outTokenList);
                return true;
            case '@':
                state->position += 2;
                sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_SPLICE, sysbvm_scanner_tokenAsSymbol, outTokenList);
                return true;
            default:
                break;
            }
        }
        break;
    case '|':
        ++state->position;
        if(sysbvm_scanner_isOperatorCharacter(sysbvm_scanner_lookAt(state, 0)))
        {
            while(sysbvm_scanner_isOperatorCharacter(sysbvm_scanner_lookAt(state, 0)))
                ++state->position;
            sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_OPERATOR, sysbvm_scanner_tokenAsSymbol, outTokenList);
            return true;
        }
        sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_BAR, sysbvm_scanner_tokenAsSymbol, outTokenList);
        return true;
    default:
        // Binary operators
        if(sysbvm_scanner_isOperatorCharacter(sysbvm_scanner_lookAt(state, 0)))
        {
            while(sysbvm_scanner_isOperatorCharacter(sysbvm_scanner_lookAt(state, 0)))
                ++state->position;
            size_t operatorSize = state->position - tokenStartState.position;
            if(operatorSize == 1)
            {
                if(c == '<')
                {
                    sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_LESS_THAN, sysbvm_scanner_tokenAsSymbol, outTokenList);
                    return true;                    
                }
                else if(c == '>')
                {
                    sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_GREATER_THAN, sysbvm_scanner_tokenAsSymbol, outTokenList);
                    return true;                    
                }
                else if(c == '*')
                {
                    sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_STAR, sysbvm_scanner_tokenAsSymbol, outTokenList);
                    return true;                    
                }
            }
            
            sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_OPERATOR, sysbvm_scanner_tokenAsSymbol, outTokenList);
            return true;
        }
        break;
    }

    ++state->position;
    sysbvm_scanner_emitTokenForStateRange(context, &tokenStartState, state, SYSBVM_TOKEN_KIND_ERROR, sysbvm_scanner_unrecognizedTokenError, outTokenList);
    return true;
}

SYSBVM_API sysbvm_tuple_t sysbvm_scanner_scan(sysbvm_context_t *context, sysbvm_tuple_t sourceCode)
{
    struct {
        sysbvm_tuple_t tokenList;
        sysbvm_tuple_t result;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    sysbvm_gc_lock(context);

    gcFrame.tokenList = sysbvm_arrayList_create(context);

    if(sysbvm_tuple_isNonNullPointer(sourceCode))
    {
        sysbvm_sourceCode_t *sourceCodeObject = (sysbvm_sourceCode_t*)sourceCode;
        if(sysbvm_tuple_isBytes(sourceCodeObject->text))
        {
            sysbvm_scannerState_t scannerState = {
                .sourceCode = sourceCode,
                .text = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(sourceCodeObject->text)->bytes,
                .size = sysbvm_tuple_getSizeInBytes(sourceCodeObject->text),
                .position = 0
            };

            while(scannerState.position < scannerState.size)
            {
                if(!sysbvm_scanner_scanNextTokenInto(context, &scannerState, gcFrame.tokenList))
                    break;
            }
        }
    }

    gcFrame.result = sysbvm_arrayList_asArraySlice(context, gcFrame.tokenList);
    sysbvm_gc_unlock(context);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_scanner_scanCString(sysbvm_context_t *context, const char *sourceCodeText, const char *sourceCodeName, const char *languageName)
{
    return sysbvm_scanner_scan(context, sysbvm_sourceCode_createWithCStrings(context, sourceCodeText, "", sourceCodeName, languageName));
}
