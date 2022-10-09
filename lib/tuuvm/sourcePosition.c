#include "tuuvm/sourcePosition.h"
#include "internal/context.h"

TUUVM_API tuuvm_tuple_t tuuvm_sourcePosition_create(tuuvm_context_t *context, tuuvm_tuple_t sourceCode, tuuvm_tuple_t startIndex, tuuvm_tuple_t endIndex)
{
    tuuvm_sourcePosition_t *result = (tuuvm_sourcePosition_t*)tuuvm_context_allocatePointerTuple(context, context->roots.sourceCodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_sourcePosition_t));
    result->sourceCode = sourceCode;
    result->startIndex = startIndex;
    result->endIndex = endIndex;
    // TODO: Compute the corresponding lines and columns.
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_sourcePosition_createWithIndices(tuuvm_context_t *context, tuuvm_tuple_t sourceCode, size_t startIndex, size_t endIndex)
{
    return tuuvm_sourcePosition_create(context, sourceCode, tuuvm_tuple_size_encode(context, startIndex), tuuvm_tuple_size_encode(context, endIndex));
}
