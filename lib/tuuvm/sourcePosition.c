#include "tuuvm/sourcePosition.h"
#include "tuuvm/sourceCode.h"
#include "internal/context.h"

TUUVM_API tuuvm_tuple_t tuuvm_sourcePosition_create(tuuvm_context_t *context, tuuvm_tuple_t sourceCode, tuuvm_tuple_t startIndex, tuuvm_tuple_t endIndex)
{
    tuuvm_sourcePosition_t *result = (tuuvm_sourcePosition_t*)tuuvm_context_allocatePointerTuple(context, context->roots.sourcePositionType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_sourcePosition_t));
    result->sourceCode = sourceCode;
    result->startIndex = startIndex;
    result->endIndex = endIndex;

    tuuvm_sourceCode_computeLineAndColumnForIndex(context, sourceCode, startIndex, &result->startLine, &result->startColumn);
    tuuvm_sourceCode_computeLineAndColumnForIndex(context, sourceCode, endIndex, &result->endLine, &result->endColumn);
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_sourcePosition_createWithIndices(tuuvm_context_t *context, tuuvm_tuple_t sourceCode, size_t startIndex, size_t endIndex)
{
    return tuuvm_sourcePosition_create(context, sourceCode, tuuvm_tuple_size_encode(context, startIndex), tuuvm_tuple_size_encode(context, endIndex));
}

TUUVM_API tuuvm_tuple_t tuuvm_sourcePosition_createWithUnion(tuuvm_context_t *context, tuuvm_tuple_t startSourcePosition, tuuvm_tuple_t endSourcePosition)
{
    tuuvm_sourcePosition_t *start = (tuuvm_sourcePosition_t*)startSourcePosition;
    tuuvm_sourcePosition_t *end = (tuuvm_sourcePosition_t*)endSourcePosition;
    return tuuvm_sourcePosition_create(context, start->sourceCode, start->startIndex, end->endIndex);
}