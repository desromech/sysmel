#include "sysbvm/sourcePosition.h"
#include "sysbvm/sourceCode.h"
#include "sysbvm/errors.h"
#include "sysbvm/function.h"
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

SYSBVM_API sysbvm_tuple_t sysbvm_sourcePosition_primitive_hash(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    (void)argumentCount;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_sourcePosition_t *sourcePosition = (sysbvm_sourcePosition_t*)arguments[0];
    size_t hash = sysbvm_hashConcatenate(
        sysbvm_hashConcatenate(sysbvm_tuple_identityHash(sourcePosition->sourceCode),
        sysbvm_tuple_identityHash(sourcePosition->startIndex)),
            sysbvm_tuple_identityHash(sourcePosition->endIndex));

    return sysbvm_tuple_size_encode(context, hash);
}

SYSBVM_API sysbvm_tuple_t sysbvm_sourcePosition_primitive_equals(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)argumentCount;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(1, argumentCount);

    if(sysbvm_tuple_getType(context, arguments[0]) != sysbvm_tuple_getType(context, arguments[1]))
        return SYSBVM_FALSE_TUPLE;

    sysbvm_sourcePosition_t *leftSourcePosition = (sysbvm_sourcePosition_t*)arguments[0];
    sysbvm_sourcePosition_t *rightSourcePosition = (sysbvm_sourcePosition_t*)arguments[1];

    return sysbvm_tuple_boolean_encode(
        leftSourcePosition->sourceCode == rightSourcePosition->sourceCode &&
        leftSourcePosition->startIndex == rightSourcePosition->startIndex &&
        leftSourcePosition->endIndex == rightSourcePosition->endIndex
    );
}

void sysbvm_sourcePosition_registerPrimitives(void)
{
    sysbvm_primitiveTable_registerFunction(sysbvm_sourcePosition_primitive_hash, "SourcePosition::hash");
    sysbvm_primitiveTable_registerFunction(sysbvm_sourcePosition_primitive_equals, "SourcePosition::=");
}

void sysbvm_sourcePosition_setupPrimitives(sysbvm_context_t *context)
{
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.sourcePositionType, "hash", 1, SYSBVM_FUNCTION_FLAGS_OVERRIDE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_sourcePosition_primitive_hash);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.sourcePositionType, "=", 2, SYSBVM_FUNCTION_FLAGS_OVERRIDE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_sourcePosition_primitive_equals);
}
