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
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_sourcePosition_createWithIndices(sysbvm_context_t *context, sysbvm_tuple_t sourceCode, size_t startIndex, size_t endIndex)
{
    return sysbvm_sourcePosition_create(context, sourceCode, sysbvm_tuple_uint32_encode(context, startIndex), sysbvm_tuple_uint32_encode(context, endIndex));
}

SYSBVM_API sysbvm_tuple_t sysbvm_sourcePosition_createWithUnion(sysbvm_context_t *context, sysbvm_tuple_t startSourcePosition, sysbvm_tuple_t endSourcePosition)
{
    sysbvm_sourcePosition_t *start = (sysbvm_sourcePosition_t*)startSourcePosition;
    sysbvm_sourcePosition_t *end = (sysbvm_sourcePosition_t*)endSourcePosition;
    return sysbvm_sourcePosition_create(context, start->sourceCode, start->startIndex, end->endIndex);
}

SYSBVM_API bool sysbvm_sourcePosition_getStartLineAndColumn(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, uint32_t *outLine, uint32_t *outColumn)
{
    if(!sysbvm_tuple_isNonNullPointer(sourcePosition))
        return false;

    sysbvm_sourcePosition_t *sourcePositionObject = (sysbvm_sourcePosition_t*)sourcePosition;
    sysbvm_sourceCode_computeLineAndColumnForIndex(context, sourcePositionObject->sourceCode, sourcePositionObject->startIndex, outLine, outColumn);
    return true;
}

SYSBVM_API void sysbvm_sourcePosition_dump(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition)
{
    if(!sysbvm_tuple_isNonNullPointer(sourcePosition)) return;

    sysbvm_sourcePosition_t *sourcePositionObject = (sysbvm_sourcePosition_t*)sourcePosition;
    uint32_t startLine = 0;
    uint32_t startColumn = 0;
    uint32_t endLine = 0;
    uint32_t endColumn = 0;
    sysbvm_sourceCode_computeLineAndColumnForIndex(context, sourcePositionObject->sourceCode, sourcePositionObject->startIndex, &startLine, &startColumn);
    sysbvm_sourceCode_computeLineAndColumnForIndex(context, sourcePositionObject->sourceCode, sourcePositionObject->endIndex, &endLine, &endColumn);
    
    if(sysbvm_tuple_isNonNullPointer(sourcePositionObject->sourceCode))
    {
        sysbvm_sourceCode_t *sourceCode = (sysbvm_sourceCode_t*)sourcePositionObject->sourceCode;
        printf(SYSBVM_STRING_PRINTF_FORMAT ":%d.%d-%d.%d\n", SYSBVM_STRING_PRINTF_ARG(sourceCode->name),
            startLine, startColumn, endLine, endColumn);
    }
    else
    {
        printf("unknown:%d.%d-%d.%d\n", startLine, startColumn, endLine, endColumn);
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
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

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

SYSBVM_API sysbvm_tuple_t sysbvm_sourcePosition_primitive_startLine(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)argumentCount;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_sourcePosition_t *sourcePosition = (sysbvm_sourcePosition_t*)arguments[0];
    uint32_t startLine = 0;
    sysbvm_sourceCode_computeLineAndColumnForIndex(context, sourcePosition->sourceCode, sourcePosition->startIndex, &startLine, NULL);
    return sysbvm_tuple_uint32_encode(context, startLine);
}

SYSBVM_API sysbvm_tuple_t sysbvm_sourcePosition_primitive_startColumn(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)argumentCount;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_sourcePosition_t *sourcePosition = (sysbvm_sourcePosition_t*)arguments[0];
    uint32_t startColumn = 0;
    sysbvm_sourceCode_computeLineAndColumnForIndex(context, sourcePosition->sourceCode, sourcePosition->startIndex, NULL, &startColumn);
    return sysbvm_tuple_uint32_encode(context, startColumn);
}

SYSBVM_API sysbvm_tuple_t sysbvm_sourcePosition_primitive_endLine(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)argumentCount;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_sourcePosition_t *sourcePosition = (sysbvm_sourcePosition_t*)arguments[0];
    uint32_t endLine = 0;
    sysbvm_sourceCode_computeLineAndColumnForIndex(context, sourcePosition->sourceCode, sourcePosition->endIndex, &endLine, NULL);
    return sysbvm_tuple_uint32_encode(context, endLine);
}

SYSBVM_API sysbvm_tuple_t sysbvm_sourcePosition_primitive_endColumn(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)argumentCount;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_sourcePosition_t *sourcePosition = (sysbvm_sourcePosition_t*)arguments[0];
    uint32_t endColumn = 0;
    sysbvm_sourceCode_computeLineAndColumnForIndex(context, sourcePosition->sourceCode, sourcePosition->endIndex, NULL, &endColumn);
    return sysbvm_tuple_uint32_encode(context, endColumn);
}

void sysbvm_sourcePosition_registerPrimitives(void)
{
    sysbvm_primitiveTable_registerFunction(sysbvm_sourcePosition_primitive_hash, "SourcePosition::hash");
    sysbvm_primitiveTable_registerFunction(sysbvm_sourcePosition_primitive_equals, "SourcePosition::=");
    sysbvm_primitiveTable_registerFunction(sysbvm_sourcePosition_primitive_startLine, "SourcePosition::startLine");
    sysbvm_primitiveTable_registerFunction(sysbvm_sourcePosition_primitive_startColumn, "SourcePosition::startColumn");
    sysbvm_primitiveTable_registerFunction(sysbvm_sourcePosition_primitive_endLine, "SourcePosition::endLine");
    sysbvm_primitiveTable_registerFunction(sysbvm_sourcePosition_primitive_endColumn, "SourcePosition::endColumn");
}

void sysbvm_sourcePosition_setupPrimitives(sysbvm_context_t *context)
{
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.sourcePositionType, "hash", 1, SYSBVM_FUNCTION_FLAGS_OVERRIDE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_sourcePosition_primitive_hash);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.sourcePositionType, "=", 2, SYSBVM_FUNCTION_FLAGS_OVERRIDE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_sourcePosition_primitive_equals);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.sourcePositionType, "startLine", 1, SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_sourcePosition_primitive_startLine);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.sourcePositionType, "startColumn", 1, SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_sourcePosition_primitive_startColumn);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.sourcePositionType, "endLine", 1, SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_sourcePosition_primitive_endLine);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.sourcePositionType, "endColumn", 1, SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_sourcePosition_primitive_endColumn);
}
