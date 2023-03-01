#include "tuuvm/io.h"
#include "tuuvm/context.h"
#include "tuuvm/errors.h"
#include "tuuvm/function.h"
#include "tuuvm/string.h"
#include "tuuvm/assert.h"
#include <stdio.h>

static tuuvm_tuple_t tuuvm_io_primitive_printLine(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    tuuvm_tuple_t string = tuuvm_tuple_asString(context, arguments[0]);
    TUUVM_ASSERT(tuuvm_tuple_isBytes(string));
    fwrite(TUUVM_CAST_OOP_TO_OBJECT_TUPLE(string)->bytes, tuuvm_tuple_getSizeInBytes(string), 1, stdout);
    fwrite("\n", 1, 1, stdout);

    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_io_primitive_print(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    tuuvm_tuple_t string = tuuvm_tuple_asString(context, arguments[0]);
    TUUVM_ASSERT(tuuvm_tuple_isBytes(string));
    fwrite(TUUVM_CAST_OOP_TO_OBJECT_TUPLE(string)->bytes, tuuvm_tuple_getSizeInBytes(string), 1, stdout);

    return TUUVM_VOID_TUPLE;
}

void tuuvm_io_setupPrimitives(tuuvm_context_t *context)
{
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "printLine", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_io_primitive_printLine);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "print", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_io_primitive_print);
}
