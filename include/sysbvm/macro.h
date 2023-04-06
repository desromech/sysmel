#ifndef SYSBVM_MACRO_CONTEXT_H
#define SYSBVM_MACRO_CONTEXT_H

#pragma once

#include "tuple.h"

typedef struct sysbvm_context_s sysbvm_context_t;

typedef struct sysbvm_generatedSymbol_s
{
    sysbvm_tuple_header_t header;
    sysbvm_tuple_t value;
    sysbvm_tuple_t sourcePosition;
} sysbvm_generatedSymbol_t;

typedef struct sysbvm_macroContext_s
{
    sysbvm_tuple_header_t header;
    sysbvm_tuple_t sourceNode;
    sysbvm_tuple_t sourcePosition;
    sysbvm_tuple_t environment;
} sysbvm_macroContext_t;

/**
 * Creates a generated symbol.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_generatedSymbol_create(sysbvm_context_t *context, sysbvm_tuple_t value, sysbvm_tuple_t sourcePosition);

/**
 * Creates a macro context.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_macroContext_create(sysbvm_context_t *context, sysbvm_tuple_t sourceNode, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t environment);

/**
 * Gets the macro context source node.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_macroContext_getSourceNode(sysbvm_tuple_t macroContext);

/**
 * Gets the macro context source position.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_macroContext_getSourcePosition(sysbvm_tuple_t macroContext);

/**
 * Gets the macro context source position.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_macroContext_getEnvironment(sysbvm_tuple_t macroContext);

#endif //SYSBVM_MACRO_CONTEXT_H
