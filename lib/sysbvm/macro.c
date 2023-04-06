#include "sysbvm/macro.h"
#include "internal/context.h"
#include <stdlib.h>

SYSBVM_API sysbvm_tuple_t sysbvm_generatedSymbol_create(sysbvm_context_t *context, sysbvm_tuple_t value, sysbvm_tuple_t sourcePosition)
{
    sysbvm_generatedSymbol_t *result = (sysbvm_generatedSymbol_t*)sysbvm_context_allocatePointerTuple(context, context->roots.generatedSymbolType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_generatedSymbol_t));
    result->value = value;
    result->sourcePosition = sourcePosition;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_macroContext_create(sysbvm_context_t *context, sysbvm_tuple_t sourceNode, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t environment)
{
    sysbvm_macroContext_t *result = (sysbvm_macroContext_t*)sysbvm_context_allocatePointerTuple(context, context->roots.macroContextType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_macroContext_t));
    result->sourceNode = sourceNode;
    result->sourcePosition = sourcePosition;
    result->environment = environment;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_macroContext_getSourceNode(sysbvm_tuple_t macroContext)
{
    if(!sysbvm_tuple_isNonNullPointer(macroContext)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_macroContext_t*)macroContext)->sourceNode;
}

SYSBVM_API sysbvm_tuple_t sysbvm_macroContext_getSourcePosition(sysbvm_tuple_t macroContext)
{
    if(!sysbvm_tuple_isNonNullPointer(macroContext)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_macroContext_t*)macroContext)->sourcePosition;
}

SYSBVM_API sysbvm_tuple_t sysbvm_macroContext_getEnvironment(sysbvm_tuple_t macroContext)
{
    if(!sysbvm_tuple_isNonNullPointer(macroContext)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_macroContext_t*)macroContext)->environment;
}
