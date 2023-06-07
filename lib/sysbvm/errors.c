#include "sysbvm/errors.h"
#include "sysbvm/assert.h"
#include "sysbvm/context.h"
#include "sysbvm/function.h"
#include "sysbvm/stackFrame.h"
#include "sysbvm/string.h"
#include <stdio.h>
#include <stdlib.h>

SYSBVM_API void sysbvm_errorWithMessageTuple(sysbvm_tuple_t message)
{
    sysbvm_stackFrame_raiseException(message);
}

SYSBVM_API void sysbvm_error(const char *message)
{
    sysbvm_context_t *activeContext = sysbvm_stackFrame_getActiveContext();
    if(!activeContext)
    {
        fprintf(stderr, "%s\n", message);
        abort();
    }

    sysbvm_tuple_t errorString = sysbvm_string_createWithCString(activeContext, message);
    sysbvm_stackFrame_raiseException(errorString);
}

SYSBVM_API void sysbvm_error_accessDummyValue()
{
    sysbvm_error("Cannot access slot of dummy value.");
}

SYSBVM_API void sysbvm_error_modifyImmediateValue()
{
    sysbvm_error("Cannot modify slot of immediate value");
}

SYSBVM_API void sysbvm_error_modifyImmutableTuple()
{
    sysbvm_error("Cannot modify slot of immutable tuple.");
}

SYSBVM_API void sysbvm_error_assertionFailure(const char *message)
{
    sysbvm_error(message);
}

SYSBVM_API void sysbvm_error_fatalAssertionFailure(const char *message)
{
    fprintf(stderr, "%s\n", message);
    abort();
}

SYSBVM_API void sysbvm_error_indexOutOfBounds()
{
    sysbvm_error("Index out of bounds");
}

SYSBVM_API void sysbvm_error_outOfBoundsSlotAccess()
{
    sysbvm_error("Accessing a slot that is out of bounds.");
}

SYSBVM_API void sysbvm_error_trap()
{
    sysbvm_error("Trap instruction.");
}

SYSBVM_API void sysbvm_error_nullArgument()
{
    sysbvm_error("Null argument.");
}

SYSBVM_API void sysbvm_error_unexpectedType(sysbvm_tuple_t expectedType, sysbvm_tuple_t value)
{
    (void)expectedType;
    (void)value;
    sysbvm_error("Unexpected type.");
}

SYSBVM_API void sysbvm_error_argumentCountMismatch(size_t expected, size_t gotten)
{
    (void)expected;
    (void)gotten;
    sysbvm_error("Argument count mismatch");
}

SYSBVM_API void sysbvm_error_primitiveFailed(void)
{
    sysbvm_error("Primitive failed");
}

static sysbvm_tuple_t sysbvm_errors_primitive_error(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_stackFrame_raiseException(arguments[0]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_errors_primitive_primitiveFailedError(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 0) sysbvm_error_argumentCountMismatch(0, argumentCount);

    sysbvm_error_primitiveFailed();
    return SYSBVM_VOID_TUPLE;
}

void sysbvm_errors_registerPrimitives(void)
{
    sysbvm_primitiveTable_registerFunction(sysbvm_errors_primitive_error, "Error::signalWithMessage");
    sysbvm_primitiveTable_registerFunction(sysbvm_errors_primitive_primitiveFailedError, "Error::primitiveFailed");
}

void sysbvm_errors_setupPrimitives(sysbvm_context_t *context)
{
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "error", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_errors_primitive_error);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "primitiveFailedError", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_errors_primitive_primitiveFailedError);
}
