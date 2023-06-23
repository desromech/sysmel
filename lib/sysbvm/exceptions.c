#include "sysbvm/exceptions.h"
#include "sysbvm/assert.h"
#include "sysbvm/context.h"
#include "sysbvm/function.h"
#include "sysbvm/stackFrame.h"
#include "sysbvm/string.h"
#include "internal/context.h"
#include <stdio.h>
#include <stdlib.h>

SYSBVM_API sysbvm_tuple_t sysbvm_exception_signal(sysbvm_context_t *context, sysbvm_tuple_t exception)
{
    (void)context;
    sysbvm_stackFrame_raiseException(exception);
    return SYSBVM_NULL_TUPLE;
}

SYSBVM_API sysbvm_tuple_t sysbvm_error_createWithMessageText(sysbvm_context_t *context, sysbvm_tuple_t messageText)
{
    sysbvm_error_t *result = (sysbvm_error_t*)sysbvm_context_allocatePointerTuple(context, context->roots.errorType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_error_t));
    result->messageText = messageText;
    return (sysbvm_tuple_t)result;
}

static sysbvm_tuple_t sysbvm_exception_primitive_printString(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_exception_t *exceptionObject = (sysbvm_exception_t*)arguments[0];
    if(exceptionObject->messageText)
        return exceptionObject->messageText;

    return sysbvm_string_createWithCString(context, "An exception");
}

static sysbvm_tuple_t sysbvm_exception_primitive_ensure(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *body = arguments + 0;
    sysbvm_tuple_t *block = arguments + 1;

    struct {
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    sysbvm_stackFrameCleanupRecord_t cleanUpRecord = {
        .type = SYSBVM_STACK_FRAME_RECORD_TYPE_CLEANUP,
        .action = *block,
    };
    sysbvm_stackFrame_pushRecord((sysbvm_stackFrameRecord_t*)&cleanUpRecord);

    gcFrame.result = sysbvm_function_apply0(context, *body);
    sysbvm_stackFrame_popRecord((sysbvm_stackFrameRecord_t*)&cleanUpRecord);

    sysbvm_function_apply0(context, cleanUpRecord.action);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_exception_primitive_onDo(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_tuple_t *body = arguments + 0;
    sysbvm_tuple_t *exceptionType = arguments + 1;
    sysbvm_tuple_t *exceptionAction = arguments + 2;

    struct {
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    sysbvm_stackFrameLandingPadRecord_t landingPadRecord = {
        .type = SYSBVM_STACK_FRAME_RECORD_TYPE_LANDING_PAD,
        .exceptionFilter = *exceptionType,
        .action = *exceptionAction
    };
    sysbvm_stackFrame_pushRecord((sysbvm_stackFrameRecord_t*)&landingPadRecord);

    if(!_setjmp(landingPadRecord.jmpbuffer))
        gcFrame.result = sysbvm_function_apply0(context, *body);
    else
        gcFrame.result = landingPadRecord.actionResult;

    sysbvm_stackFrame_popRecord((sysbvm_stackFrameRecord_t*)&landingPadRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_exception_primitive_signal(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_exception_signal(context, arguments[0]);
}

void sysbvm_exceptions_registerPrimitives(void)
{
    sysbvm_primitiveTable_registerFunction(sysbvm_exception_primitive_printString, "Exception::printString");
    sysbvm_primitiveTable_registerFunction(sysbvm_exception_primitive_signal, "Exception::signal");

    sysbvm_primitiveTable_registerFunction(sysbvm_exception_primitive_ensure, "Function::ensure:");
    sysbvm_primitiveTable_registerFunction(sysbvm_exception_primitive_onDo, "Function::on:do:");
}

void sysbvm_exceptions_setupPrimitives(sysbvm_context_t *context)
{
    sysbvm_type_setAsStringFunction(context, context->roots.exceptionType, sysbvm_function_createPrimitive(context, 1, SYSBVM_FUNCTION_FLAGS_OVERRIDE, NULL, sysbvm_exception_primitive_printString));
    sysbvm_type_setPrintStringFunction(context, context->roots.exceptionType, sysbvm_function_createPrimitive(context, 1, SYSBVM_FUNCTION_FLAGS_OVERRIDE, NULL, sysbvm_exception_primitive_signal));

    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Exception::signal", context->roots.exceptionType, "signal", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_exception_primitive_signal);

    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Function::ensure:", context->roots.functionType, "ensure:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_exception_primitive_ensure);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Function::on:do:", context->roots.functionType, "on:do:", 3, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_exception_primitive_onDo);
}
