#include "tuuvm/errors.h"
#include "tuuvm/assert.h"
#include "tuuvm/context.h"
#include "tuuvm/function.h"
#include "tuuvm/stackFrame.h"
#include "tuuvm/string.h"
#include <stdio.h>
#include <stdlib.h>

TUUVM_API void tuuvm_errorWithMessageTuple(tuuvm_tuple_t message)
{
    tuuvm_stackFrame_raiseException(message);
}

TUUVM_API void tuuvm_error(const char *message)
{
    tuuvm_context_t *activeContext = tuuvm_stackFrame_getActiveContext();
    if(!activeContext)
    {
        fprintf(stderr, "%s\n", message);
        abort();
    }

    tuuvm_tuple_t errorString = tuuvm_string_createWithCString(activeContext, message);
    tuuvm_stackFrame_raiseException(errorString);
}

TUUVM_API void tuuvm_error_accessDummyValue()
{
    tuuvm_error("Cannot access slot of dummy value.");
}

TUUVM_API void tuuvm_error_modifyImmediateValue()
{
    tuuvm_error("Cannot modify slot of immediate value");
}

TUUVM_API void tuuvm_error_modifyImmutableTuple()
{
    tuuvm_error("Cannot modify slot of immutable tuple.");
}

TUUVM_API void tuuvm_error_assertionFailure(const char *message)
{
    tuuvm_error(message);
}

TUUVM_API void tuuvm_error_indexOutOfBounds()
{
    tuuvm_error("Index out of bounds");
}

TUUVM_API void tuuvm_error_outOfBoundsSlotAccess()
{
    tuuvm_error("Accessing a slot that is out of bounds.");
}

TUUVM_API void tuuvm_error_argumentCountMismatch(size_t expected, size_t gotten)
{
    (void)expected;
    (void)gotten;
    tuuvm_error("Argument count mismatch");
}

static tuuvm_tuple_t tuuvm_errors_primitive_error(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    tuuvm_stackFrame_raiseException(arguments[0]);
    return TUUVM_VOID_TUPLE;
}

void tuuvm_errors_registerPrimitives(void)
{
    tuuvm_primitiveTable_registerFunction(tuuvm_errors_primitive_error, "Error::signalWithMessage");
}

void tuuvm_errors_setupPrimitives(tuuvm_context_t *context)
{
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "error", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_errors_primitive_error);
}
