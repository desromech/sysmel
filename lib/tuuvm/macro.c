#include "tuuvm/macro.h"
#include "internal/context.h"
#include <stdlib.h>

TUUVM_API tuuvm_tuple_t tuuvm_macroContext_create(tuuvm_context_t *context, tuuvm_tuple_t sourceNode, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t environment)
{
    tuuvm_macroContext_t *result = (tuuvm_macroContext_t*)tuuvm_context_allocatePointerTuple(context, context->roots.macroContextType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_macroContext_t));
    result->sourceNode = sourceNode;
    result->sourcePosition = sourcePosition;
    result->environment = environment;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_macroContext_getSourceNode(tuuvm_tuple_t macroContext)
{
    if(!tuuvm_tuple_isNonNullPointer(macroContext)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_macroContext_t*)macroContext)->sourceNode;
}

TUUVM_API tuuvm_tuple_t tuuvm_macroContext_getSourcePosition(tuuvm_tuple_t macroContext)
{
    if(!tuuvm_tuple_isNonNullPointer(macroContext)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_macroContext_t*)macroContext)->sourcePosition;
}

TUUVM_API tuuvm_tuple_t tuuvm_macroContext_getEnvironment(tuuvm_tuple_t macroContext)
{
    if(!tuuvm_tuple_isNonNullPointer(macroContext)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_macroContext_t*)macroContext)->environment;
}
