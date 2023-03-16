#ifndef TUUVM_AST_H
#define TUUVM_AST_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef struct tuuvm_astNode_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t sourcePosition;
    tuuvm_tuple_t analyzedType; // Placeholder for typechecker. Unused by base interpreter.
} tuuvm_astNode_t;

typedef struct tuuvm_astArgumentNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t isForAll;
    tuuvm_tuple_t name;
    tuuvm_tuple_t type;
    tuuvm_tuple_t binding;
} tuuvm_astArgumentNode_t;

typedef struct tuuvm_astBinaryExpressionSequenceNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t operands;
    tuuvm_tuple_t operators;
} tuuvm_astBinaryExpressionSequenceNode_t;

typedef struct tuuvm_astBreakNode_s
{
    tuuvm_astNode_t super;
} tuuvm_astBreakNode_t;

typedef struct tuuvm_astContinueNode_s
{
    tuuvm_astNode_t super;
} tuuvm_astContinueNode_t;

typedef struct tuuvm_astDoWhileContinueWithNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t bodyExpression;
    tuuvm_tuple_t conditionExpression;
    tuuvm_tuple_t continueExpression;
} tuuvm_astDoWhileContinueWithNode_t;

typedef struct tuuvm_astErrorNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t errorMessage;
} tuuvm_astErrorNode_t;

typedef struct tuuvm_astFunctionApplicationNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t functionExpression;
    tuuvm_tuple_t arguments;
} tuuvm_astFunctionApplicationNode_t;

typedef struct tuuvm_astLambdaNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t name;
    tuuvm_tuple_t flags;
    tuuvm_tuple_t arguments;
    tuuvm_tuple_t resultType;
    tuuvm_tuple_t body;
    tuuvm_tuple_t hasLazyAnalysis;
    tuuvm_tuple_t functionDefinition;
    tuuvm_tuple_t binding;
} tuuvm_astLambdaNode_t;

typedef struct tuuvm_astLexicalBlockNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t body;
    tuuvm_tuple_t bodyEnvironment;
} tuuvm_astLexicalBlockNode_t;

typedef struct tuuvm_astLiteralNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t value;
} tuuvm_astLiteralNode_t;

typedef struct tuuvm_astLocalDefinitionNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t nameExpression;
    tuuvm_tuple_t typeExpression;
    tuuvm_tuple_t valueExpression;
    tuuvm_tuple_t binding;
    tuuvm_tuple_t isMacroSymbol;
    tuuvm_tuple_t isMutable;
} tuuvm_astLocalDefinitionNode_t;

typedef struct tuuvm_astIdentifierReferenceNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t value;
    tuuvm_tuple_t binding;
} tuuvm_astIdentifierReferenceNode_t;

typedef struct tuuvm_astIfNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t conditionExpression;
    tuuvm_tuple_t trueExpression;
    tuuvm_tuple_t falseExpression;
} tuuvm_astIfNode_t;

typedef struct tuuvm_astMakeAssociationNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t key;
    tuuvm_tuple_t value;
} tuuvm_astMakeAssociationNode_t;

typedef struct tuuvm_astMakeByteArrayNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t elements;
} tuuvm_astMakeByteArrayNode_t;

typedef struct tuuvm_astMakeDictionaryNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t elements;
} tuuvm_astMakeDictionaryNode_t;

typedef struct tuuvm_astMakeTupleNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t elements;
} tuuvm_astMakeTupleNode_t;

typedef struct tuuvm_astMessageSendNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t receiver;
    tuuvm_tuple_t receiverLookupType;
    tuuvm_tuple_t selector;
    tuuvm_tuple_t arguments;
} tuuvm_astMessageSendNode_t;

typedef struct tuuvm_astMessageChainNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t receiver;
    tuuvm_tuple_t receiverLookupType;
    tuuvm_tuple_t messages;
} tuuvm_astMessageChainNode_t;

typedef struct tuuvm_astMessageChainMessageNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t selector;
    tuuvm_tuple_t arguments;
} tuuvm_astMessageChainMessageNode_t;

typedef struct tuuvm_astObjectWithLookupStartingFromNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t objectExpression;
    tuuvm_tuple_t typeExpression;
} tuuvm_astObjectWithLookupStartingFromNode_t;

typedef struct tuuvm_astPragmaNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t selector;
    tuuvm_tuple_t arguments;
} tuuvm_astPragmaNode_t;

typedef struct tuuvm_astReturnNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t expression;
} tuuvm_astReturnNode_t;

typedef struct tuuvm_astSequenceNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t pragmas;
    tuuvm_tuple_t expressions;
} tuuvm_astSequenceNode_t;

typedef struct tuuvm_astUnexpandedApplicationNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t functionOrMacroExpression;
    tuuvm_tuple_t arguments;
} tuuvm_astUnexpandedApplicationNode_t;

typedef struct tuuvm_astUnexpandedSExpressionNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t elements;
} tuuvm_astUnexpandedSExpressionNode_t;

typedef struct tuuvm_astQuoteNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t node;
} tuuvm_astQuoteNode_t;

typedef struct tuuvm_astQuasiQuoteNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t node;
} tuuvm_astQuasiQuoteNode_t;

typedef struct tuuvm_astQuasiUnquoteNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t expression;
} tuuvm_astQuasiUnquoteNode_t;

typedef struct tuuvm_astSpliceNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t expression;
} tuuvm_astSpliceNode_t;

typedef struct tuuvm_astWhileContinueWithNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t conditionExpression;
    tuuvm_tuple_t bodyExpression;
    tuuvm_tuple_t continueExpression;
} tuuvm_astWhileContinueWithNode_t;

/**
 * Is this an argument node?
 */ 
TUUVM_API bool tuuvm_astNode_isArgumentNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this a binary expression sequence node?
 */ 
TUUVM_API bool tuuvm_astNode_isBinaryExpressionSequenceNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this a break node?
 */ 
TUUVM_API bool tuuvm_astNode_isBreakNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this a continue node?
 */ 
TUUVM_API bool tuuvm_astNode_isContinueNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this a do while continue with node?
 */ 
TUUVM_API bool tuuvm_astNode_isDoWhileContinueWithNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this an error node?
 */ 
TUUVM_API bool tuuvm_astNode_isErrorNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this a function application node?
 */ 
TUUVM_API bool tuuvm_astNode_isFunctionApplicationNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this an identifier reference node?
 */ 
TUUVM_API bool tuuvm_astNode_isIdentifierReferenceNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this an if node?
 */ 
TUUVM_API bool tuuvm_astNode_isIfNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this a lambda node?
 */ 
TUUVM_API bool tuuvm_astNode_isLambdaNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this a lexical block node?
 */ 
TUUVM_API bool tuuvm_astNode_isLexicalBlockNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this a literal node?
 */ 
TUUVM_API bool tuuvm_astNode_isLiteralNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this a local definition node?
 */ 
TUUVM_API bool tuuvm_astNode_isLocalDefinitionNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this a make association node?
 */ 
TUUVM_API bool tuuvm_astNode_isMakeAssociationNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this a make byte array node?
 */ 
TUUVM_API bool tuuvm_astNode_isMakeByteArrayNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this a make dictionary node?
 */ 
TUUVM_API bool tuuvm_astNode_isMakeDictionaryNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this a make tuple node?
 */ 
TUUVM_API bool tuuvm_astNode_isMakeTupleNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this a message send node?
 */ 
TUUVM_API bool tuuvm_astNode_isMessageSendNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this a message chain node?
 */ 
TUUVM_API bool tuuvm_astNode_isMessageChainNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this a message chain message node?
 */ 
TUUVM_API bool tuuvm_astNode_isMessageChainMessageNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this an object with lookup starting from node?
 */ 
TUUVM_API bool tuuvm_astNode_isObjectWithLookupStartingFromNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this a pragma node?
 */ 
TUUVM_API bool tuuvm_astNode_isPragmaNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this a return node?
 */ 
TUUVM_API bool tuuvm_astNode_isReturnNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this a sequence node?
 */ 
TUUVM_API bool tuuvm_astNode_isSequenceNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this an unexpanded application node?
 */ 
TUUVM_API bool tuuvm_astNode_isUnexpandedApplicationNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this an unexpanded s-expression node?
 */ 
TUUVM_API bool tuuvm_astNode_isUnexpandedSExpressionNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this a quote node?
 */ 
TUUVM_API bool tuuvm_astNode_isQuoteNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this a quasi quote node?
 */ 
TUUVM_API bool tuuvm_astNode_isQuasiQuoteNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this a quasi unquote node?
 */ 
TUUVM_API bool tuuvm_astNode_isQuasiUnquoteNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this a splice node?
 */ 
TUUVM_API bool tuuvm_astNode_isSpliceNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this a macro evaluable expression?
 */ 
TUUVM_API bool tuuvm_astNode_isMacroExpression(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Is this an while continue with node?
 */ 
TUUVM_API bool tuuvm_astNode_isWhileContinueWithNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Gets the source position from the ast node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astNode_getSourcePosition(tuuvm_tuple_t node);

/**
 * Gets the analyzed type from the ast node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astNode_getAnalyzedType(tuuvm_tuple_t node);

/**
 * Creates an argument node
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astArgumentNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t isForAll, tuuvm_tuple_t name, tuuvm_tuple_t type);

/**
 * Is this a for all argument? 
 */
TUUVM_INLINE bool tuuvm_astArgumentNode_isForAll(tuuvm_tuple_t argumentNode)
{
    if(!tuuvm_tuple_isNonNullPointer(argumentNode)) return false;
    return tuuvm_tuple_boolean_decode(((tuuvm_astArgumentNode_t*)argumentNode)->isForAll);
}

/**
 * Creates a binary expression sequence node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astBinaryExpressionSequenceNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t operands, tuuvm_tuple_t operators);

/**
 * Creates a break node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astBreakNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition);

/**
 * Creates a continue node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astContinueNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition);

/**
 * Creates a do while node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astDoWhileContinueWithNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t bodyExpression, tuuvm_tuple_t conditionExpression, tuuvm_tuple_t continueExpression);

/**
 * Gets the condition expression from a while node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astDoWhileContinueWithNode_getConditionExpression(tuuvm_tuple_t node);

/**
 * Gets the body expression from a while node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astDoWhileContinueWithNode_getBodyExpression(tuuvm_tuple_t node);

/**
 * Gets the continue expression from a while node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astDoWhileContinueWithNode_getContinueExpression(tuuvm_tuple_t node);

/**
 * Creates an error node
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astErrorNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t errorMessage);

/**
 * Creates an error node
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astErrorNode_createWithCString(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, const char *errorMessage);

/**
 * Creates a function application node
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astFunctionApplicationNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t functionExpression, tuuvm_tuple_t arguments);

/**
 * Creates an identifier reference node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astIdentifierReferenceNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t value);

/**
 * Gets the value from an identifier reference node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astIdentifierReferenceNode_getValue(tuuvm_tuple_t node);

/**
 * Is this an ellipsis identifier?
 */ 
TUUVM_API bool tuuvm_astIdentifierReferenceNode_isEllipsis(tuuvm_tuple_t node);

/**
 * Creates an if node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astIfNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t conditionExpression, tuuvm_tuple_t trueExpression, tuuvm_tuple_t falseExpression);

/**
 * Gets the condition expression from an if node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astIfNode_getConditionExpression(tuuvm_tuple_t node);

/**
 * Gets the true expression from an if node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astIfNode_getTrueExpression(tuuvm_tuple_t node);

/**
 * Gets the false expression from an if node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astIfNode_getFalseExpression(tuuvm_tuple_t node);

/**
 * Creates a lambda node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astLambdaNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t flags, tuuvm_tuple_t arguments, tuuvm_tuple_t resultType, tuuvm_tuple_t body);

/**
 * Gets the flags from the lambda node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astLambdaNode_getFlags(tuuvm_tuple_t node);

/**
 * Gets the arguments from the lambda node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astLambdaNode_getArguments(tuuvm_tuple_t node);

/**
 * Gets the body from the lambda node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astLambdaNode_getBody(tuuvm_tuple_t node);

/**
 * Creates a lexical block node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astLexicalBlockNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t body);

/**
 * Creates a literal node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astLiteralNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t value);

/**
 * Gets the value from a literal node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astLiteralNode_getValue(tuuvm_tuple_t node);

/**
 * Creates a local definition node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astLocalDefinitionNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t nameExpression, tuuvm_tuple_t typeExpression, tuuvm_tuple_t valueExpression, bool isMutable);

/**
 * Creates a local definition node for a macro symbol.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astLocalDefinitionNode_createMacro(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t nameExpression, tuuvm_tuple_t typeExpression, tuuvm_tuple_t valueExpression);

/**
 * Gets the value from a local definition node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astLocalDefinitionNode_getNameExpression(tuuvm_tuple_t node);

/**
 * Gets the type from a local definition node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astLocalDefinitionNode_getTypeExpression(tuuvm_tuple_t node);

/**
 * Gets the value from a local definition node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astLocalDefinitionNode_getValueExpression(tuuvm_tuple_t node);

/**
 * Creates a make association node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astMakeAssociationNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t key, tuuvm_tuple_t value);

/**
 * Creates a make byte array node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astMakeByteArrayNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t elements);

/**
 * Creates a make dictionary node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astMakeDictionaryNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t elements);

/**
 * Creates a make tuple node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astMakeTupleNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t elements);

/**
 * Creates a message send node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astMessageSendNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t receiver, tuuvm_tuple_t selector, tuuvm_tuple_t arguments);

/**
 * Creates a message chain node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astMessageChainNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t receiver, tuuvm_tuple_t messages);

/**
 * Creates a message chain node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astMessageChainMessageNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t selector, tuuvm_tuple_t arguments);

/**
 * Creates an object with lookup starting from node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astObjectWithLookupStartingFrom_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t objectExpression, tuuvm_tuple_t typeExpression);

/**
 * Creates a pragma node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astPragmaNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t selector, tuuvm_tuple_t arguments);

/**
 * Creates a return node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astReturnNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t expression);

/**
 * Creates a sequence node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astSequenceNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t pragmas, tuuvm_tuple_t expressions);

/**
 * Gets the number of expressions in the sequence node.
 */
TUUVM_API size_t tuuvm_astSequenceNode_getExpressionCount(tuuvm_tuple_t sequenceNode);

/**
 * Gets an expression with the given index in the sequence node.
 */
TUUVM_API tuuvm_tuple_t tuuvm_astSequenceNode_getExpressionAt(tuuvm_tuple_t sequenceNode, size_t index);

/**
 * Creates an unexpanded application node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astUnexpandedApplicationNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t functionOrMacroExpression, tuuvm_tuple_t arguments);

/**
 * Gets the function or macro expression from the unexpanded application node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astUnexpandedApplicationNode_getFunctionOrMacroExpression(tuuvm_tuple_t unexpandedApplication);

/**
 * Gets the arguments from the unexpanded application node
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astUnexpandedApplicationNode_getArguments(tuuvm_tuple_t unexpandedApplication);

/**
 * Creates an unexpanded s-expression node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astUnexpandedSExpressionNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t elements);

/**
 * Gets the elements from the unexpanded s-expression node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astUnexpandedSExpressionNode_getElements(tuuvm_tuple_t unexpandedSExpressionNode);

/**
 * Creates a quote node
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astQuoteNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t node);

/**
 * Creates a quasi quote node
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astQuasiQuoteNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t node);

/**
 * Creates a quasi unquote node
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astQuasiUnquoteNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t expression);

/**
 * Creates a splice node
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astSpliceNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t expression);

/**
 * Creates a while node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astWhileContinueWithNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t conditionExpression, tuuvm_tuple_t bodyExpression, tuuvm_tuple_t continueExpression);

/**
 * Gets the condition expression from a while node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astWhileContinueWithNode_getConditionExpression(tuuvm_tuple_t node);

/**
 * Gets the body expression from a while node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astWhileContinueWithNode_getBodyExpression(tuuvm_tuple_t node);

/**
 * Gets the continue expression from a while node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astWhileContinueWithNode_getContinueExpression(tuuvm_tuple_t node);

#endif //TUUVM_AST_H
