#include "tuuvm/function.h"
#include "internal/context.h"
#include <stdlib.h>

TUUVM_API tuuvm_tuple_t tuuvm_function_createPrimitive(tuuvm_context_t *context, void *userdata, tuuvm_functionEntryPoint_t entryPoint)
{
    tuuvm_primitiveFunction_t *result = (tuuvm_primitiveFunction_t*)tuuvm_context_allocateByteTuple(context, context->roots.primitiveFunctionType, TUUVM_BYTE_SIZE_FOR_STRUCTURE_TYPE(tuuvm_primitiveFunction_t));
    result->userdata = userdata;
    result->entryPoint = entryPoint;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_function_apply(tuuvm_context_t *context, tuuvm_tuple_t function, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    tuuvm_tuple_t functionType = tuuvm_tuple_getType(context, function);
    if(functionType == context->roots.primitiveFunctionType)
    {
        tuuvm_primitiveFunction_t *primitiveFunction = (tuuvm_primitiveFunction_t*)function;
        return primitiveFunction->entryPoint(context, function, argumentCount, arguments);
    }

    abort();
}