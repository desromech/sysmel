#include "sysbvm/sysmelParser.h"
#include "sysbvm/ast.h"
#include "sysbvm/array.h"
#include "sysbvm/arrayList.h"
#include "sysbvm/arraySlice.h"
#include "sysbvm/function.h"
#include "sysbvm/gc.h"
#include "sysbvm/token.h"
#include "sysbvm/scanner.h"
#include "sysbvm/sourceCode.h"
#include "sysbvm/sourcePosition.h"
#include "sysbvm/string.h"
#include "sysbvm/stringStream.h"

typedef struct sysbvm_sysmelParser_state_s
{
    sysbvm_tuple_t sourceCode;
    sysbvm_tuple_t tokenSequence;
    size_t tokenPosition;
    size_t tokenSequenceSize;
} sysbvm_sysmelParser_state_t;

static sysbvm_tuple_t sysbvm_sysmelParser_parseLiteralArrayExpression(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state);
static sysbvm_tuple_t sysbvm_sysmelParser_parseLiteralByteArrayExpression(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state);

static sysbvm_tuple_t sysbvm_sysmelParser_parseUnaryExpression(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state);
static sysbvm_tuple_t sysbvm_sysmelParser_parseBinaryExpression(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state);
static sysbvm_tuple_t sysbvm_sysmelParser_parseCommaExpressionElement(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state);
static sysbvm_tuple_t sysbvm_sysmelParser_parseExpression(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state);

static sysbvm_tuple_t sysbvm_sysmelParser_parseExpressionList(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state);
static sysbvm_tuple_t sysbvm_sysmelParser_parseSequence(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state);

static sysbvm_tuple_t sysbvm_sysmelParser_parseExpressionListUntil(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state, sysbvm_tokenKind_t delimiter);
static sysbvm_tuple_t sysbvm_sysmelParser_parseSequenceUntil(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state, sysbvm_tokenKind_t delimiter);

static sysbvm_tuple_t sysbvm_sysmelParser_parsePrimaryTerm(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state);

static sysbvm_tuple_t sysbvm_sysmelParser_lookAt(sysbvm_sysmelParser_state_t *state, size_t offset)
{
    size_t lookPosition = state->tokenPosition + offset;
    return lookPosition < state->tokenSequenceSize ? sysbvm_arraySlice_at(state->tokenSequence, lookPosition) : SYSBVM_NULL_TUPLE;
}

static int sysbvm_sysmelParser_lookKindAt(sysbvm_sysmelParser_state_t *state, size_t offset)
{
    sysbvm_tuple_t token = sysbvm_sysmelParser_lookAt(state, offset);
    return token == SYSBVM_NULL_TUPLE ? (-1) : (int)sysbvm_token_getKind(token);
}

static sysbvm_tuple_t sysbvm_sysmelParser_makeSourcePositionForTokenRange(sysbvm_context_t *context, sysbvm_tuple_t sourceCode, sysbvm_tuple_t tokenSequence, size_t startIndex, size_t endIndex)
{
    if(sysbvm_arraySlice_getSize(tokenSequence) == 0)
        return sysbvm_sourcePosition_createWithIndices(context, sourceCode, 0, 0);

    if(startIndex == endIndex)
        return sysbvm_token_getSourcePosition(sysbvm_arraySlice_at(tokenSequence, startIndex));

    sysbvm_tuple_t firstToken = sysbvm_arraySlice_at(tokenSequence, startIndex);
    sysbvm_tuple_t lastToken = sysbvm_arraySlice_at(tokenSequence, endIndex - 1);
    return sysbvm_sourcePosition_createWithUnion(context, sysbvm_token_getSourcePosition(firstToken), sysbvm_token_getSourcePosition(lastToken));
}

static sysbvm_tuple_t sysbvm_sysmelParser_makeSourcePositionForEndOfSourceCode(sysbvm_context_t *context, sysbvm_tuple_t sourceCode)
{
    size_t sourceCodeTextSize = sysbvm_tuple_getSizeInBytes(sysbvm_sourceCode_getText(sourceCode));
    return sysbvm_sourcePosition_createWithIndices(context, sourceCode, sourceCodeTextSize, sourceCodeTextSize);
}

static sysbvm_tuple_t sysbvm_sysmelParser_makeSourcePositionForParserState(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    sysbvm_tuple_t token = sysbvm_sysmelParser_lookAt(state, 0);
    if(token == SYSBVM_NULL_TUPLE)
        return sysbvm_sysmelParser_makeSourcePositionForEndOfSourceCode(context, state->sourceCode);
    else
        return sysbvm_token_getSourcePosition(token);
}

static sysbvm_tuple_t sysbvm_sysmelParser_makeUnaryMessageSend(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t receiver, sysbvm_tuple_t selector)
{
    sysbvm_tuple_t arguments = sysbvm_array_create(context, 0);
    return sysbvm_astMessageSendNode_create(context, sourcePosition, receiver, selector, arguments);
}

static sysbvm_tuple_t sysbvm_sysmelParser_makeBinaryMessageSend(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t receiver, sysbvm_tuple_t selector, sysbvm_tuple_t argument)
{
    sysbvm_tuple_t arguments = sysbvm_array_create(context, 1);
    sysbvm_array_atPut(arguments, 0, argument);
    return sysbvm_astMessageSendNode_create(context, sourcePosition, receiver, selector, arguments);
}

static sysbvm_tuple_t sysbvm_sysmelParser_unexpectedTokenAt(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    sysbvm_tuple_t token = sysbvm_sysmelParser_lookAt(state, 0);
    if(token == SYSBVM_NULL_TUPLE)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForEndOfSourceCode(context, state->sourceCode), "Unexpected end of source.");

    ++state->tokenPosition;
    return sysbvm_astErrorNode_createWithCString(context, sysbvm_token_getSourcePosition(token), "Unexpected token.");
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseIdentifierReferenceReference(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    sysbvm_tuple_t token = sysbvm_sysmelParser_lookAt(state, 0);
    if(token == SYSBVM_NULL_TUPLE)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected an identifier.");

    ++state->tokenPosition;
    return sysbvm_astIdentifierReferenceNode_create(context, sysbvm_token_getSourcePosition(token), sysbvm_token_getValue(token));
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseLiteralTokenValue(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    sysbvm_tuple_t token = sysbvm_sysmelParser_lookAt(state, 0);
    if(token == SYSBVM_NULL_TUPLE)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected an atom token.");

    ++state->tokenPosition;
    return sysbvm_astLiteralNode_create(context, sysbvm_token_getSourcePosition(token), sysbvm_token_getValue(token));
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseQuote(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_QUOTE)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a quote token.");
    
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    sysbvm_tuple_t node = sysbvm_sysmelParser_parsePrimaryTerm(context, state);
    size_t endPosition = state->tokenPosition;
    return sysbvm_astQuoteNode_create(context, sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), node);
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseQuasiQuote(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_QUASI_QUOTE)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a quote token.");
    
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    sysbvm_tuple_t node = sysbvm_sysmelParser_parsePrimaryTerm(context, state);
    size_t endPosition = state->tokenPosition;
    return sysbvm_astQuasiQuoteNode_create(context, sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), node);
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseQuasiUnquote(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_QUASI_UNQUOTE)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a quote token.");
    
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    sysbvm_tuple_t node = sysbvm_sysmelParser_parsePrimaryTerm(context, state);
    size_t endPosition = state->tokenPosition;
    return sysbvm_astQuasiUnquoteNode_create(context, sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), node);
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseSplice(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_QUASI_UNQUOTE)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a quote token.");
    
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    sysbvm_tuple_t node = sysbvm_sysmelParser_parsePrimaryTerm(context, state);
    size_t endPosition = state->tokenPosition;
    return sysbvm_astSpliceNode_create(context, sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), node);
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseParenthesesExpression(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_LPARENT)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a left parenthesis.");
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    // Delimited keyword.
    if(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_KEYWORD &&
        sysbvm_sysmelParser_lookKindAt(state, 1) == SYSBVM_TOKEN_KIND_RPARENT)
    {
        sysbvm_tuple_t result = sysbvm_sysmelParser_parseIdentifierReferenceReference(context, state);
        if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RPARENT)
            return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a right parenthesis.");
        
        ++state->tokenPosition;
        return result;
    }

    if(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_RPARENT)
    {
        ++state->tokenPosition;
        size_t endPosition = state->tokenPosition;
        return sysbvm_astMakeArrayNode_create(context, sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), sysbvm_array_create(context, 0));
    }
    else
    {
        sysbvm_tuple_t expression = sysbvm_sysmelParser_parseExpression(context, state);

        if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RPARENT)
            return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a right parenthesis.");
        ++state->tokenPosition;
        return expression;
    }
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseBlockArgument(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_COLON)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a colon to delimit a block argument.");
    
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    sysbvm_tuple_t isForAll = SYSBVM_FALSE_TUPLE;
    if(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_STAR)
    {
        isForAll = SYSBVM_TRUE_TUPLE;
        ++state->tokenPosition;
    }

    sysbvm_tuple_t typeExpression = SYSBVM_NULL_TUPLE;

    if(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_LPARENT)
    {
        ++state->tokenPosition;
        typeExpression = sysbvm_sysmelParser_parseExpression(context, state);

        if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RPARENT)
            return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a right parentheses that specifies the type.");
        ++state->tokenPosition;
    }

    if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_IDENTIFIER)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected an identifier with the argument name.");

    sysbvm_tuple_t nameExpression = sysbvm_sysmelParser_parseLiteralTokenValue(context, state);

    size_t endPosition = state->tokenPosition;
    sysbvm_tuple_t sourcePosition = sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition);

    return sysbvm_astArgumentNode_create(context, sourcePosition, isForAll, nameExpression, typeExpression);
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseBlockExpression(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
   if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_LCBRACKET)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a left curly bracket.");

    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    sysbvm_tuple_t argumentList = SYSBVM_NULL_TUPLE;
    sysbvm_tuple_t resultTypeExpression = SYSBVM_NULL_TUPLE;
    bool hasArguments = false;
    bool hasEllipsis = false;
    bool hasResultType = false;
    bool hasBlockBar = false;
    while(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_COLON)
    {
        sysbvm_tuple_t argument = sysbvm_sysmelParser_parseBlockArgument(context, state);

        if(!hasArguments)
        {
            argumentList = sysbvm_arrayList_create(context);
            hasArguments = true;
        }

        sysbvm_arrayList_add(context, argumentList, argument);
    }

    if(hasArguments && sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_ELLIPSIS)
    {
        hasEllipsis = true;
        ++state->tokenPosition;
    }

    if(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_COLON_COLON)
    {
        ++state->tokenPosition;
        hasResultType = true;
        resultTypeExpression = sysbvm_sysmelParser_parseUnaryExpression(context, state);
    }

    if(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_BAR)
    {
        hasBlockBar = true;
        ++state->tokenPosition;
    }

    if((hasBlockBar || hasResultType) && !hasBlockBar)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a lambda block back.");

    bool isLambda = hasBlockBar;

    sysbvm_tuple_t sequenceNode = sysbvm_sysmelParser_parseSequenceUntil(context, state, SYSBVM_TOKEN_KIND_RCBRACKET);

    if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RCBRACKET)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a right curly bracket.");

    ++state->tokenPosition;
    size_t endPosition = state->tokenPosition;
    sysbvm_tuple_t sourcePosition = sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition);

    if(isLambda)
    {
        if(!argumentList)
            argumentList = sysbvm_arrayList_create(context);

        sysbvm_tuple_t flags = sysbvm_tuple_bitflags_encode(hasEllipsis ? SYSBVM_FUNCTION_FLAGS_VARIADIC : SYSBVM_FUNCTION_FLAGS_NONE);
        return sysbvm_astLambdaNode_create(context, sourcePosition, flags,
            sysbvm_arrayList_asArray(context, argumentList),
            resultTypeExpression, sequenceNode);
    }
    else
    {
        return sysbvm_astLexicalBlockNode_create(context, sourcePosition, sequenceNode);
    }
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseLiteralArrayElement(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    switch(sysbvm_sysmelParser_lookKindAt(state, 0))
    {
    case -1: return SYSBVM_NULL_TUPLE;
    case SYSBVM_TOKEN_KIND_IDENTIFIER:
    case SYSBVM_TOKEN_KIND_ELLIPSIS:
    case SYSBVM_TOKEN_KIND_MULTI_KEYWORD:
    case SYSBVM_TOKEN_KIND_KEYWORD:
    case SYSBVM_TOKEN_KIND_OPERATOR:
    case SYSBVM_TOKEN_KIND_STAR:
    case SYSBVM_TOKEN_KIND_LESS_THAN:
    case SYSBVM_TOKEN_KIND_GREATER_THAN:
    case SYSBVM_TOKEN_KIND_COLON:
    case SYSBVM_TOKEN_KIND_COLON_COLON:
    case SYSBVM_TOKEN_KIND_BAR:
    case SYSBVM_TOKEN_KIND_COMMA:
    case SYSBVM_TOKEN_KIND_DOT:

    case SYSBVM_TOKEN_KIND_CHARACTER:
    case SYSBVM_TOKEN_KIND_INTEGER:
    case SYSBVM_TOKEN_KIND_FLOAT:
    case SYSBVM_TOKEN_KIND_STRING:
    case SYSBVM_TOKEN_KIND_SYMBOL:
        return sysbvm_sysmelParser_parseLiteralTokenValue(context, state);

    case SYSBVM_TOKEN_KIND_LPARENT:
    case SYSBVM_TOKEN_KIND_LITERAL_ARRAY_START:
        return sysbvm_sysmelParser_parseLiteralArrayExpression(context, state);
    case SYSBVM_TOKEN_KIND_BYTE_ARRAY_START:
        return sysbvm_sysmelParser_parseLiteralByteArrayExpression(context, state);
    default:
        return sysbvm_sysmelParser_unexpectedTokenAt(context, state);
    }
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseLiteralByteArrayExpression(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_BYTE_ARRAY_START)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a left literal byte array start.");

    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    sysbvm_tuple_t elements = sysbvm_arrayList_create(context);
    while(sysbvm_sysmelParser_lookKindAt(state, 0) >= 0
        && sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RBRACKET)
    {
        if(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_DOT)
        {
            ++state->tokenPosition;
            continue;
        }
        else if(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_INTEGER)
        {
            sysbvm_tuple_t element = sysbvm_sysmelParser_parseLiteralTokenValue(context, state);
            if(!element)
                break;
            sysbvm_arrayList_add(context, elements, element);
        }
        else
        {
            break;
        }
    }

    if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RBRACKET)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a right parent.");

    ++state->tokenPosition;
    size_t endPosition = state->tokenPosition;
    return sysbvm_astMakeByteArrayNode_create(context,
        sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition),
        sysbvm_arrayList_asArray(context, elements));
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseLiteralArrayExpression(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_LITERAL_ARRAY_START && sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_LPARENT)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a left literal array start.");

    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    sysbvm_tuple_t elements = sysbvm_arrayList_create(context);
    while(sysbvm_sysmelParser_lookKindAt(state, 0) >= 0
        && sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RPARENT)
    {
        sysbvm_tuple_t element = sysbvm_sysmelParser_parseLiteralArrayElement(context, state);
        if(!element)
            break;
        
        sysbvm_arrayList_add(context, elements, element);
    }

    if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RPARENT)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a right parent.");

    ++state->tokenPosition;
    size_t endPosition = state->tokenPosition;
    return sysbvm_astMakeArrayNode_create(context,
        sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition),
        sysbvm_arrayList_asArray(context, elements));
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseByteArrayExpression(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_BYTE_ARRAY_START)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a byte array start.");
    
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    sysbvm_tuple_t elements = sysbvm_sysmelParser_parseExpressionListUntil(context, state, SYSBVM_TOKEN_KIND_RBRACKET);

    if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RBRACKET)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a right bracket.");
    
    ++state->tokenPosition;
    size_t endPosition = state->tokenPosition;

    return sysbvm_astMakeByteArrayNode_create(context, sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), elements);
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseDictionaryElement(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    size_t startPosition = state->tokenPosition;
    sysbvm_tuple_t key = SYSBVM_NULL_TUPLE;

    // Parse the key
    if(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_KEYWORD)
    {
        sysbvm_tuple_t keyToken = sysbvm_sysmelParser_lookAt(state, 0);
        sysbvm_tuple_t keyValue = sysbvm_token_getValue(keyToken);
        keyValue = sysbvm_symbol_internFromTuple(context, sysbvm_string_createWithoutSuffix(context, keyValue, ":"));
        key = sysbvm_astLiteralNode_create(context, sysbvm_token_getSourcePosition(keyToken), keyValue);
        ++state->tokenPosition;
    }
    else
    {
        key = sysbvm_sysmelParser_parseBinaryExpression(context, state);
        if(!key || sysbvm_astNode_isErrorNode(context, key))
            return key;
        
        if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_COLON)
        {
            sysbvm_tuple_t sourcePosition = sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, state->tokenPosition);
            return sysbvm_astMakeAssociationNode_create(context, sourcePosition, key, SYSBVM_NULL_TUPLE);
        }
        ++state->tokenPosition;
    }

    // Do we have a value?.
    sysbvm_tuple_t value = SYSBVM_NULL_TUPLE;
    if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_DOT &&
        sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RCBRACKET)
        value = sysbvm_sysmelParser_parseExpression(context, state);

    sysbvm_tuple_t sourcePosition = sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, state->tokenPosition);
    return sysbvm_astMakeAssociationNode_create(context, sourcePosition, key, value);
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseDictionaryElements(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    while(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_DOT)
        ++state->tokenPosition;

    sysbvm_tuple_t associations = sysbvm_arrayList_create(context);
    while(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RCBRACKET)
    {
        sysbvm_tuple_t association = sysbvm_sysmelParser_parseDictionaryElement(context, state);
        if(!association)
            break;

        sysbvm_arrayList_add(context, associations, association);
        if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_DOT || sysbvm_astNode_isErrorNode(context, association))
            break;
        
        while(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_DOT)
            ++state->tokenPosition;
    }

    return sysbvm_arrayList_asArray(context, associations);
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseDictionaryExpression(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_DICTIONARY_START)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a dictionary start.");

    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    sysbvm_tuple_t elements = sysbvm_sysmelParser_parseDictionaryElements(context, state);

    if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RCBRACKET)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a right bracket.");

    ++state->tokenPosition;
    size_t endPosition = state->tokenPosition;

    return sysbvm_astMakeDictionaryNode_create(context, sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), elements); 
}

static sysbvm_tuple_t sysbvm_sysmelParser_parsePrimaryTerm(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    switch(sysbvm_sysmelParser_lookKindAt(state, 0))
    {
    case -1: return SYSBVM_NULL_TUPLE;
    case SYSBVM_TOKEN_KIND_IDENTIFIER:
    case SYSBVM_TOKEN_KIND_ELLIPSIS:
    case SYSBVM_TOKEN_KIND_OPERATOR:
    case SYSBVM_TOKEN_KIND_BAR:
    case SYSBVM_TOKEN_KIND_STAR:
    case SYSBVM_TOKEN_KIND_LESS_THAN:
    case SYSBVM_TOKEN_KIND_GREATER_THAN:
    case SYSBVM_TOKEN_KIND_MULTI_KEYWORD:
        return sysbvm_sysmelParser_parseIdentifierReferenceReference(context, state);

    case SYSBVM_TOKEN_KIND_CHARACTER:
    case SYSBVM_TOKEN_KIND_INTEGER:
    case SYSBVM_TOKEN_KIND_FLOAT:
    case SYSBVM_TOKEN_KIND_STRING:
    case SYSBVM_TOKEN_KIND_SYMBOL:
        return sysbvm_sysmelParser_parseLiteralTokenValue(context, state);

    case SYSBVM_TOKEN_KIND_LPARENT:
        return sysbvm_sysmelParser_parseParenthesesExpression(context, state);
    case SYSBVM_TOKEN_KIND_LCBRACKET:
        return sysbvm_sysmelParser_parseBlockExpression(context, state);
    case SYSBVM_TOKEN_KIND_LITERAL_ARRAY_START:
        return sysbvm_sysmelParser_parseLiteralArrayExpression(context, state);
    case SYSBVM_TOKEN_KIND_BYTE_ARRAY_START:
        return sysbvm_sysmelParser_parseByteArrayExpression(context, state);
    case SYSBVM_TOKEN_KIND_DICTIONARY_START:
        return sysbvm_sysmelParser_parseDictionaryExpression(context, state);

    default:
        return sysbvm_sysmelParser_unexpectedTokenAt(context, state);
    }
}

static sysbvm_tuple_t sysbvm_sysmelParser_parsePrimaryExpression(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    switch(sysbvm_sysmelParser_lookKindAt(state, 0))
    {
    case SYSBVM_TOKEN_KIND_QUOTE: return sysbvm_sysmelParser_parseQuote(context, state);
    case SYSBVM_TOKEN_KIND_QUASI_QUOTE: return sysbvm_sysmelParser_parseQuasiQuote(context, state);
    case SYSBVM_TOKEN_KIND_QUASI_UNQUOTE: return sysbvm_sysmelParser_parseQuasiUnquote(context, state);
    case SYSBVM_TOKEN_KIND_SPLICE: return sysbvm_sysmelParser_parseSplice(context, state);

    default:
        return sysbvm_sysmelParser_parsePrimaryTerm(context, state);
    }
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseCallExpressionWithReceiver(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state, sysbvm_tuple_t receiver, size_t receiverPosition)
{
    if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_LPARENT)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a left parenthesis.");
    ++state->tokenPosition;

    // Optional arguments.
    sysbvm_tuple_t argumentArrayList = sysbvm_arrayList_create(context);
    sysbvm_arrayList_add(context, argumentArrayList, receiver);
    if(sysbvm_sysmelParser_lookKindAt(state, 0) >= 0 && sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RPARENT)
    {
        sysbvm_tuple_t argument = sysbvm_sysmelParser_parseCommaExpressionElement(context, state);
        sysbvm_arrayList_add(context, argumentArrayList, argument);

        while(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_COMMA)
        {
            ++state->tokenPosition;
            argument = sysbvm_sysmelParser_parseCommaExpressionElement(context, state);
            sysbvm_arrayList_add(context, argumentArrayList, argument);
        }
    }

    sysbvm_tuple_t arguments = sysbvm_arrayList_asArray(context, argumentArrayList);
    if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RPARENT)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a right parenthesis.");
    ++state->tokenPosition;

    sysbvm_tuple_t sourcePosition = sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, receiverPosition, state->tokenPosition);
    return sysbvm_astUnexpandedSExpressionNode_create(context, sourcePosition, arguments);
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseSubscriptExpressionWithReceiver(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state, sysbvm_tuple_t receiver, size_t receiverPosition)
{
    if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_LBRACKET)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a subscript left bracket.");
    ++state->tokenPosition;

    sysbvm_tuple_t argument = SYSBVM_NULL_TUPLE;
    if(sysbvm_sysmelParser_lookKindAt(state, 0) >= 0 && sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RBRACKET)
        argument = sysbvm_sysmelParser_parseExpression(context, state);

    if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RBRACKET)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a subscript right bracket.");
    ++state->tokenPosition;

    sysbvm_tuple_t sourcePosition = sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, receiverPosition, state->tokenPosition);
    if(argument)
        return sysbvm_sysmelParser_makeBinaryMessageSend(context, sourcePosition, receiver, sysbvm_astLiteralNode_create(context, sourcePosition, sysbvm_symbol_internWithCString(context, "[]:")), argument);
    else
        return sysbvm_sysmelParser_makeUnaryMessageSend(context, sourcePosition, receiver, sysbvm_astLiteralNode_create(context, sourcePosition, sysbvm_symbol_internWithCString(context, "[]")));
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseApplyBlockExpressionWithReceiver(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state, sysbvm_tuple_t receiver, size_t receiverPosition)
{
    sysbvm_tuple_t blockExpression = sysbvm_sysmelParser_parseBlockExpression(context, state);
    sysbvm_tuple_t sourcePosition = sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, receiverPosition, state->tokenPosition);
    return sysbvm_sysmelParser_makeBinaryMessageSend(context, sourcePosition, receiver, sysbvm_astLiteralNode_create(context, sourcePosition, sysbvm_symbol_internWithCString(context, "{}:")), blockExpression);
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseApplyByteArrayExpressionWithReceiver(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state, sysbvm_tuple_t receiver, size_t receiverPosition)
{
    sysbvm_tuple_t byteArrayExpression = sysbvm_sysmelParser_parseByteArrayExpression(context, state);
    sysbvm_tuple_t sourcePosition = sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, receiverPosition, state->tokenPosition);
    return sysbvm_sysmelParser_makeBinaryMessageSend(context, sourcePosition, receiver, sysbvm_astLiteralNode_create(context, sourcePosition, sysbvm_symbol_internWithCString(context, "#[]:")), byteArrayExpression);
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseApplyDictionaryWithReceiver(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state, sysbvm_tuple_t receiver, size_t receiverPosition)
{
    sysbvm_tuple_t dictionaryExpression = sysbvm_sysmelParser_parseDictionaryExpression(context, state);
    sysbvm_tuple_t sourcePosition = sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, receiverPosition, state->tokenPosition);
    return sysbvm_sysmelParser_makeBinaryMessageSend(context, sourcePosition, receiver, sysbvm_astLiteralNode_create(context, sourcePosition, sysbvm_symbol_internWithCString(context, "#{}:")), dictionaryExpression);
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseUnaryExpression(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    size_t startPosition = state->tokenPosition;

    sysbvm_tuple_t receiver = sysbvm_sysmelParser_parsePrimaryExpression(context, state);
    bool attemptToContinue = true;
    while(attemptToContinue)
    {
        switch(sysbvm_sysmelParser_lookKindAt(state, 0))
        {
        case SYSBVM_TOKEN_KIND_IDENTIFIER:
            {
                sysbvm_tuple_t selector = sysbvm_sysmelParser_parseLiteralTokenValue(context, state);
                size_t endPosition = state->tokenPosition;

                receiver = sysbvm_sysmelParser_makeUnaryMessageSend(context, sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), receiver, selector);
            }
            break;
        case SYSBVM_TOKEN_KIND_QUASI_UNQUOTE:
            {
                sysbvm_tuple_t selector = sysbvm_sysmelParser_parseQuasiUnquote(context, state);
                size_t endPosition = state->tokenPosition;

                receiver = sysbvm_sysmelParser_makeUnaryMessageSend(context, sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), receiver, selector);
            }
            break;
        case SYSBVM_TOKEN_KIND_LPARENT:
            receiver = sysbvm_sysmelParser_parseCallExpressionWithReceiver(context, state, receiver, startPosition);
            break;
        case SYSBVM_TOKEN_KIND_LCBRACKET:
            receiver = sysbvm_sysmelParser_parseApplyBlockExpressionWithReceiver(context, state, receiver, startPosition);
            break;
        case SYSBVM_TOKEN_KIND_LBRACKET:
            receiver = sysbvm_sysmelParser_parseSubscriptExpressionWithReceiver(context, state, receiver, startPosition);
            break;
        case SYSBVM_TOKEN_KIND_BYTE_ARRAY_START:
            receiver = sysbvm_sysmelParser_parseApplyByteArrayExpressionWithReceiver(context, state, receiver, startPosition);
            break;
        case SYSBVM_TOKEN_KIND_DICTIONARY_START:
            receiver = sysbvm_sysmelParser_parseApplyDictionaryWithReceiver(context, state, receiver, startPosition);
            break;
        default:
            attemptToContinue = false;
            break;
        }
    }

    return receiver;
}

static bool sysbvm_sysmelParser_isBinaryExpressionOperator(int tokenKind)
{
    switch(tokenKind)
    {
    case SYSBVM_TOKEN_KIND_OPERATOR:
    case SYSBVM_TOKEN_KIND_STAR:
    case SYSBVM_TOKEN_KIND_LESS_THAN:
    case SYSBVM_TOKEN_KIND_GREATER_THAN:
    case SYSBVM_TOKEN_KIND_BAR:
        return true;
    default:
        return false;
    }
}
static sysbvm_tuple_t sysbvm_sysmelParser_parseBinaryExpression(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    size_t startPosition = state->tokenPosition;
    sysbvm_tuple_t firstOperand = sysbvm_sysmelParser_parseUnaryExpression(context, state);
    if(sysbvm_sysmelParser_isBinaryExpressionOperator(sysbvm_sysmelParser_lookKindAt(state, 0)))
    {
        sysbvm_tuple_t operands = sysbvm_arrayList_create(context);
        sysbvm_tuple_t operators = sysbvm_arrayList_create(context);
        sysbvm_arrayList_add(context, operands, firstOperand);

        while(sysbvm_sysmelParser_isBinaryExpressionOperator(sysbvm_sysmelParser_lookKindAt(state, 0)))
        {
            sysbvm_tuple_t binaryOperator = sysbvm_sysmelParser_parseLiteralTokenValue(context, state);
            sysbvm_arrayList_add(context, operators, binaryOperator);

            sysbvm_tuple_t nextOperand = sysbvm_sysmelParser_parseUnaryExpression(context, state);
            sysbvm_arrayList_add(context, operands, nextOperand);
        }

        size_t endPosition = state->tokenPosition;
        sysbvm_tuple_t sourcePosition = sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition);

        // Collapse single binary operation into a message send here.
        if(sysbvm_arrayList_getSize(operators) == 1)
        {
            sysbvm_tuple_t receiver = sysbvm_arrayList_at(operands, 0);
            sysbvm_tuple_t selector = sysbvm_arrayList_at(operators, 0);
            sysbvm_tuple_t argument = sysbvm_arrayList_at(operands, 1);
            return sysbvm_sysmelParser_makeBinaryMessageSend(context, sourcePosition, receiver, selector, argument);
        }

        return sysbvm_astBinaryExpressionSequenceNode_create(context, sourcePosition, sysbvm_arrayList_asArray(context, operands), sysbvm_arrayList_asArray(context, operators));
    }
    else
    {
        return firstOperand;
    }
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseMessageWithoutReceiver(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_KEYWORD)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a subscript left bracket.");
    
    size_t startPosition = state->tokenPosition;
    size_t keywordEndPosition = state->tokenPosition;
    sysbvm_tuple_t selectorBuilder = sysbvm_stringStream_create(context);
    sysbvm_tuple_t argumentArrayList = sysbvm_arrayList_create(context);
    while(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_KEYWORD)
    {
        sysbvm_stringStream_nextPutString(context, selectorBuilder, sysbvm_token_getValue(sysbvm_sysmelParser_lookAt(state, 0)));
        ++state->tokenPosition;
        keywordEndPosition = state->tokenPosition;

        sysbvm_tuple_t argument = sysbvm_sysmelParser_parseBinaryExpression(context, state);
        sysbvm_arrayList_add(context, argumentArrayList, argument);
    }

    sysbvm_tuple_t keywordSourcePosition = sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, keywordEndPosition);
    sysbvm_tuple_t functionExpression = sysbvm_astIdentifierReferenceNode_create(context, keywordSourcePosition, sysbvm_stringStream_asSymbol(context, selectorBuilder));

    size_t endPosition = state->tokenPosition;
    sysbvm_tuple_t sourcePosition = sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition);
    return sysbvm_astUnexpandedApplicationNode_create(context, sourcePosition, functionExpression, sysbvm_arrayList_asArray(context, argumentArrayList));
}

static void sysbvm_sysmelParser_parseKeywordMessageParts(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state, sysbvm_tuple_t *outSelector, sysbvm_tuple_t *outArguments)
{
    size_t keywordStartPosition = state->tokenPosition;
    size_t keywordEndPosition = state->tokenPosition;
    sysbvm_tuple_t selectorBuilder = sysbvm_stringStream_create(context);
    sysbvm_tuple_t argumentArrayList = sysbvm_arrayList_create(context);
    while(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_KEYWORD)
    {
        sysbvm_stringStream_nextPutString(context, selectorBuilder, sysbvm_token_getValue(sysbvm_sysmelParser_lookAt(state, 0)));
        ++state->tokenPosition;
        keywordEndPosition = state->tokenPosition;

        sysbvm_tuple_t argument = sysbvm_sysmelParser_parseBinaryExpression(context, state);
        sysbvm_arrayList_add(context, argumentArrayList, argument);
    }

    sysbvm_tuple_t keywordSourcePosition = sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, keywordStartPosition, keywordEndPosition);
    *outSelector = sysbvm_astLiteralNode_create(context, keywordSourcePosition, sysbvm_stringStream_asSymbol(context, selectorBuilder));
    *outArguments = sysbvm_arrayList_asArray(context, argumentArrayList);
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseMessageChainList(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state, size_t startPosition, sysbvm_tuple_t receiver, sysbvm_tuple_t firstChainedMessage)
{
    sysbvm_tuple_t chainedMessageList = sysbvm_arrayList_create(context);
    sysbvm_arrayList_add(context, chainedMessageList, firstChainedMessage);

    while(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_SEMICOLON)
    {
        ++state->tokenPosition;
        switch(sysbvm_sysmelParser_lookKindAt(state, 0))
        {
        case SYSBVM_TOKEN_KIND_KEYWORD:
            {
                size_t chainedStartPosition = state->tokenPosition;
                sysbvm_tuple_t selector = SYSBVM_NULL_TUPLE;
                sysbvm_tuple_t arguments = SYSBVM_NULL_TUPLE;

                sysbvm_sysmelParser_parseKeywordMessageParts(context, state, &selector, &arguments);
                sysbvm_tuple_t chainedSourcePosition = sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, chainedStartPosition, state->tokenPosition);
                sysbvm_arrayList_add(context, chainedMessageList, sysbvm_astMessageChainMessageNode_create(context, chainedSourcePosition, selector, arguments));
            }
            break;
        
        case SYSBVM_TOKEN_KIND_OPERATOR:
        case SYSBVM_TOKEN_KIND_BAR:
        case SYSBVM_TOKEN_KIND_STAR:
        case SYSBVM_TOKEN_KIND_LESS_THAN:
        case SYSBVM_TOKEN_KIND_GREATER_THAN:
            {
                size_t chainedStartPosition = state->tokenPosition;
                sysbvm_tuple_t selector = sysbvm_sysmelParser_parseLiteralTokenValue(context, state);
                sysbvm_tuple_t argument = sysbvm_sysmelParser_parseBinaryExpression(context, state);
                sysbvm_tuple_t chainedSourcePosition = sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, chainedStartPosition, state->tokenPosition);

                sysbvm_tuple_t arguments = sysbvm_array_create(context, 1);
                sysbvm_array_atPut(arguments, 0, argument);
                sysbvm_arrayList_add(context, chainedMessageList, sysbvm_astMessageChainMessageNode_create(context, chainedSourcePosition, selector, arguments));
            }
            break;
        case SYSBVM_TOKEN_KIND_IDENTIFIER:
            {
                size_t chainedStartPosition = state->tokenPosition;
                sysbvm_tuple_t selector = sysbvm_sysmelParser_parseLiteralTokenValue(context, state);
                sysbvm_tuple_t arguments = sysbvm_array_create(context, 0);
                sysbvm_tuple_t chainedSourcePosition = sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, chainedStartPosition, state->tokenPosition);
                sysbvm_arrayList_add(context, chainedMessageList, sysbvm_astMessageChainMessageNode_create(context, chainedSourcePosition, selector, arguments));
            }
            break;
        default:
            sysbvm_arrayList_add(context, chainedMessageList, sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a chained message."));
            ++state->tokenPosition;
            break;
        }
    }

    size_t endPosition = state->tokenPosition;
    sysbvm_tuple_t sourcePosition = sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition);
    return sysbvm_astMessageChainNode_create(context, sourcePosition, receiver, sysbvm_arrayList_asArray(context, chainedMessageList));
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseChainExpression(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    size_t startPosition = state->tokenPosition;
    if(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_KEYWORD)
    {
        sysbvm_tuple_t messageWithoutReceiver = sysbvm_sysmelParser_parseMessageWithoutReceiver(context, state);
        if(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_SEMICOLON)
        {
            sysbvm_tuple_t messages = sysbvm_arrayList_create(context);
            sysbvm_arrayList_add(context, messages, messageWithoutReceiver);
            while(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_SEMICOLON)
            {
                ++state->tokenPosition;
                messageWithoutReceiver = sysbvm_sysmelParser_parseMessageWithoutReceiver(context, state);
                sysbvm_arrayList_add(context, messages, messageWithoutReceiver);
            }

            size_t endPosition = state->tokenPosition;
            sysbvm_tuple_t sourcePosition = sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition);
            sysbvm_tuple_t pragmas = sysbvm_array_create(context, 0);
            return sysbvm_astSequenceNode_create(context, sourcePosition, pragmas, sysbvm_arrayList_asArray(context, messages));
        }
        else
        {
            return messageWithoutReceiver;
        }
    }

    sysbvm_tuple_t receiver = sysbvm_sysmelParser_parseBinaryExpression(context, state);
    if(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_KEYWORD)
    {
        sysbvm_tuple_t selector = SYSBVM_NULL_TUPLE;
        sysbvm_tuple_t arguments = SYSBVM_NULL_TUPLE;
        size_t keywordStartPosition = state->tokenPosition;
        sysbvm_sysmelParser_parseKeywordMessageParts(context, state, &selector, &arguments);

        if(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_SEMICOLON)
        {
            sysbvm_tuple_t chainedSourcePosition = sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, keywordStartPosition, state->tokenPosition);
            sysbvm_tuple_t firstChainedMessage = sysbvm_astMessageChainMessageNode_create(context, chainedSourcePosition, selector, arguments);
            return sysbvm_sysmelParser_parseMessageChainList(context, state, startPosition, receiver, firstChainedMessage);
        }
        else
        {
            size_t endPosition = state->tokenPosition;
            sysbvm_tuple_t sourcePosition = sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition);
            return sysbvm_astMessageSendNode_create(context, sourcePosition, receiver, selector, arguments);
        }
    }
    else if(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_SEMICOLON)
    {
        if(!sysbvm_astNode_isMessageSendNode(context, receiver))
            return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Message chain requires a starting message expression for its receiver.");

        sysbvm_astMessageSendNode_t *receiverMessageSend = (sysbvm_astMessageSendNode_t *)receiver;
        sysbvm_tuple_t firstChainedMessage = sysbvm_astMessageChainMessageNode_create(context, receiverMessageSend->super.sourcePosition, receiverMessageSend->selector, receiverMessageSend->arguments);
        return sysbvm_sysmelParser_parseMessageChainList(context, state, startPosition, receiverMessageSend->receiver, firstChainedMessage);
    }

    return receiver;
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseLowPrecedenceExpression(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    size_t startPosition = state->tokenPosition;
    sysbvm_tuple_t lastExpression = sysbvm_sysmelParser_parseChainExpression(context, state);
    while(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_COLON_COLON && sysbvm_sysmelParser_isBinaryExpressionOperator(sysbvm_sysmelParser_lookKindAt(state, 1)))
    {
        ++state->tokenPosition;
        sysbvm_tuple_t binaryOperator = sysbvm_sysmelParser_parseLiteralTokenValue(context, state);

        sysbvm_tuple_t argument = sysbvm_sysmelParser_parseChainExpression(context, state);
        size_t endPosition = state->tokenPosition;
        lastExpression = sysbvm_sysmelParser_makeBinaryMessageSend(context, sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), lastExpression, binaryOperator, argument);
    }
    
    return lastExpression;
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseAssignmentExpression(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    size_t startPosition = state->tokenPosition;
    sysbvm_tuple_t reference = sysbvm_sysmelParser_parseLowPrecedenceExpression(context, state);
    if(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_ASSIGNMENT)
    {
        sysbvm_tuple_t assignmentOperator = sysbvm_sysmelParser_parseLiteralTokenValue(context, state);
        
        sysbvm_tuple_t value = sysbvm_sysmelParser_parseAssignmentExpression(context, state);
        size_t endPosition = state->tokenPosition;
        return sysbvm_sysmelParser_makeBinaryMessageSend(context, sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), reference, assignmentOperator, value);
    }
    else
    {
        return reference;
    }
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseCommaExpressionElement(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    return sysbvm_sysmelParser_parseAssignmentExpression(context, state);
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseCommaExpression(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    size_t startPosition = state->tokenPosition;
    sysbvm_tuple_t firstElement = sysbvm_sysmelParser_parseCommaExpressionElement(context, state);
    if(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_COMMA)
    {
        sysbvm_tuple_t elements = sysbvm_arrayList_create(context);
        sysbvm_arrayList_add(context, elements, firstElement);

        while(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_COMMA)
        {
            ++state->tokenPosition;
            sysbvm_sysmelParser_state_t savedState = *state;
            sysbvm_tuple_t nextElement = sysbvm_sysmelParser_parseCommaExpressionElement(context, state);
            if(!nextElement || sysbvm_astNode_isErrorNode(context, nextElement))
            {
                *state = savedState;
                break;
            }
            sysbvm_arrayList_add(context, elements, nextElement);
        }

        size_t endPosition = state->tokenPosition;
        return sysbvm_astMakeArrayNode_create(context,
            sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition),
            sysbvm_arrayList_asArray(context, elements));
    }
    else
    {
        return firstElement;
    }
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseExpression(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    return sysbvm_sysmelParser_parseCommaExpression(context, state);
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseExpressionList(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    sysbvm_tuple_t expressionArrayList = sysbvm_arrayList_create(context);

    // Parse the expressions on the sequence.
    while(sysbvm_sysmelParser_lookKindAt(state, 0) >= 0)
    {
        // Skip the dots at the beginning
        while(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_DOT)
            ++state->tokenPosition;

        sysbvm_tuple_t expression = sysbvm_sysmelParser_parseExpression(context, state);
        if(SYSBVM_NULL_TUPLE == expression)
            break;

        sysbvm_arrayList_add(context, expressionArrayList, expression);

        // We need at least a single dot before the next expression.
        if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_DOT)
            break;

        // Skip the extra dots.
        while(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_DOT)
            ++state->tokenPosition;
    }

    return sysbvm_arrayList_asArray(context, expressionArrayList);
}

static sysbvm_tuple_t sysbvm_sysmelParser_parsePragma(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    size_t startPosition = state->tokenPosition;
    if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_LESS_THAN)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a subscript left bracket.");
    ++state->tokenPosition;

    sysbvm_tuple_t selector = SYSBVM_NULL_TUPLE;
    sysbvm_tuple_t arguments = SYSBVM_NULL_TUPLE;

    switch(sysbvm_sysmelParser_lookKindAt(state, 0))
    {
    case SYSBVM_TOKEN_KIND_IDENTIFIER:
        selector = sysbvm_sysmelParser_parseLiteralTokenValue(context, state);
        arguments = sysbvm_array_create(context, 0);
        break;
    case SYSBVM_TOKEN_KIND_KEYWORD:
        {
            size_t keywordStartPosition = state->tokenPosition;
            size_t keywordEndPosition = keywordStartPosition;
            sysbvm_tuple_t selectorBuilder = sysbvm_stringStream_create(context);
            sysbvm_tuple_t argumentArrayList = sysbvm_arrayList_create(context);
            while(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_KEYWORD)
            {
                sysbvm_stringStream_nextPutString(context, selectorBuilder, sysbvm_token_getValue(sysbvm_sysmelParser_lookAt(state, 0)));
                ++state->tokenPosition;

                sysbvm_tuple_t argument = sysbvm_sysmelParser_parseUnaryExpression(context, state);
                sysbvm_arrayList_add(context, argumentArrayList, argument);
            }

            sysbvm_tuple_t selectorSourcePosition = sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, keywordStartPosition, keywordEndPosition);
            selector = sysbvm_astLiteralNode_create(context, selectorSourcePosition, sysbvm_stringStream_asSymbol(context, selectorBuilder));
            arguments = sysbvm_arrayList_asArray(context, argumentArrayList);
        }
        break;
    default:
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a valid pragma content.");
    }

    if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_GREATER_THAN)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a pragma end.");
    ++state->tokenPosition;

    sysbvm_tuple_t sourcePosition = sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, state->tokenPosition);
    return sysbvm_astPragmaNode_create(context, sourcePosition, selector, arguments);
}

static sysbvm_tuple_t sysbvm_sysmelParser_parsePragmaList(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_LESS_THAN)
        return sysbvm_array_create(context, 0);

    sysbvm_tuple_t list = sysbvm_arrayList_create(context);
    while(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_LESS_THAN)
    {
        sysbvm_tuple_t pragma = sysbvm_sysmelParser_parsePragma(context, state);
        sysbvm_arrayList_add(context, list, pragma);
    }

    return sysbvm_arrayList_asArray(context, list);
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseSequence(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state)
{
    size_t startPosition = state->tokenPosition;
    sysbvm_tuple_t pragmas = sysbvm_sysmelParser_parsePragmaList(context, state);
    sysbvm_tuple_t expressionsArraySlice = sysbvm_sysmelParser_parseExpressionList(context, state);
    size_t endPosition = state->tokenPosition;

    sysbvm_tuple_t sourcePosition = sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition);
    return sysbvm_astSequenceNode_create(context, sourcePosition, pragmas, expressionsArraySlice);
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseExpressionListUntil(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state, sysbvm_tokenKind_t delimiter)
{
    sysbvm_tuple_t expressionArrayList = sysbvm_arrayList_create(context);

    // Parse the expressions on the sequence.
    while(sysbvm_sysmelParser_lookKindAt(state, 0) >= 0 && sysbvm_sysmelParser_lookKindAt(state, 0) != (int)delimiter)
    {
        // Skip the dots at the beginning
        while(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_DOT)
            ++state->tokenPosition;

        sysbvm_tuple_t expression = sysbvm_sysmelParser_parseExpression(context, state);
        if(SYSBVM_NULL_TUPLE == expression)
            break;

        sysbvm_arrayList_add(context, expressionArrayList, expression);

        // We need at least a single dot before the next expression.
        if(sysbvm_sysmelParser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_DOT)
            break;

        // Skip the extra dots.
        while(sysbvm_sysmelParser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_DOT)
            ++state->tokenPosition;
    }

    return sysbvm_arrayList_asArray(context, expressionArrayList);
}

static sysbvm_tuple_t sysbvm_sysmelParser_parseSequenceUntil(sysbvm_context_t *context, sysbvm_sysmelParser_state_t *state, sysbvm_tokenKind_t delimiter)
{
    size_t startPosition = state->tokenPosition;
    sysbvm_tuple_t pragmas = sysbvm_sysmelParser_parsePragmaList(context, state);
    sysbvm_tuple_t expressionsArraySlice = sysbvm_sysmelParser_parseExpressionListUntil(context, state, delimiter);
    size_t endPosition = state->tokenPosition;

    sysbvm_tuple_t sourcePosition = sysbvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition);
    return sysbvm_astSequenceNode_create(context, sourcePosition, pragmas, expressionsArraySlice);
}

SYSBVM_API sysbvm_tuple_t sysbvm_sysmelParser_parseTokens(sysbvm_context_t *context, sysbvm_tuple_t sourceCode, sysbvm_tuple_t tokenSequence)
{
    struct {
        sysbvm_tuple_t result;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    sysbvm_gc_lock(context);
    sysbvm_sysmelParser_state_t parserState = {
        .sourceCode = sourceCode,
        .tokenSequence = tokenSequence,
        .tokenPosition = 0,
        .tokenSequenceSize = sysbvm_arraySlice_getSize(tokenSequence),
    };

    gcFrame.result = sysbvm_sysmelParser_parseSequence(context, &parserState);
    sysbvm_gc_unlock(context);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_sysmelParser_parseSourceCode(sysbvm_context_t *context, sysbvm_tuple_t sourceCode)
{
    struct {
        sysbvm_tuple_t sourceCode;
        sysbvm_tuple_t tokenSequence;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.sourceCode = sourceCode;
    gcFrame.tokenSequence = sysbvm_scanner_scan(context, gcFrame.sourceCode);
    gcFrame.result = sysbvm_sysmelParser_parseTokens(context, gcFrame.sourceCode, gcFrame.tokenSequence);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_sysmelParser_parseCString(sysbvm_context_t *context, const char *sourceCodeText, const char *sourceCodeName)
{
    sysbvm_tuple_t sourceCode = sysbvm_sourceCode_createWithCStrings(context, sourceCodeText, "", sourceCodeName, "sysmel");
    return sysbvm_sysmelParser_parseSourceCode(context, sourceCode);
}
