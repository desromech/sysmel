#ifndef TUUVM_MACRO_CONTEXT_H
#define TUUVM_MACRO_CONTEXT_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef struct tuuvm_generatedSymbol_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t value;
    tuuvm_tuple_t sourcePosition;
} tuuvm_generatedSymbol_t;

typedef struct tuuvm_macroContext_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t sourceNode;
    tuuvm_tuple_t sourcePosition;
    tuuvm_tuple_t environment;
} tuuvm_macroContext_t;

/**
 * Creates a generated symbol.
 */
TUUVM_API tuuvm_tuple_t tuuvm_generatedSymbol_create(tuuvm_context_t *context, tuuvm_tuple_t value, tuuvm_tuple_t sourcePosition);

/**
 * Creates a macro context.
 */
TUUVM_API tuuvm_tuple_t tuuvm_macroContext_create(tuuvm_context_t *context, tuuvm_tuple_t sourceNode, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t environment);

/**
 * Gets the macro context source node.
 */
TUUVM_API tuuvm_tuple_t tuuvm_macroContext_getSourceNode(tuuvm_tuple_t macroContext);

/**
 * Gets the macro context source position.
 */
TUUVM_API tuuvm_tuple_t tuuvm_macroContext_getSourcePosition(tuuvm_tuple_t macroContext);

/**
 * Gets the macro context source position.
 */
TUUVM_API tuuvm_tuple_t tuuvm_macroContext_getEnvironment(tuuvm_tuple_t macroContext);

#endif //TUUVM_MACRO_CONTEXT_H
