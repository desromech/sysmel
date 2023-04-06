#include "sysbvm/sourcePosition.h"
#include "sysbvm/sourceCode.h"
#include "internal/context.h"

SYSBVM_API sysbvm_tuple_t sysbvm_sourcePosition_create(sysbvm_context_t *context, sysbvm_tuple_t sourceCode, sysbvm_tuple_t startIndex, sysbvm_tuple_t endIndex)
{
    sysbvm_sourcePosition_t *result = (sysbvm_sourcePosition_t*)sysbvm_context_allocatePointerTuple(context, context->roots.sourcePositionType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_sourcePosition_t));
    result->sourceCode = sourceCode;
    result->startIndex = startIndex;
    result->endIndex = endIndex;

    sysbvm_sourceCode_computeLineAndColumnForIndex(context, sourceCode, startIndex, &result->startLine, &result->startColumn);
    sysbvm_sourceCode_computeLineAndColumnForIndex(context, sourceCode, endIndex, &result->endLine, &result->endColumn);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_sourcePosition_createWithIndices(sysbvm_context_t *context, sysbvm_tuple_t sourceCode, size_t startIndex, size_t endIndex)
{
    return sysbvm_sourcePosition_create(context, sourceCode, sysbvm_tuple_size_encode(context, startIndex), sysbvm_tuple_size_encode(context, endIndex));
}

SYSBVM_API sysbvm_tuple_t sysbvm_sourcePosition_createWithUnion(sysbvm_context_t *context, sysbvm_tuple_t startSourcePosition, sysbvm_tuple_t endSourcePosition)
{
    sysbvm_sourcePosition_t *start = (sysbvm_sourcePosition_t*)startSourcePosition;
    sysbvm_sourcePosition_t *end = (sysbvm_sourcePosition_t*)endSourcePosition;
    return sysbvm_sourcePosition_create(context, start->sourceCode, start->startIndex, end->endIndex);
}