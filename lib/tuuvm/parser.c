#include "tuuvm/parser.h"
#include "tuuvm/ast.h"
#include "tuuvm/arrayList.h"
#include "tuuvm/arraySlice.h"
#include "tuuvm/gc.h"
#include "tuuvm/token.h"
#include "tuuvm/scanner.h"
#include "tuuvm/sourceCode.h"
#include "tuuvm/sourcePosition.h"

typedef struct tuuvm_parser_state_s
{
    tuuvm_tuple_t sourceCode;
    tuuvm_tuple_t tokenSequence;
    size_t tokenPosition;
    size_t tokenSequenceSize;
} tuuvm_parser_state_t;

static tuuvm_tuple_t tuuvm_parser_parseExpression(tuuvm_context_t *context, tuuvm_parser_state_t *state);

static tuuvm_tuple_t tuuvm_parser_lookAt(tuuvm_parser_state_t *state, size_t offset)
{
    size_t lookPosition = state->tokenPosition + offset;
    return lookPosition < state->tokenSequenceSize ? tuuvm_arraySlice_at(state->tokenSequence, lookPosition) : TUUVM_NULL_TUPLE;
}

static int tuuvm_parser_lookKindAt(tuuvm_parser_state_t *state, size_t offset)
{
    tuuvm_tuple_t token = tuuvm_parser_lookAt(state, offset);
    return token == TUUVM_NULL_TUPLE ? (-1) : (int)tuuvm_token_getKind(token);
}

static tuuvm_tuple_t tuuvm_parser_makeSourcePositionForNodeRange(tuuvm_context_t *context, tuuvm_tuple_t firstNode, tuuvm_tuple_t lastNode)
{
    return tuuvm_sourcePosition_createWithUnion(context, tuuvm_astNode_getSourcePosition(firstNode), tuuvm_astNode_getSourcePosition(lastNode));
}

static tuuvm_tuple_t tuuvm_parser_makeSourcePositionForTokenRange(tuuvm_context_t *context, tuuvm_tuple_t tokenSequence, size_t startIndex, size_t endIndex)
{
    if(startIndex == endIndex)
        return tuuvm_token_getSourcePosition(tuuvm_arraySlice_at(tokenSequence, startIndex));

    tuuvm_tuple_t firstToken = tuuvm_arraySlice_at(tokenSequence, startIndex);
    tuuvm_tuple_t lastToken = tuuvm_arraySlice_at(tokenSequence, endIndex - 1);
    return tuuvm_sourcePosition_createWithUnion(context, tuuvm_token_getSourcePosition(firstToken), tuuvm_token_getSourcePosition(lastToken));
}

static tuuvm_tuple_t tuuvm_parser_makeSourcePositionForSourceCode(tuuvm_context_t *context, tuuvm_tuple_t sourceCode)
{
    size_t sourceCodeTextSize = tuuvm_tuple_getSizeInBytes(tuuvm_sourceCode_getText(sourceCode));
    return tuuvm_sourcePosition_createWithIndices(context, sourceCode, 0, sourceCodeTextSize);
}

static tuuvm_tuple_t tuuvm_parser_makeSourcePositionForEndOfSourceCode(tuuvm_context_t *context, tuuvm_tuple_t sourceCode)
{
    size_t sourceCodeTextSize = tuuvm_tuple_getSizeInBytes(tuuvm_sourceCode_getText(sourceCode));
    return tuuvm_sourcePosition_createWithIndices(context, sourceCode, sourceCodeTextSize, sourceCodeTextSize);
}

static tuuvm_tuple_t tuuvm_parser_makeSourcePositionForParserState(tuuvm_context_t *context, tuuvm_parser_state_t *state)
{
    tuuvm_tuple_t token = tuuvm_parser_lookAt(state, 0);
    if(token == TUUVM_NULL_TUPLE)
        return tuuvm_parser_makeSourcePositionForEndOfSourceCode(context, state->sourceCode);
    else
        return tuuvm_token_getSourcePosition(token);
}

static tuuvm_tuple_t tuuvm_parser_unexpectedTokenAt(tuuvm_context_t *context, tuuvm_parser_state_t *state)
{
    tuuvm_tuple_t token = tuuvm_parser_lookAt(state, 0);
    if(token == TUUVM_NULL_TUPLE)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_parser_makeSourcePositionForEndOfSourceCode(context, state->sourceCode), "Unexpected end of source.");

    ++state->tokenPosition;
    return tuuvm_astErrorNode_createWithCString(context, tuuvm_token_getSourcePosition(token), "Unexpected token.");
}

static tuuvm_tuple_t tuuvm_parser_parseIdentifierReferenceReference(tuuvm_context_t *context, tuuvm_parser_state_t *state)
{
    tuuvm_tuple_t token = tuuvm_parser_lookAt(state, 0);
    if(token == TUUVM_NULL_TUPLE)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_parser_makeSourcePositionForParserState(context, state), "Expected an identifier.");

    ++state->tokenPosition;
    return tuuvm_astIdentifierReferenceNode_create(context, tuuvm_token_getSourcePosition(token), tuuvm_token_getValue(token));
}

static tuuvm_tuple_t tuuvm_parser_parseLiteralTokenValue(tuuvm_context_t *context, tuuvm_parser_state_t *state)
{
    tuuvm_tuple_t token = tuuvm_parser_lookAt(state, 0);
    if(token == TUUVM_NULL_TUPLE)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_parser_makeSourcePositionForParserState(context, state), "Expected an atom token.");

    ++state->tokenPosition;
    return tuuvm_astLiteralNode_create(context, tuuvm_token_getSourcePosition(token), tuuvm_token_getValue(token));
}

static tuuvm_tuple_t tuuvm_parser_parsePrimaryExpression(tuuvm_context_t *context, tuuvm_parser_state_t *state)
{
    switch(tuuvm_parser_lookKindAt(state, 0))
    {
    case -1: return TUUVM_NULL_TUPLE;
    case TUUVM_TOKEN_KIND_IDENTIFIER:
    case TUUVM_TOKEN_KIND_KEYWORD:
    case TUUVM_TOKEN_KIND_MULTI_KEYWORD:
    case TUUVM_TOKEN_KIND_OPERATOR:
    case TUUVM_TOKEN_KIND_BAR:
    case TUUVM_TOKEN_KIND_COLON:
    case TUUVM_TOKEN_KIND_COLON_COLON:
    case TUUVM_TOKEN_KIND_ELLIPSIS:
    case TUUVM_TOKEN_KIND_COMMA:
    case TUUVM_TOKEN_KIND_SEMICOLON:
    case TUUVM_TOKEN_KIND_ASSIGNMENT:
        return tuuvm_parser_parseIdentifierReferenceReference(context, state);

    case TUUVM_TOKEN_KIND_CHARACTER:
    case TUUVM_TOKEN_KIND_INTEGER:
    case TUUVM_TOKEN_KIND_FLOAT:
    case TUUVM_TOKEN_KIND_STRING:
    case TUUVM_TOKEN_KIND_SYMBOL:
        return tuuvm_parser_parseLiteralTokenValue(context, state);

    default:
        return tuuvm_parser_unexpectedTokenAt(context, state);
    }
}

static tuuvm_tuple_t tuuvm_parser_parseUnexpandedSExpression(tuuvm_context_t *context, tuuvm_parser_state_t *state, tuuvm_tokenKind_t openingToken, tuuvm_tokenKind_t closingToken)
{
    // Left opening parenthesis.
    if(tuuvm_parser_lookKindAt(state, 0) != (int)openingToken)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_parser_makeSourcePositionForParserState(context, state), "Expected an opening parentheses.");
    
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;


    // Elements.
    tuuvm_tuple_t elementList = tuuvm_arrayList_create(context);
    while(tuuvm_parser_lookKindAt(state, 0) >= 0 && tuuvm_parser_lookKindAt(state, 0) != (int)closingToken)
        tuuvm_arrayList_add(context, elementList, tuuvm_parser_parseExpression(context, state));

    // Right closing parenthesis.
    if(tuuvm_parser_lookKindAt(state, 0) != (int)closingToken)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_parser_makeSourcePositionForParserState(context, state), "Expected a closing parentheses.");

    ++state->tokenPosition;
    size_t endPosition = state->tokenPosition;

    tuuvm_tuple_t sourcePosition = tuuvm_parser_makeSourcePositionForTokenRange(context, state->tokenSequence, startPosition, endPosition);
    tuuvm_tuple_t elementsArraySlice = tuuvm_arrayList_asArraySlice(context, elementList);
    return tuuvm_astUnexpandedSExpressionNode_create(context, sourcePosition, elementsArraySlice);
}

static tuuvm_tuple_t tuuvm_parser_parseQuote(tuuvm_context_t *context, tuuvm_parser_state_t *state)
{
    if(tuuvm_parser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_QUOTE)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_parser_makeSourcePositionForParserState(context, state), "Expected a quote token.");
    
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    tuuvm_tuple_t node = tuuvm_parser_parseExpression(context, state);
    size_t endPosition = state->tokenPosition;
    return tuuvm_astQuoteNode_create(context, tuuvm_parser_makeSourcePositionForTokenRange(context, state->tokenSequence, startPosition, endPosition), node);
}

static tuuvm_tuple_t tuuvm_parser_parseQuasiQuote(tuuvm_context_t *context, tuuvm_parser_state_t *state)
{
    if(tuuvm_parser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_QUASI_QUOTE)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_parser_makeSourcePositionForParserState(context, state), "Expected a quote token.");
    
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    tuuvm_tuple_t node = tuuvm_parser_parseExpression(context, state);
    size_t endPosition = state->tokenPosition;
    return tuuvm_astQuasiQuoteNode_create(context, tuuvm_parser_makeSourcePositionForTokenRange(context, state->tokenSequence, startPosition, endPosition), node);
}

static tuuvm_tuple_t tuuvm_parser_parseQuasiUnquote(tuuvm_context_t *context, tuuvm_parser_state_t *state)
{
    if(tuuvm_parser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_QUASI_UNQUOTE)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_parser_makeSourcePositionForParserState(context, state), "Expected a quote token.");
    
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    tuuvm_tuple_t node = tuuvm_parser_parseExpression(context, state);
    size_t endPosition = state->tokenPosition;
    return tuuvm_astQuasiUnquoteNode_create(context, tuuvm_parser_makeSourcePositionForTokenRange(context, state->tokenSequence, startPosition, endPosition), node);
}

static tuuvm_tuple_t tuuvm_parser_parseSplice(tuuvm_context_t *context, tuuvm_parser_state_t *state)
{
    if(tuuvm_parser_lookKindAt(state, 0) != TUUVM_TOKEN_KIND_QUASI_UNQUOTE)
        return tuuvm_astErrorNode_createWithCString(context, tuuvm_parser_makeSourcePositionForParserState(context, state), "Expected a quote token.");
    
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    tuuvm_tuple_t node = tuuvm_parser_parseExpression(context, state);
    size_t endPosition = state->tokenPosition;
    return tuuvm_astSpliceNode_create(context, tuuvm_parser_makeSourcePositionForTokenRange(context, state->tokenSequence, startPosition, endPosition), node);
}

static tuuvm_tuple_t tuuvm_parser_parseExpression(tuuvm_context_t *context, tuuvm_parser_state_t *state)
{
    switch(tuuvm_parser_lookKindAt(state, 0))
    {
    case TUUVM_TOKEN_KIND_QUOTE: return tuuvm_parser_parseQuote(context, state);
    case TUUVM_TOKEN_KIND_QUASI_QUOTE: return tuuvm_parser_parseQuasiQuote(context, state);
    case TUUVM_TOKEN_KIND_QUASI_UNQUOTE: return tuuvm_parser_parseQuasiUnquote(context, state);
    case TUUVM_TOKEN_KIND_SPLICE: return tuuvm_parser_parseSplice(context, state);

    case TUUVM_TOKEN_KIND_LPARENT: return tuuvm_parser_parseUnexpandedSExpression(context, state, TUUVM_TOKEN_KIND_LPARENT, TUUVM_TOKEN_KIND_RPARENT);
    case TUUVM_TOKEN_KIND_LBRACKET: return tuuvm_parser_parseUnexpandedSExpression(context, state, TUUVM_TOKEN_KIND_LBRACKET, TUUVM_TOKEN_KIND_RBRACKET);
    case TUUVM_TOKEN_KIND_LCBRACKET: return tuuvm_parser_parseUnexpandedSExpression(context, state, TUUVM_TOKEN_KIND_LCBRACKET, TUUVM_TOKEN_KIND_RCBRACKET);

    default:
        return tuuvm_parser_parsePrimaryExpression(context, state);
    }
}

TUUVM_API tuuvm_tuple_t tuuvm_parser_parseTokens(tuuvm_context_t *context, tuuvm_tuple_t sourceCode, tuuvm_tuple_t tokenSequence)
{
    struct {
        tuuvm_tuple_t result;
    } gcFrame = {};

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    tuuvm_gc_lock(context);
    tuuvm_parser_state_t parserState = {
        .sourceCode = sourceCode,
        .tokenSequence = tokenSequence,
        .tokenPosition = 0,
        .tokenSequenceSize = tuuvm_arraySlice_getSize(tokenSequence),
    };

    tuuvm_tuple_t expressionArrayList = tuuvm_arrayList_create(context);

    // Parse the expressions on the sequence.
    while(tuuvm_parser_lookKindAt(&parserState, 0) >= 0)
    {
        tuuvm_tuple_t expression = tuuvm_parser_parseExpression(context, &parserState);
        if(TUUVM_NULL_TUPLE == expression)
            break;

        tuuvm_arrayList_add(context, expressionArrayList, expression);
    }

    tuuvm_tuple_t expressionsArraySlice = tuuvm_arrayList_asArraySlice(context, expressionArrayList);
    size_t expressionsCount = tuuvm_arraySlice_getSize(expressionsArraySlice);

    tuuvm_tuple_t sourcePosition = (expressionsCount > 0)
        ? tuuvm_parser_makeSourcePositionForNodeRange(context, tuuvm_arraySlice_at(expressionsArraySlice, 0), tuuvm_arraySlice_at(expressionsArraySlice, expressionsCount - 1))
        : tuuvm_parser_makeSourcePositionForSourceCode(context, sourceCode);

    gcFrame.result = tuuvm_astSequenceNode_create(context, sourcePosition, expressionsArraySlice);
    tuuvm_gc_unlock(context);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

TUUVM_API tuuvm_tuple_t tuuvm_parser_parseSourceCode(tuuvm_context_t *context, tuuvm_tuple_t sourceCode)
{
    struct {
        tuuvm_tuple_t sourceCode;
        tuuvm_tuple_t tokenSequence;
        tuuvm_tuple_t result;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.sourceCode = sourceCode;
    gcFrame.tokenSequence = tuuvm_scanner_scan(context, gcFrame.sourceCode);
    gcFrame.result = tuuvm_parser_parseTokens(context, gcFrame.sourceCode, gcFrame.tokenSequence);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

TUUVM_API tuuvm_tuple_t tuuvm_parser_parseCString(tuuvm_context_t *context, const char *sourceCodeText, const char *sourceCodeName)
{
    tuuvm_tuple_t sourceCode = tuuvm_sourceCode_createWithCStrings(context, sourceCodeText, sourceCodeName);
    return tuuvm_parser_parseSourceCode(context, sourceCode);
}
