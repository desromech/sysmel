#include "tuuvm/macro.h"
#include "internal/context.h"
#include <stdlib.h>

TUUVM_API tuuvm_tuple_t tuuvm_macroContext_create(tuuvm_context_t *context, tuuvm_tuple_t sourceNode, tuuvm_tuple_t sourcePosition)
{
    tuuvm_macroContext_t *result = (tuuvm_macroContext_t*)tuuvm_context_allocateByteTuple(context, context->roots.macroContextType, TUUVM_BYTE_SIZE_FOR_STRUCTURE_TYPE(tuuvm_macroContext_t));
    result->sourceNode = sourceNode;
    result->sourcePosition = sourcePosition;
    return (tuuvm_tuple_t)result;
}
