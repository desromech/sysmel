#include "sysbvm/boolean.h"
#include "sysbvm/errors.h"
#include "sysbvm/function.h"
#include "sysbvm/string.h"
#include "sysbvm/type.h"
#include "internal/context.h"

static sysbvm_tuple_t sysbvm_boolean_primitive_not(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_boolean_encode(!sysbvm_tuple_boolean_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_boolean_primitive_xor(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    return sysbvm_tuple_boolean_encode(sysbvm_tuple_boolean_decode(arguments[0]) ^ sysbvm_tuple_boolean_decode(arguments[1]));
}

void sysbvm_boolean_registerPrimitives(void)
{
    sysbvm_primitiveTable_registerFunction(sysbvm_boolean_primitive_not, "Boolean::not");
    sysbvm_primitiveTable_registerFunction(sysbvm_boolean_primitive_xor, "Boolean::xor");
}

void sysbvm_boolean_setupPrimitives(sysbvm_context_t *context)
{
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.booleanType, "not", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_boolean_primitive_not);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.booleanType, "xor:",2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_boolean_primitive_xor);
}
