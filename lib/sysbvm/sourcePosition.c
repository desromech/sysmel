#include "sysbvm/sourcePosition.h"
#include "sysbvm/sourceCode.h"
#include "internal/context.h"
#include <stdio.h>

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

SYSBVM_API void sysbvm_sourcePosition_dump(sysbvm_tuple_t sourcePosition)
{
    if(!sysbvm_tuple_isNonNullPointer(sourcePosition)) return;

    sysbvm_sourcePosition_t *sourcePositionObject = (sysbvm_sourcePosition_t*)sourcePosition;
    sysbvm_size_t startLine = sysbvm_tuple_size_decode(sourcePositionObject->startLine);
    sysbvm_size_t startColumn = sysbvm_tuple_size_decode(sourcePositionObject->startColumn);
    sysbvm_size_t endLine = sysbvm_tuple_size_decode(sourcePositionObject->endLine);
    sysbvm_size_t endColumn = sysbvm_tuple_size_decode(sourcePositionObject->endColumn);
    
    if(sysbvm_tuple_isNonNullPointer(sourcePositionObject->sourceCode))
    {
        sysbvm_sourceCode_t *sourceCode = (sysbvm_sourceCode_t*)sourcePositionObject->sourceCode;
        printf(SYSBVM_STRING_PRINTF_FORMAT ":%d.%d-%d.%d\n", SYSBVM_STRING_PRINTF_ARG(sourceCode->name),
            (int)startLine, (int)startColumn, (int)endLine, (int)endColumn);
    }
    else
    {
        printf("unknown:%d.%d-%d.%d\n", (int)startLine, (int)startColumn, (int)endLine, (int)endColumn);
    }
}
