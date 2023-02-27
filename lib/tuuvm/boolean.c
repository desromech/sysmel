#include "tuuvm/boolean.h"
#include "tuuvm/errors.h"
#include "tuuvm/function.h"
#include "tuuvm/string.h"
#include "tuuvm/type.h"
#include "internal/context.h"

static tuuvm_tuple_t tuuvm_boolean_primitive_not(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_boolean_encode(!tuuvm_tuple_boolean_decode(arguments[0]));
}

static tuuvm_tuple_t tuuvm_boolean_primitive_xor(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return tuuvm_tuple_boolean_encode(tuuvm_tuple_boolean_decode(arguments[0]) ^ tuuvm_tuple_boolean_decode(arguments[1]));
}

void tuuvm_boolean_setupPrimitives(tuuvm_context_t *context)
{
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveMethod(context, "Boolean::not", context->roots.booleanType, "not", 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_boolean_primitive_not);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveMethod(context, "Boolean::xor:", context->roots.booleanType, "xor:",2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_boolean_primitive_xor);
}
