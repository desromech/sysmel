#ifndef TUUVM_AST_H
#define TUUVM_AST_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef struct tuuvm_astNode_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t sourcePosition;
} tuuvm_astNode_t;

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
 * Creates an unexpanded application node
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_astUnexpandedApplicationNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t functionOrMacroExpression, tuuvm_tuple_t arguments);

#endif //TUUVM_AST_H
