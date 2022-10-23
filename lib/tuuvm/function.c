#include "tuuvm/function.h"
#include "tuuvm/arraySlice.h"
#include "tuuvm/interpreter.h"
#include "internal/context.h"
#include <stdlib.h>

TUUVM_API tuuvm_tuple_t tuuvm_function_createPrimitive(tuuvm_context_t *context, size_t argumentCount, size_t flags, void *userdata, tuuvm_functionEntryPoint_t entryPoint)
{
    tuuvm_primitiveFunction_t *result = (tuuvm_primitiveFunction_t*)tuuvm_context_allocateByteTuple(context, context->roots.primitiveFunctionType, TUUVM_BYTE_SIZE_FOR_STRUCTURE_TYPE(tuuvm_primitiveFunction_t));
    result->argumentCount = argumentCount;
    result->flags = flags;
    result->userdata = userdata;
    result->entryPoint = entryPoint;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_function_createClosureAST(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t flags, tuuvm_tuple_t closureEnvironment, tuuvm_tuple_t argumentSymbols, tuuvm_tuple_t body)
{
    tuuvm_closureASTFunction_t *result = (tuuvm_closureASTFunction_t*)tuuvm_context_allocateByteTuple(context, context->roots.closureASTFunctionType, TUUVM_BYTE_SIZE_FOR_STRUCTURE_TYPE(tuuvm_closureASTFunction_t));
    result->sourcePosition = sourcePosition;
    result->flags = flags;
    result->closureEnvironment = closureEnvironment;
    result->argumentSymbols = argumentSymbols;
    result->body = body;
    return (tuuvm_tuple_t)result;
}

TUUVM_API size_t tuuvm_function_getArgumentCount(tuuvm_context_t *context, tuuvm_tuple_t function)
{
    tuuvm_tuple_t functionType = tuuvm_tuple_getType(context, function);
    if(functionType == context->roots.primitiveFunctionType)
    {
        tuuvm_primitiveFunction_t *primitiveFunction = (tuuvm_primitiveFunction_t*)function;
        return primitiveFunction->argumentCount;
    }
    else if(functionType == context->roots.closureASTFunctionType)
    {
        tuuvm_closureASTFunction_t *closureASTFunction = (tuuvm_closureASTFunction_t*)function;
        return tuuvm_arraySlice_getSize(closureASTFunction->argumentSymbols);
    }

    return 0;
}

TUUVM_API size_t tuuvm_function_getFlags(tuuvm_context_t *context, tuuvm_tuple_t function)
{
    tuuvm_tuple_t functionType = tuuvm_tuple_getType(context, function);
    if(functionType == context->roots.primitiveFunctionType)
    {
        tuuvm_primitiveFunction_t *primitiveFunction = (tuuvm_primitiveFunction_t*)function;
        return primitiveFunction->flags;
    }
    else if(functionType == context->roots.closureASTFunctionType)
    {
        tuuvm_closureASTFunction_t *closureASTFunction = (tuuvm_closureASTFunction_t*)function;
        return tuuvm_tuple_size_decode(closureASTFunction->flags);
    }

    return 0;
}

TUUVM_API tuuvm_tuple_t tuuvm_function_apply(tuuvm_context_t *context, tuuvm_tuple_t function, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    tuuvm_tuple_t functionType = tuuvm_tuple_getType(context, function);
    if(functionType == context->roots.primitiveFunctionType)
    {
        tuuvm_primitiveFunction_t *primitiveFunction = (tuuvm_primitiveFunction_t*)function;
        return primitiveFunction->entryPoint(context, function, argumentCount, arguments);
    }
    else if(functionType == context->roots.closureASTFunctionType)
    {
        return tuuvm_interpreter_applyClosureASTFunction(context, function, argumentCount, arguments);
    }

    abort();
}
