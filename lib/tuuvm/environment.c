#include "tuuvm/environment.h"
#include "tuuvm/dictionary.h"
#include "tuuvm/errors.h"
#include "tuuvm/function.h"
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

TUUVM_API void tuuvm_environment_setNewSymbolBinding(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t symbol, tuuvm_tuple_t binding)
{
    if(!tuuvm_tuple_isNonNullPointer(environment))
        return;

    tuuvm_environment_t *environmentObject = (tuuvm_environment_t*)environment;
    tuuvm_tuple_t existentBinding = TUUVM_NULL_TUPLE;

    if(tuuvm_dictionary_find(context, environmentObject->symbolTable, symbol, &existentBinding))
        tuuvm_error("Overriding existent symbol binding.");

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

static tuuvm_tuple_t tuuvm_environment_primitive_setNewSymbolBinding(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) tuuvm_error_argumentCountMismatch(3, argumentCount);

    tuuvm_environment_setNewSymbolBinding(context, arguments[0], arguments[1], arguments[2]);
    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_environment_primitive_setSymbolBinding(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) tuuvm_error_argumentCountMismatch(3, argumentCount);

    tuuvm_environment_setSymbolBinding(context, arguments[0], arguments[1], arguments[2]);
    return TUUVM_VOID_TUPLE;
}

void tuuvm_environment_setupPrimitives(tuuvm_context_t *context)
{
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "Environment::setNewSymbol:binding:", 3, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_environment_primitive_setNewSymbolBinding);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "Environment::setSymbol:binding:", 3, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_environment_primitive_setSymbolBinding);
}