#include "tuuvm/io.h"
#include "tuuvm/context.h"
#include "tuuvm/errors.h"
#include "tuuvm/function.h"
#include "tuuvm/string.h"
#include <stdio.h>

static tuuvm_tuple_t tuuvm_io_primitivePrintLine(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    printf("TODO: tuuvm_io_primitivePrintLine\n");

    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_io_primitivePrint(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    printf("TODO: tuuvm_io_primitivePrint\n");

    return TUUVM_VOID_TUPLE;
}

void tuuvm_io_setupPrimitives(tuuvm_context_t *context)
{
    tuuvm_context_setIntrinsicSymbolBinding(context, tuuvm_symbol_internWithCString(context, "printLine"), tuuvm_function_createPrimitive(context, 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_io_primitivePrintLine));
    tuuvm_context_setIntrinsicSymbolBinding(context, tuuvm_symbol_internWithCString(context, "print"), tuuvm_function_createPrimitive(context, 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_io_primitivePrint));
}
