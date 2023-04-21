#include "sysbvm/parser.h"
#include "sysbvm/ast.h"
#include "sysbvm/array.h"
#include "sysbvm/orderedCollection.h"
#include "sysbvm/arraySlice.h"
#include "sysbvm/gc.h"
#include "sysbvm/token.h"
#include "sysbvm/scanner.h"
#include "sysbvm/sourceCode.h"
#include "sysbvm/sourcePosition.h"

typedef struct sysbvm_parser_state_s
{
    sysbvm_tuple_t sourceCode;
    sysbvm_tuple_t tokenSequence;
    size_t tokenPosition;
    size_t tokenSequenceSize;
} sysbvm_parser_state_t;

static sysbvm_tuple_t sysbvm_parser_parseExpression(sysbvm_context_t *context, sysbvm_parser_state_t *state);

static sysbvm_tuple_t sysbvm_parser_lookAt(sysbvm_parser_state_t *state, size_t offset)
{
    size_t lookPosition = state->tokenPosition + offset;
    return lookPosition < state->tokenSequenceSize ? sysbvm_arraySlice_at(state->tokenSequence, lookPosition) : SYSBVM_NULL_TUPLE;
}

static int sysbvm_parser_lookKindAt(sysbvm_parser_state_t *state, size_t offset)
{
    sysbvm_tuple_t token = sysbvm_parser_lookAt(state, offset);
    return token == SYSBVM_NULL_TUPLE ? (-1) : (int)sysbvm_token_getKind(token);
}

static sysbvm_tuple_t sysbvm_parser_makeSourcePositionForNodeRange(sysbvm_context_t *context, sysbvm_tuple_t firstNode, sysbvm_tuple_t lastNode)
{
    return sysbvm_sourcePosition_createWithUnion(context, sysbvm_astNode_getSourcePosition(firstNode), sysbvm_astNode_getSourcePosition(lastNode));
}

static sysbvm_tuple_t sysbvm_parser_makeSourcePositionForTokenRange(sysbvm_context_t *context, sysbvm_tuple_t tokenSequence, size_t startIndex, size_t endIndex)
{
    if(startIndex == endIndex)
        return sysbvm_token_getSourcePosition(sysbvm_arraySlice_at(tokenSequence, startIndex));

    sysbvm_tuple_t firstToken = sysbvm_arraySlice_at(tokenSequence, startIndex);
    sysbvm_tuple_t lastToken = sysbvm_arraySlice_at(tokenSequence, endIndex - 1);
    return sysbvm_sourcePosition_createWithUnion(context, sysbvm_token_getSourcePosition(firstToken), sysbvm_token_getSourcePosition(lastToken));
}

static sysbvm_tuple_t sysbvm_parser_makeSourcePositionForSourceCode(sysbvm_context_t *context, sysbvm_tuple_t sourceCode)
{
    size_t sourceCodeTextSize = sysbvm_tuple_getSizeInBytes(sysbvm_sourceCode_getText(sourceCode));
    return sysbvm_sourcePosition_createWithIndices(context, sourceCode, 0, sourceCodeTextSize);
}

static sysbvm_tuple_t sysbvm_parser_makeSourcePositionForEndOfSourceCode(sysbvm_context_t *context, sysbvm_tuple_t sourceCode)
{
    size_t sourceCodeTextSize = sysbvm_tuple_getSizeInBytes(sysbvm_sourceCode_getText(sourceCode));
    return sysbvm_sourcePosition_createWithIndices(context, sourceCode, sourceCodeTextSize, sourceCodeTextSize);
}

static sysbvm_tuple_t sysbvm_parser_makeSourcePositionForParserState(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    sysbvm_tuple_t token = sysbvm_parser_lookAt(state, 0);
    if(token == SYSBVM_NULL_TUPLE)
        return sysbvm_parser_makeSourcePositionForEndOfSourceCode(context, state->sourceCode);
    else
        return sysbvm_token_getSourcePosition(token);
}

static sysbvm_tuple_t sysbvm_parser_unexpectedTokenAt(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    sysbvm_tuple_t token = sysbvm_parser_lookAt(state, 0);
    if(token == SYSBVM_NULL_TUPLE)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForEndOfSourceCode(context, state->sourceCode), "Unexpected end of source.");

    ++state->tokenPosition;
    return sysbvm_astErrorNode_createWithCString(context, sysbvm_token_getSourcePosition(token), "Unexpected token.");
}

static sysbvm_tuple_t sysbvm_parser_parseIdentifierReferenceReference(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    sysbvm_tuple_t token = sysbvm_parser_lookAt(state, 0);
    if(token == SYSBVM_NULL_TUPLE)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected an identifier.");

    ++state->tokenPosition;
    return sysbvm_astIdentifierReferenceNode_create(context, sysbvm_token_getSourcePosition(token), sysbvm_token_getValue(token));
}

static sysbvm_tuple_t sysbvm_parser_parseLiteralTokenValue(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    sysbvm_tuple_t token = sysbvm_parser_lookAt(state, 0);
    if(token == SYSBVM_NULL_TUPLE)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected an atom token.");

    ++state->tokenPosition;
    return sysbvm_astLiteralNode_create(context, sysbvm_token_getSourcePosition(token), sysbvm_token_getValue(token));
}

static sysbvm_tuple_t sysbvm_parser_parsePrimaryExpression(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    switch(sysbvm_parser_lookKindAt(state, 0))
    {
    case -1: return SYSBVM_NULL_TUPLE;
    case SYSBVM_TOKEN_KIND_IDENTIFIER:
    case SYSBVM_TOKEN_KIND_KEYWORD:
    case SYSBVM_TOKEN_KIND_MULTI_KEYWORD:
    case SYSBVM_TOKEN_KIND_OPERATOR:
    case SYSBVM_TOKEN_KIND_STAR:
    case SYSBVM_TOKEN_KIND_LESS_THAN:
    case SYSBVM_TOKEN_KIND_GREATER_THAN:
    case SYSBVM_TOKEN_KIND_BAR:
    case SYSBVM_TOKEN_KIND_COLON:
    case SYSBVM_TOKEN_KIND_COLON_COLON:
    case SYSBVM_TOKEN_KIND_ELLIPSIS:
    case SYSBVM_TOKEN_KIND_COMMA:
    case SYSBVM_TOKEN_KIND_SEMICOLON:
    case SYSBVM_TOKEN_KIND_ASSIGNMENT:
        return sysbvm_parser_parseIdentifierReferenceReference(context, state);

    case SYSBVM_TOKEN_KIND_CHARACTER:
    case SYSBVM_TOKEN_KIND_INTEGER:
    case SYSBVM_TOKEN_KIND_FLOAT:
    case SYSBVM_TOKEN_KIND_STRING:
    case SYSBVM_TOKEN_KIND_SYMBOL:
        return sysbvm_parser_parseLiteralTokenValue(context, state);

    default:
        return sysbvm_parser_unexpectedTokenAt(context, state);
    }
}

static sysbvm_tuple_t sysbvm_parser_parseUnexpandedSExpression(sysbvm_context_t *context, sysbvm_parser_state_t *state, sysbvm_tokenKind_t openingToken, sysbvm_tokenKind_t closingToken)
{
    // Left opening parenthesis.
    if(sysbvm_parser_lookKindAt(state, 0) != (int)openingToken)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected an opening parentheses.");
    
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;


    // Elements.
    sysbvm_tuple_t elementList = sysbvm_orderedCollection_create(context);
    while(sysbvm_parser_lookKindAt(state, 0) >= 0 && sysbvm_parser_lookKindAt(state, 0) != (int)closingToken)
        sysbvm_orderedCollection_add(context, elementList, sysbvm_parser_parseExpression(context, state));

    // Right closing parenthesis.
    if(sysbvm_parser_lookKindAt(state, 0) != (int)closingToken)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a closing parentheses.");

    ++state->tokenPosition;
    size_t endPosition = state->tokenPosition;

    sysbvm_tuple_t sourcePosition = sysbvm_parser_makeSourcePositionForTokenRange(context, state->tokenSequence, startPosition, endPosition);
    sysbvm_tuple_t elementsArray = sysbvm_orderedCollection_asArray(context, elementList);
    return sysbvm_astUnexpandedSExpressionNode_create(context, sourcePosition, elementsArray);
}

static sysbvm_tuple_t sysbvm_parser_parseQuote(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_QUOTE)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a quote token.");
    
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    sysbvm_tuple_t node = sysbvm_parser_parseExpression(context, state);
    size_t endPosition = state->tokenPosition;
    return sysbvm_astQuoteNode_create(context, sysbvm_parser_makeSourcePositionForTokenRange(context, state->tokenSequence, startPosition, endPosition), node);
}

static sysbvm_tuple_t sysbvm_parser_parseQuasiQuote(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_QUASI_QUOTE)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a quote token.");
    
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    sysbvm_tuple_t node = sysbvm_parser_parseExpression(context, state);
    size_t endPosition = state->tokenPosition;
    return sysbvm_astQuasiQuoteNode_create(context, sysbvm_parser_makeSourcePositionForTokenRange(context, state->tokenSequence, startPosition, endPosition), node);
}

static sysbvm_tuple_t sysbvm_parser_parseQuasiUnquote(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_QUASI_UNQUOTE)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a quote token.");
    
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    sysbvm_tuple_t node = sysbvm_parser_parseExpression(context, state);
    size_t endPosition = state->tokenPosition;
    return sysbvm_astQuasiUnquoteNode_create(context, sysbvm_parser_makeSourcePositionForTokenRange(context, state->tokenSequence, startPosition, endPosition), node);
}

static sysbvm_tuple_t sysbvm_parser_parseSplice(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_QUASI_UNQUOTE)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a quote token.");
    
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    sysbvm_tuple_t node = sysbvm_parser_parseExpression(context, state);
    size_t endPosition = state->tokenPosition;
    return sysbvm_astSpliceNode_create(context, sysbvm_parser_makeSourcePositionForTokenRange(context, state->tokenSequence, startPosition, endPosition), node);
}

static sysbvm_tuple_t sysbvm_parser_parseExpression(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    switch(sysbvm_parser_lookKindAt(state, 0))
    {
    case SYSBVM_TOKEN_KIND_QUOTE: return sysbvm_parser_parseQuote(context, state);
    case SYSBVM_TOKEN_KIND_QUASI_QUOTE: return sysbvm_parser_parseQuasiQuote(context, state);
    case SYSBVM_TOKEN_KIND_QUASI_UNQUOTE: return sysbvm_parser_parseQuasiUnquote(context, state);
    case SYSBVM_TOKEN_KIND_SPLICE: return sysbvm_parser_parseSplice(context, state);

    case SYSBVM_TOKEN_KIND_LPARENT: return sysbvm_parser_parseUnexpandedSExpression(context, state, SYSBVM_TOKEN_KIND_LPARENT, SYSBVM_TOKEN_KIND_RPARENT);
    case SYSBVM_TOKEN_KIND_LBRACKET: return sysbvm_parser_parseUnexpandedSExpression(context, state, SYSBVM_TOKEN_KIND_LBRACKET, SYSBVM_TOKEN_KIND_RBRACKET);
    case SYSBVM_TOKEN_KIND_LCBRACKET: return sysbvm_parser_parseUnexpandedSExpression(context, state, SYSBVM_TOKEN_KIND_LCBRACKET, SYSBVM_TOKEN_KIND_RCBRACKET);

    default:
        return sysbvm_parser_parsePrimaryExpression(context, state);
    }
}

SYSBVM_API sysbvm_tuple_t sysbvm_parser_parseTokens(sysbvm_context_t *context, sysbvm_tuple_t sourceCode, sysbvm_tuple_t tokenSequence)
{
    struct {
        sysbvm_tuple_t result;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    sysbvm_gc_lock(context);
    sysbvm_parser_state_t parserState = {
        .sourceCode = sourceCode,
        .tokenSequence = tokenSequence,
        .tokenPosition = 0,
        .tokenSequenceSize = sysbvm_arraySlice_getSize(tokenSequence),
    };

    sysbvm_tuple_t expressionOrderedCollection = sysbvm_orderedCollection_create(context);

    // Parse the expressions on the sequence.
    while(sysbvm_parser_lookKindAt(&parserState, 0) >= 0)
    {
        sysbvm_tuple_t expression = sysbvm_parser_parseExpression(context, &parserState);
        if(SYSBVM_NULL_TUPLE == expression)
            break;

        sysbvm_orderedCollection_add(context, expressionOrderedCollection, expression);
    }

    sysbvm_tuple_t expressionsArray = sysbvm_orderedCollection_asArray(context, expressionOrderedCollection);
    size_t expressionsCount = sysbvm_array_getSize(expressionsArray);

    sysbvm_tuple_t sourcePosition = (expressionsCount > 0)
        ? sysbvm_parser_makeSourcePositionForNodeRange(context, sysbvm_array_at(expressionsArray, 0), sysbvm_array_at(expressionsArray, expressionsCount - 1))
        : sysbvm_parser_makeSourcePositionForSourceCode(context, sourceCode);

    sysbvm_tuple_t pragmas = sysbvm_array_create(context, 0);
    gcFrame.result = sysbvm_astSequenceNode_create(context, sourcePosition, pragmas, expressionsArray);
    sysbvm_gc_unlock(context);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_parser_parseSourceCode(sysbvm_context_t *context, sysbvm_tuple_t sourceCode)
{
    struct {
        sysbvm_tuple_t sourceCode;
        sysbvm_tuple_t tokenSequence;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.sourceCode = sourceCode;
    gcFrame.tokenSequence = sysbvm_scanner_scan(context, gcFrame.sourceCode);
    gcFrame.result = sysbvm_parser_parseTokens(context, gcFrame.sourceCode, gcFrame.tokenSequence);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_parser_parseCString(sysbvm_context_t *context, const char *sourceCodeText, const char *sourceCodeName)
{
    sysbvm_tuple_t sourceCode = sysbvm_sourceCode_createWithCStrings(context, sourceCodeText, "", sourceCodeName, "tlisp");
    return sysbvm_parser_parseSourceCode(context, sourceCode);
}
