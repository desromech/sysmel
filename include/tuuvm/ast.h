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

typedef struct tuuvm_astLiteralNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t value;
} tuuvm_astLiteralNode_t;

typedef struct tuuvm_astIdentifierReferenceNode_s
{
    tuuvm_astNode_t super;
    tuuvm_tuple_t value;
} tuuvm_astIdentifierReferenceNode_t;

typedef struct tuuvm_astSequenceNode_s
{
    tuuvm_astNode_t super;
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
 * Is this a literal node?
 */ 
TUUVM_API bool tuuvm_astNode_isLiteralNode(tuuvm_context_t *context, tuuvm_tuple_t tuple);

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
 * Creates a literal node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astLiteralNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t value);

/**
 * Gets the value from a literal node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astLiteralNode_getValue(tuuvm_tuple_t node);

/**
 * Creates a sequence node.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astSequenceNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t expressions);

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

#endif //TUUVM_AST_H
