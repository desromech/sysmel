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

void tuuvm_boolean_registerPrimitives(void)
{
    tuuvm_primitiveTable_registerFunction(tuuvm_boolean_primitive_not, "Boolean::not");
    tuuvm_primitiveTable_registerFunction(tuuvm_boolean_primitive_xor, "Boolean::xor");
}

void tuuvm_boolean_setupPrimitives(tuuvm_context_t *context)
{
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Boolean::not", context->roots.booleanType, "not", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_boolean_primitive_not);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Boolean::xor:", context->roots.booleanType, "xor:",2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_boolean_primitive_xor);
}
