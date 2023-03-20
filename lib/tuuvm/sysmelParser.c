#include "tuuvm/sysmelParser.h"
#include "tuuvm/ast.h"
#include "tuuvm/array.h"
#include "tuuvm/arrayList.h"
#include "tuuvm/arraySlice.h"
#include "tuuvm/function.h"
#include "tuuvm/gc.h"
#include "tuuvm/token.h"
#include "tuuvm/scanner.h"
#include "tuuvm/sourceCode.h"
#include "tuuvm/sourcePosition.h"
#include "tuuvm/string.h"
#include "tuuvm/stringStream.h"

typedef struct tuuvm_sysmelParser_state_s
{
    tuuvm_tuple_t sourceCode;
    tuuvm_tuple_t tokenSequence;
    size_t tokenPosition;
    size_t tokenSequenceSize;
} tuuvm_sysmelParser_state_t;

static tuuvm_tuple_t tuuvm_sysmelParser_parseLiteralArrayExpression(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state);

static tuuvm_tuple_t tuuvm_sysmelParser_parseUnaryExpression(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state);
static tuuvm_tuple_t tuuvm_sysmelParser_parseBinaryExpression(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state);
static tuuvm_tuple_t tuuvm_sysmelParser_parseCommaExpressionElement(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state);
static tuuvm_tuple_t tuuvm_sysmelParser_parseExpression(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state);

static tuuvm_tuple_t tuuvm_sysmelParser_parseExpressionList(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state);
static tuuvm_tuple_t tuuvm_sysmelParser_parseSequence(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state);

static tuuvm_tuple_t tuuvm_sysmelParser_parseExpressionListUntil(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state, tuuvm_tokenKind_t delimiter);
static tuuvm_tuple_t tuuvm_sysmelParser_parseSequenceUntil(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state, tuuvm_tokenKind_t delimiter);

static tuuvm_tuple_t tuuvm_sysmelParser_parsePrimaryTerm(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state);

static tuuvm_tuple_t tuuvm_sysmelParser_lookAt(tuuvm_sysmelParser_state_t *state, size_t offset)
{
    size_t lookPosition = state->tokenPosition + offset;
    return lookPosition < state->tokenSequenceSize ? tuuvm_arraySlice_at(state->tokenSequence, lookPosition) : TUUVM_NULL_TUPLE;
}

static int tuuvm_sysmelParser_lookKindAt(tuuvm_sysmelParser_state_t *state, size_t offset)
{
    tuuvm_tuple_t token = tuuvm_sysmelParser_lookAt(state, offset);
    return token == TUUVM_NULL_TUPLE ? (-1) : (int)tuuvm_token_getKind(token);
}

static tuuvm_tuple_t tuuvm_sysmelParser_makeSourcePositionForTokenRange(tuuvm_context_t *context, tuuvm_tuple_t sourceCode, tuuvm_tuple_t tokenSequence, size_t startIndex, size_t endIndex)
{
    if(tuuvm_arraySlice_getSize(tokenSequence) == 0)
        return tuuvm_sourcePosition_createWithIndices(context, sourceCode, 0, 0);

    if(startIndex == endIndex)
        return tuuvm_token_getSourcePosition(tuuvm_arraySlice_at(tokenSequence, startIndex));

    tuuvm_tuple_t firstToken = tuuvm_arraySlice_at(tokenSequence, startIndex);
    tuuvm_tuple_t lastToken = tuuvm_arraySlice_at(tokenSequence, endIndex - 1);
    return tuuvm_sourcePosition_createWithUnion(context, tuuvm_token_getSourcePosition(firstToken), tuuvm_token_getSourcePosition(lastToken));
}

static tuuvm_tuple_t tuuvm_sysmelParser_makeSourcePositionForEndOfSourceCode(tuuvm_context_t *context, tuuvm_tuple_t sourceCode)
{
    size_t sourceCodeTextSize = tuuvm_tuple_getSizeInBytes(tuuvm_sourceCode_getText(sourceCode));
    return tuuvm_sourcePosition_createWithIndices(context, sourceCode, sourceCodeTextSize, sourceCodeTextSize);
}

static tuuvm_tuple_t tuuvm_sysmelParser_makeSourcePositionForParserState(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    tuuvm_tuple_t token = tuuvm_sysmelParser_lookAt(state, 0);
    if(token == TUUVM_NULL_TUPLE)
        return tuuvm_sysmelParser_makeSourcePositionForEndOfSourceCode(context, state->sourceCode);
    else
        return tuuvm_token_getSourcePosition(token);
}

static tuuvm_tuple_t tuuvm_sysmelParser_makeUnaryMessageSend(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t receiver, tuuvm_tuple_t selector)
{
    tuuvm_tuple_t arguments = tuuvm_array_create(context, 0);
    return tuuvm_astMessageSendNode_create(context, sourcePosition, receiver, selector, arguments);
}

static tuuvm_tuple_t tuuvm_sysmelParser_makeBinaryMessageSend(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t receiver, tuuvm_tuple_t selector, tuuvm_tuple_t argument)
{
    tuuvm_tuple_t arguments = tuuvm_array_create(context, 1);
    tuuvm_array_atPut(arguments, 0, argument);
    return tuuvm_astMessageSendNode_create(context, sourcePosition, receiver, selector, arguments);
}

static tuuvm_tuple_t tuuvm_sysmelParser_unexpectedTokenAt(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    tuuvm_tuple_t token = tuuvm_sysmelParser_lookAt(state, 0);
    if(token == TUUVM_NULL_TUPLE)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForEndOfSourceCode(context, state->sourceCode), "Unexpected end of source.");

    ++state->tokenPosition;
    return tuuvm_astErrorNode_createWithCString(context, tuuvm_token_getSourcePosition(token), "Unexpected token.");
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseIdentifierReferenceReference(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    tuuvm_tuple_t token = tuuvm_sysmelParser_lookAt(state, 0);
    if(token == TUUVM_NULL_TUPLE)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected an identifier.");

    ++state->tokenPosition;
    return tuuvm_astIdentifierReferenceNode_create(context, tuuvm_token_getSourcePosition(token), tuuvm_token_getValue(token));
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseLiteralTokenValue(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    tuuvm_tuple_t token = tuuvm_sysmelParser_lookAt(state, 0);
    if(token == TUUVM_NULL_TUPLE)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected an atom token.");

    ++state->tokenPosition;
    return tuuvm_astLiteralNode_create(context, tuuvm_token_getSourcePosition(token), tuuvm_token_getValue(token));
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseQuote(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_QUOTE)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a quote token.");
    
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    tuuvm_tuple_t node = tuuvm_sysmelParser_parsePrimaryTerm(context, state);
    size_t endPosition = state->tokenPosition;
    return tuuvm_astQuoteNode_create(context, tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), node);
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseQuasiQuote(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_QUASI_QUOTE)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a quote token.");
    
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    tuuvm_tuple_t node = tuuvm_sysmelParser_parsePrimaryTerm(context, state);
    size_t endPosition = state->tokenPosition;
    return tuuvm_astQuasiQuoteNode_create(context, tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), node);
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseQuasiUnquote(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_QUASI_UNQUOTE)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a quote token.");
    
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    tuuvm_tuple_t node = tuuvm_sysmelParser_parsePrimaryTerm(context, state);
    size_t endPosition = state->tokenPosition;
    return tuuvm_astQuasiUnquoteNode_create(context, tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), node);
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseSplice(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_QUASI_UNQUOTE)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a quote token.");
    
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    tuuvm_tuple_t node = tuuvm_sysmelParser_parsePrimaryTerm(context, state);
    size_t endPosition = state->tokenPosition;
    return tuuvm_astSpliceNode_create(context, tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), node);
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseParenthesesExpression(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_LPARENT)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a left parenthesis.");
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    // Delimited keyword.
    if(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_KEYWORD &&
        tuuvm_sysmelParser_lookKindAt(state, 1) == TUUVM_TOKEN_KIND_RPARENT)
    {
        tuuvm_tuple_t result = tuuvm_sysmelParser_parseIdentifierReferenceReference(context, state);
        if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_RPARENT)
            return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a right parenthesis.");
        
        ++state->tokenPosition;
        return result;
    }

    if(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_RPARENT)
    {
        ++state->tokenPosition;
        size_t endPosition = state->tokenPosition;
        return tuuvm_astMakeArrayNode_create(context, tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), tuuvm_array_create(context, 0));
    }
    else
    {
        tuuvm_tuple_t expression = tuuvm_sysmelParser_parseExpression(context, state);

        if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_RPARENT)
            return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a right parenthesis.");
        ++state->tokenPosition;
        return expression;
    }
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseBlockArgument(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_COLON)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a colon to delimit a block argument.");
    
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    tuuvm_tuple_t isForAll = TUUVM_FALSE_TUPLE;
    if(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_STAR)
    {
        isForAll = TUUVM_TRUE_TUPLE;
        ++state->tokenPosition;
    }

    tuuvm_tuple_t typeExpression = TUUVM_NULL_TUPLE;

    if(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_LPARENT)
    {
        ++state->tokenPosition;
        typeExpression = tuuvm_sysmelParser_parseExpression(context, state);

        if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_RPARENT)
            return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a right parentheses that specifies the type.");
        ++state->tokenPosition;
    }

    if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_IDENTIFIER)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected an identifier with the argument name.");

    tuuvm_tuple_t nameExpression = tuuvm_sysmelParser_parseLiteralTokenValue(context, state);

    size_t endPosition = state->tokenPosition;
    tuuvm_tuple_t sourcePosition = tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition);

    return tuuvm_astArgumentNode_create(context, sourcePosition, isForAll, nameExpression, typeExpression);
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseBlockExpression(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
   if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_LCBRACKET)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a left curly bracket.");

    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    tuuvm_tuple_t argumentList = TUUVM_NULL_TUPLE;
    tuuvm_tuple_t resultTypeExpression = TUUVM_NULL_TUPLE;
    bool hasArguments = false;
    bool hasEllipsis = false;
    bool hasResultType = false;
    bool hasBlockBar = false;
    while(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_COLON)
    {
        tuuvm_tuple_t argument = tuuvm_sysmelParser_parseBlockArgument(context, state);

        if(!hasArguments)
        {
            argumentList = tuuvm_arrayList_create(context);
            hasArguments = true;
        }

        tuuvm_arrayList_add(context, argumentList, argument);
    }

    if(hasArguments && tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_ELLIPSIS)
    {
        hasEllipsis = true;
        ++state->tokenPosition;
    }

    if(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_COLON_COLON)
    {
        ++state->tokenPosition;
        hasResultType = true;
        resultTypeExpression = tuuvm_sysmelParser_parseUnaryExpression(context, state);
    }

    if(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_BAR)
    {
        hasBlockBar = true;
        ++state->tokenPosition;
    }

    if((hasBlockBar || hasResultType) && !hasBlockBar)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a lambda block back.");

    bool isLambda = hasBlockBar;

    tuuvm_tuple_t sequenceNode = tuuvm_sysmelParser_parseSequenceUntil(context, state, TUUVM_TOKEN_KIND_RCBRACKET);

    if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_RCBRACKET)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a right curly bracket.");

    ++state->tokenPosition;
    size_t endPosition = state->tokenPosition;
    tuuvm_tuple_t sourcePosition = tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition);

    if(isLambda)
    {
        if(!argumentList)
            argumentList = tuuvm_arrayList_create(context);

        tuuvm_tuple_t flags = tuuvm_tuple_bitflags_encode(hasEllipsis ? TUUVM_FUNCTION_FLAGS_VARIADIC : TUUVM_FUNCTION_FLAGS_NONE);
        return tuuvm_astLambdaNode_create(context, sourcePosition, flags,
            tuuvm_arrayList_asArray(context, argumentList),
            resultTypeExpression, sequenceNode);
    }
    else
    {
        return tuuvm_astLexicalBlockNode_create(context, sourcePosition, sequenceNode);
    }
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseLiteralArrayElement(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    switch(tuuvm_sysmelParser_lookKindAt(state, 0))
    {
    case -1: return TUUVM_NULL_TUPLE;
    case TUUVM_TOKEN_KIND_IDENTIFIER:
    case TUUVM_TOKEN_KIND_ELLIPSIS:
    case TUUVM_TOKEN_KIND_MULTI_KEYWORD:
    case TUUVM_TOKEN_KIND_KEYWORD:
    case TUUVM_TOKEN_KIND_OPERATOR:
    case TUUVM_TOKEN_KIND_STAR:
    case TUUVM_TOKEN_KIND_LESS_THAN:
    case TUUVM_TOKEN_KIND_GREATER_THAN:
    case TUUVM_TOKEN_KIND_COLON:
    case TUUVM_TOKEN_KIND_COLON_COLON:
    case TUUVM_TOKEN_KIND_BAR:
    case TUUVM_TOKEN_KIND_COMMA:
    case TUUVM_TOKEN_KIND_DOT:

    case TUUVM_TOKEN_KIND_CHARACTER:
    case TUUVM_TOKEN_KIND_INTEGER:
    case TUUVM_TOKEN_KIND_FLOAT:
    case TUUVM_TOKEN_KIND_STRING:
    case TUUVM_TOKEN_KIND_SYMBOL:
        return tuuvm_sysmelParser_parseLiteralTokenValue(context, state);

    case TUUVM_TOKEN_KIND_LPARENT:
    case TUUVM_TOKEN_KIND_LITERAL_ARRAY_START:
        return tuuvm_sysmelParser_parseLiteralArrayExpression(context, state);
    default:
        return tuuvm_sysmelParser_unexpectedTokenAt(context, state);
    }
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseLiteralArrayExpression(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_LITERAL_ARRAY_START && tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_LPARENT)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a left literal array start.");

    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    tuuvm_tuple_t elements = tuuvm_arrayList_create(context);
    while(tuuvm_sysmelParser_lookKindAt(state, 0) >= 0
        && tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_RPARENT)
    {
        tuuvm_tuple_t element = tuuvm_sysmelParser_parseLiteralArrayElement(context, state);
        if(!element)
            break;
        
        tuuvm_arrayList_add(context, elements, element);
    }

    if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_RPARENT)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a right parent.");

    ++state->tokenPosition;
    size_t endPosition = state->tokenPosition;
    return tuuvm_astMakeArrayNode_create(context,
        tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition),
        tuuvm_arrayList_asArray(context, elements));
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseByteArrayExpression(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_BYTE_ARRAY_START)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a byte array start.");
    
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    tuuvm_tuple_t elements = tuuvm_sysmelParser_parseExpressionListUntil(context, state, TUUVM_TOKEN_KIND_RBRACKET);

    if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_RBRACKET)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a right bracket.");
    
    ++state->tokenPosition;
    size_t endPosition = state->tokenPosition;

    return tuuvm_astMakeByteArrayNode_create(context, tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), elements);
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseDictionaryElement(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    size_t startPosition = state->tokenPosition;
    tuuvm_tuple_t key = TUUVM_NULL_TUPLE;

    // Parse the key
    if(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_KEYWORD)
    {
        tuuvm_tuple_t keyToken = tuuvm_sysmelParser_lookAt(state, 0);
        tuuvm_tuple_t keyValue = tuuvm_token_getValue(keyToken);
        keyValue = tuuvm_symbol_internFromTuple(context, tuuvm_string_createWithoutSuffix(context, keyValue, ":"));
        key = tuuvm_astLiteralNode_create(context, tuuvm_token_getSourcePosition(keyToken), keyValue);
        ++state->tokenPosition;
    }
    else
    {
        key = tuuvm_sysmelParser_parseBinaryExpression(context, state);
        if(!key || tuuvm_astNode_isErrorNode(context, key))
            return key;
        
        if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_COLON)
        {
            tuuvm_tuple_t sourcePosition = tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, state->tokenPosition);
            return tuuvm_astMakeAssociationNode_create(context, sourcePosition, key, TUUVM_NULL_TUPLE);
        }
        ++state->tokenPosition;
    }

    // Do we have a value?.
    tuuvm_tuple_t value = TUUVM_NULL_TUPLE;
    if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_DOT &&
        tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_RCBRACKET)
        value = tuuvm_sysmelParser_parseExpression(context, state);

    tuuvm_tuple_t sourcePosition = tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, state->tokenPosition);
    return tuuvm_astMakeAssociationNode_create(context, sourcePosition, key, value);
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseDictionaryElements(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    while(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_DOT)
        ++state->tokenPosition;

    tuuvm_tuple_t associations = tuuvm_arrayList_create(context);
    while(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_RCBRACKET)
    {
        tuuvm_tuple_t association = tuuvm_sysmelParser_parseDictionaryElement(context, state);
        if(!association)
            break;

        tuuvm_arrayList_add(context, associations, association);
        if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_DOT || tuuvm_astNode_isErrorNode(context, association))
            break;
        
        while(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_DOT)
            ++state->tokenPosition;
    }

    return tuuvm_arrayList_asArray(context, associations);
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseDictionaryExpression(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_DICTIONARY_START)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a dictionary start.");

    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    tuuvm_tuple_t elements = tuuvm_sysmelParser_parseDictionaryElements(context, state);

    if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_RCBRACKET)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a right bracket.");

    ++state->tokenPosition;
    size_t endPosition = state->tokenPosition;

    return tuuvm_astMakeDictionaryNode_create(context, tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), elements); 
}

static tuuvm_tuple_t tuuvm_sysmelParser_parsePrimaryTerm(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    switch(tuuvm_sysmelParser_lookKindAt(state, 0))
    {
    case -1: return TUUVM_NULL_TUPLE;
    case TUUVM_TOKEN_KIND_IDENTIFIER:
    case TUUVM_TOKEN_KIND_ELLIPSIS:
    case TUUVM_TOKEN_KIND_OPERATOR:
    case TUUVM_TOKEN_KIND_BAR:
    case TUUVM_TOKEN_KIND_STAR:
    case TUUVM_TOKEN_KIND_LESS_THAN:
    case TUUVM_TOKEN_KIND_GREATER_THAN:
    case TUUVM_TOKEN_KIND_MULTI_KEYWORD:
        return tuuvm_sysmelParser_parseIdentifierReferenceReference(context, state);

    case TUUVM_TOKEN_KIND_CHARACTER:
    case TUUVM_TOKEN_KIND_INTEGER:
    case TUUVM_TOKEN_KIND_FLOAT:
    case TUUVM_TOKEN_KIND_STRING:
    case TUUVM_TOKEN_KIND_SYMBOL:
        return tuuvm_sysmelParser_parseLiteralTokenValue(context, state);

    case TUUVM_TOKEN_KIND_LPARENT:
        return tuuvm_sysmelParser_parseParenthesesExpression(context, state);
    case TUUVM_TOKEN_KIND_LCBRACKET:
        return tuuvm_sysmelParser_parseBlockExpression(context, state);
    case TUUVM_TOKEN_KIND_LITERAL_ARRAY_START:
        return tuuvm_sysmelParser_parseLiteralArrayExpression(context, state);
    case TUUVM_TOKEN_KIND_BYTE_ARRAY_START:
        return tuuvm_sysmelParser_parseByteArrayExpression(context, state);
    case TUUVM_TOKEN_KIND_DICTIONARY_START:
        return tuuvm_sysmelParser_parseDictionaryExpression(context, state);

    default:
        return tuuvm_sysmelParser_unexpectedTokenAt(context, state);
    }
}

static tuuvm_tuple_t tuuvm_sysmelParser_parsePrimaryExpression(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    switch(tuuvm_sysmelParser_lookKindAt(state, 0))
    {
    case TUUVM_TOKEN_KIND_QUOTE: return tuuvm_sysmelParser_parseQuote(context, state);
    case TUUVM_TOKEN_KIND_QUASI_QUOTE: return tuuvm_sysmelParser_parseQuasiQuote(context, state);
    case TUUVM_TOKEN_KIND_QUASI_UNQUOTE: return tuuvm_sysmelParser_parseQuasiUnquote(context, state);
    case TUUVM_TOKEN_KIND_SPLICE: return tuuvm_sysmelParser_parseSplice(context, state);

    default:
        return tuuvm_sysmelParser_parsePrimaryTerm(context, state);
    }
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseCallExpressionWithReceiver(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state, tuuvm_tuple_t receiver, size_t receiverPosition)
{
    if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_LPARENT)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a left parenthesis.");
    ++state->tokenPosition;

    // Optional arguments.
    tuuvm_tuple_t argumentArrayList = tuuvm_arrayList_create(context);
    tuuvm_arrayList_add(context, argumentArrayList, receiver);
    if(tuuvm_sysmelParser_lookKindAt(state, 0) >= 0 && tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_RPARENT)
    {
        tuuvm_tuple_t argument = tuuvm_sysmelParser_parseCommaExpressionElement(context, state);
        tuuvm_arrayList_add(context, argumentArrayList, argument);

        while(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_COMMA)
        {
            ++state->tokenPosition;
            argument = tuuvm_sysmelParser_parseCommaExpressionElement(context, state);
            tuuvm_arrayList_add(context, argumentArrayList, argument);
        }
    }

    tuuvm_tuple_t arguments = tuuvm_arrayList_asArray(context, argumentArrayList);
    if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_RPARENT)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a right parenthesis.");
    ++state->tokenPosition;

    tuuvm_tuple_t sourcePosition = tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, receiverPosition, state->tokenPosition);
    return tuuvm_astUnexpandedSExpressionNode_create(context, sourcePosition, arguments);
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseSubscriptExpressionWithReceiver(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state, tuuvm_tuple_t receiver, size_t receiverPosition)
{
    if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_LBRACKET)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a subscript left bracket.");
    ++state->tokenPosition;

    tuuvm_tuple_t argument = TUUVM_NULL_TUPLE;
    if(tuuvm_sysmelParser_lookKindAt(state, 0) >= 0 && tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_RBRACKET)
        argument = tuuvm_sysmelParser_parseExpression(context, state);

    if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_RBRACKET)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a subscript right bracket.");
    ++state->tokenPosition;

    tuuvm_tuple_t sourcePosition = tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, receiverPosition, state->tokenPosition);
    if(argument)
        return tuuvm_sysmelParser_makeBinaryMessageSend(context, sourcePosition, receiver, tuuvm_astLiteralNode_create(context, sourcePosition, tuuvm_symbol_internWithCString(context, "[]:")), argument);
    else
        return tuuvm_sysmelParser_makeUnaryMessageSend(context, sourcePosition, receiver, tuuvm_astLiteralNode_create(context, sourcePosition, tuuvm_symbol_internWithCString(context, "[]")));
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseApplyBlockExpressionWithReceiver(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state, tuuvm_tuple_t receiver, size_t receiverPosition)
{
    tuuvm_tuple_t blockExpression = tuuvm_sysmelParser_parseBlockExpression(context, state);
    tuuvm_tuple_t sourcePosition = tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, receiverPosition, state->tokenPosition);
    return tuuvm_sysmelParser_makeBinaryMessageSend(context, sourcePosition, receiver, tuuvm_astLiteralNode_create(context, sourcePosition, tuuvm_symbol_internWithCString(context, "{}:")), blockExpression);
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseApplyByteArrayExpressionWithReceiver(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state, tuuvm_tuple_t receiver, size_t receiverPosition)
{
    tuuvm_tuple_t byteArrayExpression = tuuvm_sysmelParser_parseByteArrayExpression(context, state);
    tuuvm_tuple_t sourcePosition = tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, receiverPosition, state->tokenPosition);
    return tuuvm_sysmelParser_makeBinaryMessageSend(context, sourcePosition, receiver, tuuvm_astLiteralNode_create(context, sourcePosition, tuuvm_symbol_internWithCString(context, "#[]:")), byteArrayExpression);
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseApplyDictionaryWithReceiver(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state, tuuvm_tuple_t receiver, size_t receiverPosition)
{
    tuuvm_tuple_t dictionaryExpression = tuuvm_sysmelParser_parseDictionaryExpression(context, state);
    tuuvm_tuple_t sourcePosition = tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, receiverPosition, state->tokenPosition);
    return tuuvm_sysmelParser_makeBinaryMessageSend(context, sourcePosition, receiver, tuuvm_astLiteralNode_create(context, sourcePosition, tuuvm_symbol_internWithCString(context, "#{}:")), dictionaryExpression);
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseUnaryExpression(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    size_t startPosition = state->tokenPosition;

    tuuvm_tuple_t receiver = tuuvm_sysmelParser_parsePrimaryExpression(context, state);
    bool attemptToContinue = true;
    while(attemptToContinue)
    {
        switch(tuuvm_sysmelParser_lookKindAt(state, 0))
        {
        case TUUVM_TOKEN_KIND_IDENTIFIER:
            {
                tuuvm_tuple_t selector = tuuvm_sysmelParser_parseLiteralTokenValue(context, state);
                size_t endPosition = state->tokenPosition;

                receiver = tuuvm_sysmelParser_makeUnaryMessageSend(context, tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), receiver, selector);
            }
            break;
        case TUUVM_TOKEN_KIND_QUASI_UNQUOTE:
            {
                tuuvm_tuple_t selector = tuuvm_sysmelParser_parseQuasiUnquote(context, state);
                size_t endPosition = state->tokenPosition;

                receiver = tuuvm_sysmelParser_makeUnaryMessageSend(context, tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), receiver, selector);
            }
            break;
        case TUUVM_TOKEN_KIND_LPARENT:
            receiver = tuuvm_sysmelParser_parseCallExpressionWithReceiver(context, state, receiver, startPosition);
            break;
        case TUUVM_TOKEN_KIND_LCBRACKET:
            receiver = tuuvm_sysmelParser_parseApplyBlockExpressionWithReceiver(context, state, receiver, startPosition);
            break;
        case TUUVM_TOKEN_KIND_LBRACKET:
            receiver = tuuvm_sysmelParser_parseSubscriptExpressionWithReceiver(context, state, receiver, startPosition);
            break;
        case TUUVM_TOKEN_KIND_BYTE_ARRAY_START:
            receiver = tuuvm_sysmelParser_parseApplyByteArrayExpressionWithReceiver(context, state, receiver, startPosition);
            break;
        case TUUVM_TOKEN_KIND_DICTIONARY_START:
            receiver = tuuvm_sysmelParser_parseApplyDictionaryWithReceiver(context, state, receiver, startPosition);
            break;
        default:
            attemptToContinue = false;
            break;
        }
    }

    return receiver;
}

static bool tuuvm_sysmelParser_isBinaryExpressionOperator(int tokenKind)
{
    switch(tokenKind)
    {
    case TUUVM_TOKEN_KIND_OPERATOR:
    case TUUVM_TOKEN_KIND_STAR:
    case TUUVM_TOKEN_KIND_LESS_THAN:
    case TUUVM_TOKEN_KIND_GREATER_THAN:
    case TUUVM_TOKEN_KIND_BAR:
        return true;
    default:
        return false;
    }
}
static tuuvm_tuple_t tuuvm_sysmelParser_parseBinaryExpression(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    size_t startPosition = state->tokenPosition;
    tuuvm_tuple_t firstOperand = tuuvm_sysmelParser_parseUnaryExpression(context, state);
    if(tuuvm_sysmelParser_isBinaryExpressionOperator(tuuvm_sysmelParser_lookKindAt(state, 0)))
    {
        tuuvm_tuple_t operands = tuuvm_arrayList_create(context);
        tuuvm_tuple_t operators = tuuvm_arrayList_create(context);
        tuuvm_arrayList_add(context, operands, firstOperand);

        while(tuuvm_sysmelParser_isBinaryExpressionOperator(tuuvm_sysmelParser_lookKindAt(state, 0)))
        {
            tuuvm_tuple_t binaryOperator = tuuvm_sysmelParser_parseLiteralTokenValue(context, state);
            tuuvm_arrayList_add(context, operators, binaryOperator);

            tuuvm_tuple_t nextOperand = tuuvm_sysmelParser_parseUnaryExpression(context, state);
            tuuvm_arrayList_add(context, operands, nextOperand);
        }

        size_t endPosition = state->tokenPosition;
        tuuvm_tuple_t sourcePosition = tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition);

        // Collapse single binary operation into a message send here.
        if(tuuvm_arrayList_getSize(operators) == 1)
        {
            tuuvm_tuple_t receiver = tuuvm_arrayList_at(operands, 0);
            tuuvm_tuple_t selector = tuuvm_arrayList_at(operators, 0);
            tuuvm_tuple_t argument = tuuvm_arrayList_at(operands, 1);
            return tuuvm_sysmelParser_makeBinaryMessageSend(context, sourcePosition, receiver, selector, argument);
        }

        return tuuvm_astBinaryExpressionSequenceNode_create(context, sourcePosition, tuuvm_arrayList_asArray(context, operands), tuuvm_arrayList_asArray(context, operators));
    }
    else
    {
        return firstOperand;
    }
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseMessageWithoutReceiver(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_KEYWORD)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a subscript left bracket.");
    
    size_t startPosition = state->tokenPosition;
    size_t keywordEndPosition = state->tokenPosition;
    tuuvm_tuple_t selectorBuilder = tuuvm_stringStream_create(context);
    tuuvm_tuple_t argumentArrayList = tuuvm_arrayList_create(context);
    while(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_KEYWORD)
    {
        tuuvm_stringStream_nextPutString(context, selectorBuilder, tuuvm_token_getValue(tuuvm_sysmelParser_lookAt(state, 0)));
        ++state->tokenPosition;
        keywordEndPosition = state->tokenPosition;

        tuuvm_tuple_t argument = tuuvm_sysmelParser_parseBinaryExpression(context, state);
        tuuvm_arrayList_add(context, argumentArrayList, argument);
    }

    tuuvm_tuple_t keywordSourcePosition = tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, keywordEndPosition);
    tuuvm_tuple_t functionExpression = tuuvm_astIdentifierReferenceNode_create(context, keywordSourcePosition, tuuvm_stringStream_asSymbol(context, selectorBuilder));

    size_t endPosition = state->tokenPosition;
    tuuvm_tuple_t sourcePosition = tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition);
    return tuuvm_astUnexpandedApplicationNode_create(context, sourcePosition, functionExpression, tuuvm_arrayList_asArray(context, argumentArrayList));
}

static void tuuvm_sysmelParser_parseKeywordMessageParts(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state, tuuvm_tuple_t *outSelector, tuuvm_tuple_t *outArguments)
{
    size_t keywordStartPosition = state->tokenPosition;
    size_t keywordEndPosition = state->tokenPosition;
    tuuvm_tuple_t selectorBuilder = tuuvm_stringStream_create(context);
    tuuvm_tuple_t argumentArrayList = tuuvm_arrayList_create(context);
    while(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_KEYWORD)
    {
        tuuvm_stringStream_nextPutString(context, selectorBuilder, tuuvm_token_getValue(tuuvm_sysmelParser_lookAt(state, 0)));
        ++state->tokenPosition;
        keywordEndPosition = state->tokenPosition;

        tuuvm_tuple_t argument = tuuvm_sysmelParser_parseBinaryExpression(context, state);
        tuuvm_arrayList_add(context, argumentArrayList, argument);
    }

    tuuvm_tuple_t keywordSourcePosition = tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, keywordStartPosition, keywordEndPosition);
    *outSelector = tuuvm_astLiteralNode_create(context, keywordSourcePosition, tuuvm_stringStream_asSymbol(context, selectorBuilder));
    *outArguments = tuuvm_arrayList_asArray(context, argumentArrayList);
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseMessageChainList(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state, size_t startPosition, tuuvm_tuple_t receiver, tuuvm_tuple_t firstChainedMessage)
{
    tuuvm_tuple_t chainedMessageList = tuuvm_arrayList_create(context);
    tuuvm_arrayList_add(context, chainedMessageList, firstChainedMessage);

    while(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_SEMICOLON)
    {
        ++state->tokenPosition;
        switch(tuuvm_sysmelParser_lookKindAt(state, 0))
        {
        case TUUVM_TOKEN_KIND_KEYWORD:
            {
                size_t chainedStartPosition = state->tokenPosition;
                tuuvm_tuple_t selector = TUUVM_NULL_TUPLE;
                tuuvm_tuple_t arguments = TUUVM_NULL_TUPLE;

                tuuvm_sysmelParser_parseKeywordMessageParts(context, state, &selector, &arguments);
                tuuvm_tuple_t chainedSourcePosition = tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, chainedStartPosition, state->tokenPosition);
                tuuvm_arrayList_add(context, chainedMessageList, tuuvm_astMessageChainMessageNode_create(context, chainedSourcePosition, selector, arguments));
            }
            break;
        
        case TUUVM_TOKEN_KIND_OPERATOR:
        case TUUVM_TOKEN_KIND_BAR:
        case TUUVM_TOKEN_KIND_STAR:
        case TUUVM_TOKEN_KIND_LESS_THAN:
        case TUUVM_TOKEN_KIND_GREATER_THAN:
            {
                size_t chainedStartPosition = state->tokenPosition;
                tuuvm_tuple_t selector = tuuvm_sysmelParser_parseLiteralTokenValue(context, state);
                tuuvm_tuple_t argument = tuuvm_sysmelParser_parseBinaryExpression(context, state);
                tuuvm_tuple_t chainedSourcePosition = tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, chainedStartPosition, state->tokenPosition);

                tuuvm_tuple_t arguments = tuuvm_array_create(context, 1);
                tuuvm_array_atPut(arguments, 0, argument);
                tuuvm_arrayList_add(context, chainedMessageList, tuuvm_astMessageChainMessageNode_create(context, chainedSourcePosition, selector, arguments));
            }
            break;
        case TUUVM_TOKEN_KIND_IDENTIFIER:
            {
                size_t chainedStartPosition = state->tokenPosition;
                tuuvm_tuple_t selector = tuuvm_sysmelParser_parseLiteralTokenValue(context, state);
                tuuvm_tuple_t arguments = tuuvm_array_create(context, 0);
                tuuvm_tuple_t chainedSourcePosition = tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, chainedStartPosition, state->tokenPosition);
                tuuvm_arrayList_add(context, chainedMessageList, tuuvm_astMessageChainMessageNode_create(context, chainedSourcePosition, selector, arguments));
            }
            break;
        default:
            tuuvm_arrayList_add(context, chainedMessageList, tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a chained message."));
            ++state->tokenPosition;
            break;
        }
    }

    size_t endPosition = state->tokenPosition;
    tuuvm_tuple_t sourcePosition = tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition);
    return tuuvm_astMessageChainNode_create(context, sourcePosition, receiver, tuuvm_arrayList_asArray(context, chainedMessageList));
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseChainExpression(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    size_t startPosition = state->tokenPosition;
    if(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_KEYWORD)
    {
        tuuvm_tuple_t messageWithoutReceiver = tuuvm_sysmelParser_parseMessageWithoutReceiver(context, state);
        if(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_SEMICOLON)
        {
            tuuvm_tuple_t messages = tuuvm_arrayList_create(context);
            tuuvm_arrayList_add(context, messages, messageWithoutReceiver);
            while(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_SEMICOLON)
            {
                ++state->tokenPosition;
                messageWithoutReceiver = tuuvm_sysmelParser_parseMessageWithoutReceiver(context, state);
                tuuvm_arrayList_add(context, messages, messageWithoutReceiver);
            }

            size_t endPosition = state->tokenPosition;
            tuuvm_tuple_t sourcePosition = tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition);
            tuuvm_tuple_t pragmas = tuuvm_array_create(context, 0);
            return tuuvm_astSequenceNode_create(context, sourcePosition, pragmas, tuuvm_arrayList_asArray(context, messages));
        }
        else
        {
            return messageWithoutReceiver;
        }
    }

    tuuvm_tuple_t receiver = tuuvm_sysmelParser_parseBinaryExpression(context, state);
    if(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_KEYWORD)
    {
        tuuvm_tuple_t selector = TUUVM_NULL_TUPLE;
        tuuvm_tuple_t arguments = TUUVM_NULL_TUPLE;
        size_t keywordStartPosition = state->tokenPosition;
        tuuvm_sysmelParser_parseKeywordMessageParts(context, state, &selector, &arguments);

        if(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_SEMICOLON)
        {
            tuuvm_tuple_t chainedSourcePosition = tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, keywordStartPosition, state->tokenPosition);
            tuuvm_tuple_t firstChainedMessage = tuuvm_astMessageChainMessageNode_create(context, chainedSourcePosition, selector, arguments);
            return tuuvm_sysmelParser_parseMessageChainList(context, state, startPosition, receiver, firstChainedMessage);
        }
        else
        {
            size_t endPosition = state->tokenPosition;
            tuuvm_tuple_t sourcePosition = tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition);
            return tuuvm_astMessageSendNode_create(context, sourcePosition, receiver, selector, arguments);
        }
    }
    else if(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_SEMICOLON)
    {
        if(!tuuvm_astNode_isMessageSendNode(context, receiver))
            return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Message chain requires a starting message expression for its receiver.");

        tuuvm_astMessageSendNode_t *receiverMessageSend = (tuuvm_astMessageSendNode_t *)receiver;
        tuuvm_tuple_t firstChainedMessage = tuuvm_astMessageChainMessageNode_create(context, receiverMessageSend->super.sourcePosition, receiverMessageSend->selector, receiverMessageSend->arguments);
        return tuuvm_sysmelParser_parseMessageChainList(context, state, startPosition, receiverMessageSend->receiver, firstChainedMessage);
    }

    return receiver;
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseLowPrecedenceExpression(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    size_t startPosition = state->tokenPosition;
    tuuvm_tuple_t lastExpression = tuuvm_sysmelParser_parseChainExpression(context, state);
    while(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_COLON_COLON && tuuvm_sysmelParser_isBinaryExpressionOperator(tuuvm_sysmelParser_lookKindAt(state, 1)))
    {
        ++state->tokenPosition;
        tuuvm_tuple_t binaryOperator = tuuvm_sysmelParser_parseLiteralTokenValue(context, state);

        tuuvm_tuple_t argument = tuuvm_sysmelParser_parseChainExpression(context, state);
        size_t endPosition = state->tokenPosition;
        lastExpression = tuuvm_sysmelParser_makeBinaryMessageSend(context, tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), lastExpression, binaryOperator, argument);
    }
    
    return lastExpression;
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseAssignmentExpression(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    size_t startPosition = state->tokenPosition;
    tuuvm_tuple_t reference = tuuvm_sysmelParser_parseLowPrecedenceExpression(context, state);
    if(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_ASSIGNMENT)
    {
        tuuvm_tuple_t assignmentOperator = tuuvm_sysmelParser_parseLiteralTokenValue(context, state);
        
        tuuvm_tuple_t value = tuuvm_sysmelParser_parseAssignmentExpression(context, state);
        size_t endPosition = state->tokenPosition;
        return tuuvm_sysmelParser_makeBinaryMessageSend(context, tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), reference, assignmentOperator, value);
    }
    else
    {
        return reference;
    }
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseCommaExpressionElement(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    return tuuvm_sysmelParser_parseAssignmentExpression(context, state);
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseCommaExpression(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    size_t startPosition = state->tokenPosition;
    tuuvm_tuple_t firstElement = tuuvm_sysmelParser_parseCommaExpressionElement(context, state);
    if(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_COMMA)
    {
        tuuvm_tuple_t elements = tuuvm_arrayList_create(context);
        tuuvm_arrayList_add(context, elements, firstElement);

        while(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_COMMA)
        {
            ++state->tokenPosition;
            tuuvm_sysmelParser_state_t savedState = *state;
            tuuvm_tuple_t nextElement = tuuvm_sysmelParser_parseCommaExpressionElement(context, state);
            if(!nextElement || tuuvm_astNode_isErrorNode(context, nextElement))
            {
                *state = savedState;
                break;
            }
            tuuvm_arrayList_add(context, elements, nextElement);
        }

        size_t endPosition = state->tokenPosition;
        return tuuvm_astMakeArrayNode_create(context,
            tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition),
            tuuvm_arrayList_asArray(context, elements));
    }
    else
    {
        return firstElement;
    }
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseExpression(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    return tuuvm_sysmelParser_parseCommaExpression(context, state);
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseExpressionList(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    tuuvm_tuple_t expressionArrayList = tuuvm_arrayList_create(context);

    // Parse the expressions on the sequence.
    while(tuuvm_sysmelParser_lookKindAt(state, 0) >= 0)
    {
        // Skip the dots at the beginning
        while(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_DOT)
            ++state->tokenPosition;

        tuuvm_tuple_t expression = tuuvm_sysmelParser_parseExpression(context, state);
        if(TUUVM_NULL_TUPLE == expression)
            break;

        tuuvm_arrayList_add(context, expressionArrayList, expression);

        // We need at least a single dot before the next expression.
        if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_DOT)
            break;

        // Skip the extra dots.
        while(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_DOT)
            ++state->tokenPosition;
    }

    return tuuvm_arrayList_asArray(context, expressionArrayList);
}

static tuuvm_tuple_t tuuvm_sysmelParser_parsePragma(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    size_t startPosition = state->tokenPosition;
    if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_LESS_THAN)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a subscript left bracket.");
    ++state->tokenPosition;

    tuuvm_tuple_t selector = TUUVM_NULL_TUPLE;
    tuuvm_tuple_t arguments = TUUVM_NULL_TUPLE;

    switch(tuuvm_sysmelParser_lookKindAt(state, 0))
    {
    case TUUVM_TOKEN_KIND_IDENTIFIER:
        selector = tuuvm_sysmelParser_parseLiteralTokenValue(context, state);
        arguments = tuuvm_array_create(context, 0);
        break;
    case TUUVM_TOKEN_KIND_KEYWORD:
        {
            size_t keywordStartPosition = state->tokenPosition;
            size_t keywordEndPosition = keywordStartPosition;
            tuuvm_tuple_t selectorBuilder = tuuvm_stringStream_create(context);
            tuuvm_tuple_t argumentArrayList = tuuvm_arrayList_create(context);
            while(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_KEYWORD)
            {
                tuuvm_stringStream_nextPutString(context, selectorBuilder, tuuvm_token_getValue(tuuvm_sysmelParser_lookAt(state, 0)));
                ++state->tokenPosition;

                tuuvm_tuple_t argument = tuuvm_sysmelParser_parseUnaryExpression(context, state);
                tuuvm_arrayList_add(context, argumentArrayList, argument);
            }

            tuuvm_tuple_t selectorSourcePosition = tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, keywordStartPosition, keywordEndPosition);
            selector = tuuvm_astLiteralNode_create(context, selectorSourcePosition, tuuvm_stringStream_asSymbol(context, selectorBuilder));
            arguments = tuuvm_arrayList_asArray(context, argumentArrayList);
        }
        break;
    default:
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a valid pragma content.");
    }

    if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_GREATER_THAN)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_sysmelParser_makeSourcePositionForParserState(context, state), "Expected a pragma end.");
    ++state->tokenPosition;

    tuuvm_tuple_t sourcePosition = tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, state->tokenPosition);
    return tuuvm_astPragmaNode_create(context, sourcePosition, selector, arguments);
}

static tuuvm_tuple_t tuuvm_sysmelParser_parsePragmaList(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_LESS_THAN)
        return tuuvm_array_create(context, 0);

    tuuvm_tuple_t list = tuuvm_arrayList_create(context);
    while(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_LESS_THAN)
    {
        tuuvm_tuple_t pragma = tuuvm_sysmelParser_parsePragma(context, state);
        tuuvm_arrayList_add(context, list, pragma);
    }

    return tuuvm_arrayList_asArray(context, list);
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseSequence(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state)
{
    size_t startPosition = state->tokenPosition;
    tuuvm_tuple_t pragmas = tuuvm_sysmelParser_parsePragmaList(context, state);
    tuuvm_tuple_t expressionsArraySlice = tuuvm_sysmelParser_parseExpressionList(context, state);
    size_t endPosition = state->tokenPosition;

    tuuvm_tuple_t sourcePosition = tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition);
    return tuuvm_astSequenceNode_create(context, sourcePosition, pragmas, expressionsArraySlice);
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseExpressionListUntil(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state, tuuvm_tokenKind_t delimiter)
{
    tuuvm_tuple_t expressionArrayList = tuuvm_arrayList_create(context);

    // Parse the expressions on the sequence.
    while(tuuvm_sysmelParser_lookKindAt(state, 0) >= 0 && tuuvm_sysmelParser_lookKindAt(state, 0) != (int)delimiter)
    {
        // Skip the dots at the beginning
        while(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_DOT)
            ++state->tokenPosition;

        tuuvm_tuple_t expression = tuuvm_sysmelParser_parseExpression(context, state);
        if(TUUVM_NULL_TUPLE == expression)
            break;

        tuuvm_arrayList_add(context, expressionArrayList, expression);

        // We need at least a single dot before the next expression.
        if(tuuvm_sysmelParser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_DOT)
            break;

        // Skip the extra dots.
        while(tuuvm_sysmelParser_lookKindAt(state, 0) == TUUVM_TOKEN_KIND_DOT)
            ++state->tokenPosition;
    }

    return tuuvm_arrayList_asArray(context, expressionArrayList);
}

static tuuvm_tuple_t tuuvm_sysmelParser_parseSequenceUntil(tuuvm_context_t *context, tuuvm_sysmelParser_state_t *state, tuuvm_tokenKind_t delimiter)
{
    size_t startPosition = state->tokenPosition;
    tuuvm_tuple_t pragmas = tuuvm_sysmelParser_parsePragmaList(context, state);
    tuuvm_tuple_t expressionsArraySlice = tuuvm_sysmelParser_parseExpressionListUntil(context, state, delimiter);
    size_t endPosition = state->tokenPosition;

    tuuvm_tuple_t sourcePosition = tuuvm_sysmelParser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition);
    return tuuvm_astSequenceNode_create(context, sourcePosition, pragmas, expressionsArraySlice);
}

TUUVM_API tuuvm_tuple_t tuuvm_sysmelParser_parseTokens(tuuvm_context_t *context, tuuvm_tuple_t sourceCode, tuuvm_tuple_t tokenSequence)
{
    struct {
        tuuvm_tuple_t result;
    } gcFrame = {0};

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    tuuvm_gc_lock(context);
    tuuvm_sysmelParser_state_t parserState = {
        .sourceCode = sourceCode,
        .tokenSequence = tokenSequence,
        .tokenPosition = 0,
        .tokenSequenceSize = tuuvm_arraySlice_getSize(tokenSequence),
    };

    gcFrame.result = tuuvm_sysmelParser_parseSequence(context, &parserState);
    tuuvm_gc_unlock(context);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

TUUVM_API tuuvm_tuple_t tuuvm_sysmelParser_parseSourceCode(tuuvm_context_t *context, tuuvm_tuple_t sourceCode)
{
    struct {
        tuuvm_tuple_t sourceCode;
        tuuvm_tuple_t tokenSequence;
        tuuvm_tuple_t result;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.sourceCode = sourceCode;
    gcFrame.tokenSequence = tuuvm_scanner_scan(context, gcFrame.sourceCode);
    gcFrame.result = tuuvm_sysmelParser_parseTokens(context, gcFrame.sourceCode, gcFrame.tokenSequence);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

TUUVM_API tuuvm_tuple_t tuuvm_sysmelParser_parseCString(tuuvm_context_t *context, const char *sourceCodeText, const char *sourceCodeName)
{
    tuuvm_tuple_t sourceCode = tuuvm_sourceCode_createWithCStrings(context, sourceCodeText, "", sourceCodeName, "sysmel");
    return tuuvm_sysmelParser_parseSourceCode(context, sourceCode);
}
