#include "tuuvm/environment.h"
#include "tuuvm/dictionary.h"
#include "tuuvm/errors.h"
#include "tuuvm/function.h"
#include "internal/context.h"

TUUVM_API bool tuuvm_symbolBinding_isValue(tuuvm_context_t *context, tuuvm_tuple_t binding)
{
    return tuuvm_tuple_isKindOf(context, binding, context->roots.symbolValueBindingType);
}

TUUVM_API tuuvm_tuple_t tuuvm_symbolArgumentBinding_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t name)
{
    tuuvm_symbolArgumentBinding_t *result = (tuuvm_symbolArgumentBinding_t*)tuuvm_context_allocatePointerTuple(context, context->roots.symbolArgumentBindingType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_symbolArgumentBinding_t));
    result->super.sourcePosition = sourcePosition;
    result->super.name = name;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_symbolLocalBinding_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t name)
{
    tuuvm_symbolLocalBinding_t *result = (tuuvm_symbolLocalBinding_t*)tuuvm_context_allocatePointerTuple(context, context->roots.symbolLocalBindingType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_symbolLocalBinding_t));
    result->super.sourcePosition = sourcePosition;
    result->super.name = name;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_symbolValueBinding_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t name, tuuvm_tuple_t value)
{
    tuuvm_symbolValueBinding_t *result = (tuuvm_symbolValueBinding_t*)tuuvm_context_allocatePointerTuple(context, context->roots.symbolValueBindingType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_symbolValueBinding_t));
    result->super.sourcePosition = sourcePosition;
    result->super.name = name;
    result->value = value;
    return (tuuvm_tuple_t)result;
}

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

TUUVM_API void tuuvm_environment_setSymbolBindingWithValue(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t symbol, tuuvm_tuple_t value)
{
    if(!tuuvm_tuple_isNonNullPointer(environment))
        return;

    tuuvm_environment_t *environmentObject = (tuuvm_environment_t*)environment;
    tuuvm_tuple_t binding = tuuvm_symbolValueBinding_create(context, TUUVM_NULL_TUPLE, symbol, value);
    tuuvm_dictionary_atPut(context, environmentObject->symbolTable, symbol, binding);
}

TUUVM_API void tuuvm_environment_setNewSymbolBindingWithValueAtSourcePosition(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t symbol, tuuvm_tuple_t value, tuuvm_tuple_t sourcePosition)
{
    if(!tuuvm_tuple_isNonNullPointer(environment))
        return;

    tuuvm_environment_t *environmentObject = (tuuvm_environment_t*)environment;
    tuuvm_tuple_t existentBinding = TUUVM_NULL_TUPLE;

    if(tuuvm_dictionary_find(context, environmentObject->symbolTable, symbol, &existentBinding))
        tuuvm_error("Overriding existent symbol binding.");

    tuuvm_tuple_t binding = tuuvm_symbolValueBinding_create(context, sourcePosition, symbol, value);
    tuuvm_dictionary_atPut(context, environmentObject->symbolTable, symbol, binding);
}

TUUVM_API void tuuvm_environment_setNewSymbolBindingWithValue(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t symbol, tuuvm_tuple_t value)
{
   tuuvm_environment_setNewSymbolBindingWithValueAtSourcePosition(context, environment, symbol, value, TUUVM_NULL_TUPLE);
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

static tuuvm_tuple_t tuuvm_environment_primitive_setNewSymbolBindingWithValue(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) tuuvm_error_argumentCountMismatch(3, argumentCount);

    tuuvm_environment_setNewSymbolBindingWithValue(context, arguments[0], arguments[1], arguments[2]);
    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_environment_primitive_setSymbolBindingWithValue(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) tuuvm_error_argumentCountMismatch(3, argumentCount);

    tuuvm_environment_setSymbolBindingWithValue(context, arguments[0], arguments[1], arguments[2]);
    return TUUVM_VOID_TUPLE;
}

void tuuvm_environment_setupPrimitives(tuuvm_context_t *context)
{
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveMethod(context, "Environment::setNewSymbol:binding:", context->roots.environmentType, "setNewSymbol:binding:", 3, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_environment_primitive_setNewSymbolBinding);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveMethod(context, "Environment::setSymbol:binding:", context->roots.environmentType, "setSymbol:binding:", 3, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_environment_primitive_setSymbolBinding);
    
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveMethod(context, "Environment::setNewSymbol:bindingWithValue:", context->roots.environmentType, "setNewSymbol:bindingWithValue:", 3, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_environment_primitive_setNewSymbolBindingWithValue);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveMethod(context, "Environment::setSymbol:bindingWithValue:", context->roots.environmentType, "setSymbol:bindingWithValue:", 3, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_environment_primitive_setSymbolBindingWithValue);
}