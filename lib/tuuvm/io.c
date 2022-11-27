#include "tuuvm/io.h"
#include "tuuvm/context.h"
#include "tuuvm/errors.h"
#include "tuuvm/function.h"
#include "tuuvm/string.h"
#include "tuuvm/assert.h"
#include <stdio.h>

static tuuvm_tuple_t tuuvm_io_primitivePrintLine(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    tuuvm_tuple_t string = tuuvm_tuple_toString(context, arguments[0]);
    TUUVM_ASSERT(tuuvm_tuple_isBytes(string));
    fwrite(TUUVM_CAST_OOP_TO_OBJECT_TUPLE(string)->bytes, tuuvm_tuple_getSizeInBytes(string), 1, stdout);
    fwrite("\n", 1, 1, stdout);

    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_io_primitivePrint(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    tuuvm_tuple_t string = tuuvm_tuple_toString(context, arguments[0]);
    TUUVM_ASSERT(tuuvm_tuple_isBytes(string));
    fwrite(TUUVM_CAST_OOP_TO_OBJECT_TUPLE(string)->bytes, tuuvm_tuple_getSizeInBytes(string), 1, stdout);

    return TUUVM_VOID_TUPLE;
}

void tuuvm_io_setupPrimitives(tuuvm_context_t *context)
{
    tuuvm_context_setIntrinsicSymbolBinding(context, tuuvm_symbol_internWithCString(context, "printLine"), tuuvm_function_createPrimitive(context, 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_io_primitivePrintLine));
    tuuvm_context_setIntrinsicSymbolBinding(context, tuuvm_symbol_internWithCString(context, "print"), tuuvm_function_createPrimitive(context, 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_io_primitivePrint));
}
