#include "tuuvm/environment.h"
#include "tuuvm/dictionary.h"
#include "internal/context.h"

TUUVM_API tuuvm_tuple_t tuuvm_environment_create(tuuvm_context_t *context, tuuvm_tuple_t parent)
{
    tuuvm_environment_t *result = (tuuvm_environment_t*)tuuvm_context_allocatePointerTuple(context, context->roots.environmentType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_environment_t));
    result->parent = parent;
    result->symbolTable = tuuvm_identityDictionary_create(context);
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_environment_getIntrinsicsBuiltInEnvironment(tuuvm_context_t *context)
{
    return context->roots.intrinsicsBuiltInEnvironment;
}

TUUVM_API tuuvm_tuple_t tuuvm_environment_createDefaultForEvaluation(tuuvm_context_t *context)
{
    return tuuvm_environment_create(context, tuuvm_environment_getIntrinsicsBuiltInEnvironment(context));
}

TUUVM_API void tuuvm_environment_setSymbolBinding(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t symbol, tuuvm_tuple_t binding)
{
    if(!tuuvm_tuple_isNonNullPointer(environment))
        return;

    tuuvm_environment_t *environmentObject = (tuuvm_environment_t*)environment;
    tuuvm_dictionary_atPut(context, environmentObject->symbolTable, symbol, binding);
}

TUUVM_API bool tuuvm_environment_lookSymbolRecursively(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t symbol, tuuvm_tuple_t *outBinding)
{
    *outBinding = TUUVM_NULL_TUPLE;
    if(!tuuvm_tuple_isNonNullPointer(environment))
        return false;

    tuuvm_environment_t *environmentObject = (tuuvm_environment_t*)environment;
    if(tuuvm_dictionary_find(context, environmentObject->symbolTable, symbol, outBinding))
        return true;

    return tuuvm_environment_lookSymbolRecursively(context, environmentObject->parent, symbol, outBinding);
}