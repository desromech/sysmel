#include "tuuvm/environment.h"
#include "tuuvm/association.h"
#include "tuuvm/arrayList.h"
#include "tuuvm/dictionary.h"
#include "tuuvm/errors.h"
#include "tuuvm/sourceCode.h"
#include "tuuvm/string.h"
#include "tuuvm/function.h"
#include "tuuvm/type.h"
#include "internal/context.h"

TUUVM_API bool tuuvm_symbolBinding_isValue(tuuvm_context_t *context, tuuvm_tuple_t binding)
{
    return tuuvm_tuple_isKindOf(context, binding, context->roots.symbolValueBindingType);
}

TUUVM_API tuuvm_tuple_t tuuvm_symbolArgumentBinding_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t name, tuuvm_tuple_t type)
{
    tuuvm_symbolArgumentBinding_t *result = (tuuvm_symbolArgumentBinding_t*)tuuvm_context_allocatePointerTuple(context, context->roots.symbolArgumentBindingType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_symbolArgumentBinding_t));
    result->super.name = name;
    result->super.sourcePosition = sourcePosition;
    result->super.type = type;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_symbolLocalBinding_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t name, tuuvm_tuple_t type)
{
    tuuvm_symbolLocalBinding_t *result = (tuuvm_symbolLocalBinding_t*)tuuvm_context_allocatePointerTuple(context, context->roots.symbolLocalBindingType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_symbolLocalBinding_t));
    result->super.name = name;
    result->super.sourcePosition = sourcePosition;
    result->super.type = type;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_symbolValueBinding_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t name, tuuvm_tuple_t value)
{
    tuuvm_symbolValueBinding_t *result = (tuuvm_symbolValueBinding_t*)tuuvm_context_allocatePointerTuple(context, context->roots.symbolValueBindingType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_symbolValueBinding_t));
    result->super.name = name;
    result->super.sourcePosition = sourcePosition;
    result->super.type = tuuvm_tuple_getType(context, value);
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

TUUVM_API tuuvm_tuple_t tuuvm_functionAnalysisEnvironment_create(tuuvm_context_t *context, tuuvm_tuple_t parent, tuuvm_tuple_t functionDefinition)
{
    tuuvm_functionAnalysisEnvironment_t *result = (tuuvm_functionAnalysisEnvironment_t*)tuuvm_context_allocatePointerTuple(context, context->roots.functionAnalysisEnvironmentType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_functionAnalysisEnvironment_t));
    result->super.parent = parent;
    result->super.symbolTable = tuuvm_identityDictionary_create(context);
    result->functionDefinition = functionDefinition;
    result->captureBindingList = tuuvm_arrayList_create(context);
    result->argumentBindingList = tuuvm_arrayList_create(context);
    result->localBindingList = tuuvm_arrayList_create(context);
    result->hasBreakTarget = TUUVM_FALSE_TUPLE;
    result->hasContinueTarget = TUUVM_FALSE_TUPLE;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_localAnalysisEnvironment_create(tuuvm_context_t *context, tuuvm_tuple_t parent)
{
    tuuvm_localAnalysisEnvironment_t *result = (tuuvm_localAnalysisEnvironment_t*)tuuvm_context_allocatePointerTuple(context, context->roots.localAnalysisEnvironmentType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_localAnalysisEnvironment_t));
    result->super.parent = parent;
    result->super.symbolTable = tuuvm_identityDictionary_create(context);
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

TUUVM_API tuuvm_tuple_t tuuvm_environment_createDefaultForSourceCodeEvaluation(tuuvm_context_t *context, tuuvm_tuple_t sourceCode)
{
    tuuvm_tuple_t environment = tuuvm_environment_createDefaultForEvaluation(context);
    tuuvm_environment_setNewSymbolBindingWithValue(context, environment, tuuvm_symbol_internWithCString(context, "__SourceDirectory__"), tuuvm_sourceCode_getDirectory(sourceCode));
    tuuvm_environment_setNewSymbolBindingWithValue(context, environment, tuuvm_symbol_internWithCString(context, "__SourceName__"), tuuvm_sourceCode_getName(sourceCode));
    tuuvm_environment_setNewSymbolBindingWithValue(context, environment, tuuvm_symbol_internWithCString(context, "__SourceLanguage__"), tuuvm_sourceCode_getLanguage(sourceCode));
    return environment;
}

TUUVM_API void tuuvm_environment_setBinding(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t binding)
{
    if(!tuuvm_tuple_isNonNullPointer(environment))
        return;

    tuuvm_environment_t *environmentObject = (tuuvm_environment_t*)environment;
    tuuvm_identityDictionary_addAssociation(context, environmentObject->symbolTable, binding);
}

TUUVM_API void tuuvm_environment_setNewBinding(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t binding)
{
    if(!tuuvm_tuple_isNonNullPointer(environment))
        return;

    tuuvm_environment_t *environmentObject = (tuuvm_environment_t*)environment;
    tuuvm_tuple_t existentBinding = TUUVM_NULL_TUPLE;
    tuuvm_tuple_t symbol = tuuvm_association_getKey(binding);

    if(tuuvm_identityDictionary_findAssociation(environmentObject->symbolTable, symbol, &existentBinding))
        tuuvm_error("Overriding existent symbol binding.");

    tuuvm_identityDictionary_addAssociation(context, environmentObject->symbolTable, binding);
}

TUUVM_API void tuuvm_environment_setSymbolBindingWithValue(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t symbol, tuuvm_tuple_t value)
{
    if(!tuuvm_tuple_isNonNullPointer(environment))
        return;

    tuuvm_environment_t *environmentObject = (tuuvm_environment_t*)environment;
    tuuvm_tuple_t binding = tuuvm_symbolValueBinding_create(context, TUUVM_NULL_TUPLE, symbol, value);
    tuuvm_identityDictionary_addAssociation(context, environmentObject->symbolTable, binding);
}

TUUVM_API void tuuvm_environment_setNewSymbolBindingWithValueAtSourcePosition(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t symbol, tuuvm_tuple_t value, tuuvm_tuple_t sourcePosition)
{
    if(!tuuvm_tuple_isNonNullPointer(environment))
        return;


    tuuvm_tuple_t binding = tuuvm_symbolValueBinding_create(context, sourcePosition, symbol, value);
    tuuvm_environment_setNewBinding(context, environment, binding);
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
    if(tuuvm_identityDictionary_findAssociation(environmentObject->symbolTable, symbol, outBinding))
        return true;

    return tuuvm_environment_lookSymbolRecursively(context, environmentObject->parent, symbol, outBinding);
}

TUUVM_API tuuvm_tuple_t tuuvm_environment_lookReturnTargetRecursively(tuuvm_context_t *context, tuuvm_tuple_t environment)
{
    if(!tuuvm_tuple_isNonNullPointer(environment))
        return TUUVM_NULL_TUPLE;

    tuuvm_environment_t *environmentObject = (tuuvm_environment_t*)environment;
    if(environmentObject->returnTarget)
        return environmentObject->returnTarget;

    return tuuvm_environment_lookReturnTargetRecursively(context, environmentObject->parent);
}

TUUVM_API tuuvm_tuple_t tuuvm_environment_lookBreakTargetRecursively(tuuvm_context_t *context, tuuvm_tuple_t environment)
{
    if(!tuuvm_tuple_isNonNullPointer(environment))
        return TUUVM_NULL_TUPLE;

    tuuvm_environment_t *environmentObject = (tuuvm_environment_t*)environment;
    if(environmentObject->breakTarget)
        return environmentObject->breakTarget;

    return tuuvm_environment_lookReturnTargetRecursively(context, environmentObject->parent);
}

TUUVM_API tuuvm_tuple_t tuuvm_environment_lookContinueTargetRecursively(tuuvm_context_t *context, tuuvm_tuple_t environment)
{
    if(!tuuvm_tuple_isNonNullPointer(environment))
        return TUUVM_NULL_TUPLE;

    tuuvm_environment_t *environmentObject = (tuuvm_environment_t*)environment;
    if(environmentObject->continueTarget)
        return environmentObject->continueTarget;

    return tuuvm_environment_lookReturnTargetRecursively(context, environmentObject->parent);
}

TUUVM_API void tuuvm_environment_setBreakTarget(tuuvm_tuple_t environment, tuuvm_tuple_t breakTarget)
{
    if(!tuuvm_tuple_isNonNullPointer(environment))
        return;

    tuuvm_environment_t *environmentObject = (tuuvm_environment_t*)environment;
    environmentObject->breakTarget = breakTarget;
}

TUUVM_API void tuuvm_environment_setContinueTarget(tuuvm_tuple_t environment, tuuvm_tuple_t continueTarget)
{
    if(!tuuvm_tuple_isNonNullPointer(environment))
        return;

    tuuvm_environment_t *environmentObject = (tuuvm_environment_t*)environment;
    environmentObject->continueTarget = continueTarget;
}

TUUVM_API void tuuvm_environment_setReturnTarget(tuuvm_tuple_t environment, tuuvm_tuple_t returnTarget)
{
    if(!tuuvm_tuple_isNonNullPointer(environment))
        return;

    tuuvm_environment_t *environmentObject = (tuuvm_environment_t*)environment;
    environmentObject->returnTarget = returnTarget;
}

TUUVM_API void tuuvm_environment_clearUnwindingRecords(tuuvm_tuple_t environment)
{
    if(!tuuvm_tuple_isNonNullPointer(environment))
        return;

    tuuvm_environment_t *environmentObject = (tuuvm_environment_t*)environment;
    environmentObject->breakTarget = TUUVM_NULL_TUPLE;
    environmentObject->continueTarget = TUUVM_NULL_TUPLE;
    environmentObject->returnTarget = TUUVM_NULL_TUPLE;
}

TUUVM_API bool tuuvm_environment_isAnalysisEnvironment(tuuvm_context_t *context, tuuvm_tuple_t environment)
{
    return tuuvm_tuple_isKindOf(context, environment, context->roots.analysisEnvironmentType);
}

TUUVM_API bool tuuvm_environment_isFunctionAnalysisEnvironment(tuuvm_context_t *context, tuuvm_tuple_t environment)
{
    return tuuvm_tuple_isKindOf(context, environment, context->roots.functionAnalysisEnvironmentType);
}

TUUVM_API tuuvm_tuple_t tuuvm_environment_lookFunctionAnalysisEnvironmentRecursively(tuuvm_context_t *context, tuuvm_tuple_t environment)
{
    if(!environment) return TUUVM_NULL_TUPLE;
    if(tuuvm_environment_isFunctionAnalysisEnvironment(context, environment)) return environment;
    return tuuvm_environment_lookFunctionAnalysisEnvironmentRecursively(context, ((tuuvm_environment_t*)environment)->parent);
}

TUUVM_API tuuvm_tuple_t tuuvm_analysisEnvironment_setNewSymbolArgumentBinding(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t name, tuuvm_tuple_t type)
{
    tuuvm_tuple_t functionAnalysisEnvironment = tuuvm_environment_lookFunctionAnalysisEnvironmentRecursively(context, environment);
    if(!functionAnalysisEnvironment)
        tuuvm_error("A function analysis environment is required here.");

    tuuvm_tuple_t binding = tuuvm_symbolArgumentBinding_create(context, sourcePosition, name, type);
    tuuvm_environment_setNewBinding(context, environment, binding);
    tuuvm_arrayList_add(context, ((tuuvm_functionAnalysisEnvironment_t*)functionAnalysisEnvironment)->argumentBindingList, binding);
    return binding;
}

TUUVM_API tuuvm_tuple_t tuuvm_analysisEnvironment_setNewSymbolLocalBinding(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t name, tuuvm_tuple_t type)
{
    tuuvm_tuple_t functionAnalysisEnvironment = tuuvm_environment_lookFunctionAnalysisEnvironmentRecursively(context, environment);
    if(!functionAnalysisEnvironment)
        tuuvm_error("A function analysis environment is required here.");

    tuuvm_tuple_t binding = tuuvm_symbolLocalBinding_create(context, sourcePosition, name, type);
    tuuvm_environment_setNewBinding(context, environment, binding);
    tuuvm_arrayList_add(context, ((tuuvm_functionAnalysisEnvironment_t*)functionAnalysisEnvironment)->localBindingList, binding);
    return binding;
}

TUUVM_API tuuvm_tuple_t tuuvm_analysisEnvironment_setNewValueBinding(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t name, tuuvm_tuple_t value)
{
    tuuvm_tuple_t binding = tuuvm_symbolValueBinding_create(context, sourcePosition, name, value);
    tuuvm_environment_setNewBinding(context, environment, binding);
    return binding;
}

static tuuvm_tuple_t tuuvm_environment_primitive_setNewBinding(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_environment_setNewBinding(context, arguments[0], arguments[1]);
    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_environment_primitive_setBinding(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_environment_setBinding(context, arguments[0], arguments[1]);
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

void tuuvm_environment_registerPrimitives(void)
{
    tuuvm_primitiveTable_registerFunction(tuuvm_environment_primitive_setNewBinding);
    tuuvm_primitiveTable_registerFunction(tuuvm_environment_primitive_setBinding);
    tuuvm_primitiveTable_registerFunction(tuuvm_environment_primitive_setNewSymbolBindingWithValue);
    tuuvm_primitiveTable_registerFunction(tuuvm_environment_primitive_setSymbolBindingWithValue);
}

void tuuvm_environment_setupPrimitives(tuuvm_context_t *context)
{
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Environment::setNewBinding:", context->roots.environmentType, "setBinding:", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_environment_primitive_setNewBinding);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Environment::setBinding:", context->roots.environmentType, "setBinding:", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_environment_primitive_setBinding);
    
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Environment::setNewSymbol:bindingWithValue:", context->roots.environmentType, "setNewSymbol:bindingWithValue:", 3, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_environment_primitive_setNewSymbolBindingWithValue);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Environment::setSymbol:bindingWithValue:", context->roots.environmentType, "setSymbol:bindingWithValue:", 3, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_environment_primitive_setSymbolBindingWithValue);
}