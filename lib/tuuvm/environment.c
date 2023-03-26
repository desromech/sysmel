#include "tuuvm/environment.h"
#include "tuuvm/array.h"
#include "tuuvm/arrayList.h"
#include "tuuvm/assert.h"
#include "tuuvm/association.h"
#include "tuuvm/dictionary.h"
#include "tuuvm/errors.h"
#include "tuuvm/pragma.h"
#include "tuuvm/sourceCode.h"
#include "tuuvm/string.h"
#include "tuuvm/function.h"
#include "tuuvm/type.h"
#include "internal/context.h"
#include <stdlib.h>

static inline bool tuuvm_symbolBinding_isValueQuick(tuuvm_context_t *context, tuuvm_tuple_t binding)
{
    if(!tuuvm_tuple_isNonNullPointer(binding)) return false;

    tuuvm_tuple_t type = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(binding)->header.typePointerAndFlags & TUUVM_TUPLE_TYPE_POINTER_MASK;
    return type == context->roots.symbolValueBindingType;
}

TUUVM_API tuuvm_tuple_t tuuvm_analyzerToken_create(tuuvm_context_t *context)
{
    return (tuuvm_tuple_t)tuuvm_context_allocatePointerTuple(context, context->roots.objectType, TUUVM_NULL_TUPLE);
}

TUUVM_API bool tuuvm_symbolBinding_isValue(tuuvm_context_t *context, tuuvm_tuple_t binding)
{
    return tuuvm_tuple_isKindOf(context, binding, context->roots.symbolValueBindingType);
}

TUUVM_API bool tuuvm_symbolBinding_isMacroValue(tuuvm_context_t *context, tuuvm_tuple_t binding)
{
    return tuuvm_tuple_isKindOf(context, binding, context->roots.symbolMacroValueBindingType);
}

TUUVM_API bool tuuvm_symbolBinding_isAnalysisBinding(tuuvm_context_t *context, tuuvm_tuple_t binding)
{
    return tuuvm_tuple_isKindOf(context, binding, context->roots.symbolAnalysisBindingType);
}

TUUVM_API tuuvm_tuple_t tuuvm_symbolArgumentBinding_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t name, tuuvm_tuple_t type, tuuvm_tuple_t ownerFunction, size_t vectorIndex)
{
    tuuvm_symbolArgumentBinding_t *result = (tuuvm_symbolArgumentBinding_t*)tuuvm_context_allocatePointerTuple(context, context->roots.symbolArgumentBindingType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_symbolArgumentBinding_t));
    result->super.super.name = name;
    result->super.super.sourcePosition = sourcePosition;
    result->super.super.type = type;
    result->super.ownerFunction = ownerFunction;
    result->super.vectorIndex = tuuvm_tuple_size_encode(context, vectorIndex);
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_symbolLocalBinding_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t name, tuuvm_tuple_t type, tuuvm_tuple_t ownerFunction, size_t vectorIndex)
{
    tuuvm_symbolLocalBinding_t *result = (tuuvm_symbolLocalBinding_t*)tuuvm_context_allocatePointerTuple(context, context->roots.symbolLocalBindingType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_symbolLocalBinding_t));
    result->super.super.name = name;
    result->super.super.sourcePosition = sourcePosition;
    result->super.super.type = type;
    result->super.ownerFunction = ownerFunction;
    result->super.vectorIndex = tuuvm_tuple_size_encode(context, vectorIndex);
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_symbolCaptureBinding_create(tuuvm_context_t *context, tuuvm_tuple_t capturedBinding, tuuvm_tuple_t ownerFunction, size_t vectorIndex)
{
    if(!tuuvm_tuple_isKindOf(context, capturedBinding, context->roots.symbolBindingType))
        tuuvm_error("Expected a symbol binding");

    tuuvm_symbolBinding_t *capturedBindingObject = (tuuvm_symbolBinding_t*)capturedBinding;

    tuuvm_symbolCaptureBinding_t *result = (tuuvm_symbolCaptureBinding_t*)tuuvm_context_allocatePointerTuple(context, context->roots.symbolCaptureBindingType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_symbolCaptureBinding_t));
    result->super.super.sourcePosition = capturedBindingObject->sourcePosition;
    result->super.super.name = capturedBindingObject->name;
    result->super.super.type = capturedBindingObject->type;
    result->super.ownerFunction = ownerFunction;
    result->super.vectorIndex = tuuvm_tuple_size_encode(context, vectorIndex);
    result->capturedBinding = capturedBinding;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_symbolMacroValueBinding_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t name, tuuvm_tuple_t expansion)
{
    tuuvm_symbolMacroValueBinding_t *result = (tuuvm_symbolMacroValueBinding_t*)tuuvm_context_allocatePointerTuple(context, context->roots.symbolMacroValueBindingType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_symbolMacroValueBinding_t));
    result->super.name = name;
    result->super.sourcePosition = sourcePosition;
    result->super.type = tuuvm_tuple_getType(context, expansion);
    result->expansion = expansion;
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
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_analysisAndEvaluationEnvironment_create(tuuvm_context_t *context, tuuvm_tuple_t parent)
{
    tuuvm_analysisAndEvaluationEnvironment_t *result = (tuuvm_analysisAndEvaluationEnvironment_t*)tuuvm_context_allocatePointerTuple(context, context->roots.analysisAndEvaluationEnvironmentType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_analysisAndEvaluationEnvironment_t));
    result->super.parent = parent;

    // Inherit the analyzer token, if possible.
    if(parent && tuuvm_environment_isAnalysisAndEvaluationEnvironment(context, parent))
        result->analyzerToken = ((tuuvm_analysisAndEvaluationEnvironment_t*)parent)->analyzerToken;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_namespace_create(tuuvm_context_t *context, tuuvm_tuple_t parent)
{
    tuuvm_namespace_t *result = (tuuvm_namespace_t*)tuuvm_context_allocatePointerTuple(context, context->roots.namespaceType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_namespace_t));
    result->parent = parent;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_functionActivationEnvironment_create(tuuvm_context_t *context, tuuvm_tuple_t parent, tuuvm_tuple_t function)
{
    if(!function || !tuuvm_tuple_isFunction(context, function))
        tuuvm_error("Expected a function");

    tuuvm_function_t *functionObject = (tuuvm_function_t*)function;
    if(!functionObject->definition)
        tuuvm_error("Expected a function with a definition.");

    tuuvm_functionDefinition_t *functionDefinitionObject = (tuuvm_functionDefinition_t*)functionObject->definition;
    if(!functionDefinitionObject->analysisEnvironment)
        tuuvm_error("Expected a function with an analyzed definition.");

    tuuvm_functionActivationEnvironment_t *result = (tuuvm_functionActivationEnvironment_t*)tuuvm_context_allocatePointerTuple(context, context->roots.functionActivationEnvironmentType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_functionActivationEnvironment_t));
    result->super.super.parent = parent;
    result->function = function;
    result->functionDefinition = functionObject->definition;
    result->captureVector = functionObject->captureVector;
    
    size_t argumentCount = tuuvm_array_getSize(functionDefinitionObject->analyzedArguments);
    size_t localCount = tuuvm_array_getSize(functionDefinitionObject->analyzedLocals);
    result->argumentVectorSize = tuuvm_tuple_size_encode(context, argumentCount);
    result->valueVector = tuuvm_array_create(context, argumentCount + localCount);
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_functionActivationEnvironment_createForDependentFunctionType(tuuvm_context_t *context, tuuvm_tuple_t parent, tuuvm_tuple_t dependentFunctionType)
{
    if(!dependentFunctionType || !tuuvm_tuple_isKindOf(context, dependentFunctionType, context->roots.dependentFunctionTypeType))
        tuuvm_error("Expected a dependent function type");

    tuuvm_dependentFunctionType_t *dependentFunctionTypeObject = (tuuvm_dependentFunctionType_t*)dependentFunctionType;
 
    tuuvm_functionActivationEnvironment_t *result = (tuuvm_functionActivationEnvironment_t*)tuuvm_context_allocatePointerTuple(context, context->roots.functionActivationEnvironmentType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_functionActivationEnvironment_t));
    result->super.super.parent = parent;
    result->dependentFunctionType = dependentFunctionType;
    result->captureVector = tuuvm_array_create(context, 0);
    
    size_t argumentCount = tuuvm_array_getSize(dependentFunctionTypeObject->argumentBindings);
    size_t localCount = tuuvm_array_getSize(dependentFunctionTypeObject->localBindings);
    result->argumentVectorSize = tuuvm_tuple_size_encode(context, argumentCount);
    result->valueVector = tuuvm_array_create(context, argumentCount + localCount);
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_functionAnalysisEnvironment_create(tuuvm_context_t *context, tuuvm_tuple_t parent, tuuvm_tuple_t functionDefinition)
{
    tuuvm_functionAnalysisEnvironment_t *result = (tuuvm_functionAnalysisEnvironment_t*)tuuvm_context_allocatePointerTuple(context, context->roots.functionAnalysisEnvironmentType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_functionAnalysisEnvironment_t));
    result->super.super.parent = parent;
    result->functionDefinition = functionDefinition;
    result->captureBindingTable = tuuvm_identityDictionary_create(context);
    result->captureBindingList = tuuvm_arrayList_create(context);
    result->argumentBindingList = tuuvm_arrayList_create(context);
    result->localBindingList = tuuvm_arrayList_create(context);
    result->innerFunctionList = tuuvm_arrayList_create(context);
    result->pragmaList = tuuvm_arrayList_create(context);
    result->hasBreakTarget = TUUVM_FALSE_TUPLE;
    result->hasContinueTarget = TUUVM_FALSE_TUPLE;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_localAnalysisEnvironment_create(tuuvm_context_t *context, tuuvm_tuple_t parent)
{
    tuuvm_localAnalysisEnvironment_t *result = (tuuvm_localAnalysisEnvironment_t*)tuuvm_context_allocatePointerTuple(context, context->roots.localAnalysisEnvironmentType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_localAnalysisEnvironment_t));
    result->super.super.parent = parent;
    result->super.super.symbolTable = tuuvm_identityDictionary_create(context);
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_environment_getIntrinsicsBuiltInEnvironment(tuuvm_context_t *context)
{
    return context->roots.globalNamespace;
}

TUUVM_API tuuvm_tuple_t tuuvm_environment_createDefaultForEvaluation(tuuvm_context_t *context)
{
    return tuuvm_analysisAndEvaluationEnvironment_create(context, tuuvm_environment_getIntrinsicsBuiltInEnvironment(context));
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
    if(!environmentObject->symbolTable)
        environmentObject->symbolTable = tuuvm_identityDictionary_create(context);
    tuuvm_identityDictionary_addAssociation(context, environmentObject->symbolTable, binding);
}

TUUVM_API void tuuvm_environment_setNewBinding(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t binding)
{
    if(!tuuvm_tuple_isNonNullPointer(environment))
        return;

    tuuvm_environment_t *environmentObject = (tuuvm_environment_t*)environment;
    tuuvm_tuple_t existentBinding = TUUVM_NULL_TUPLE;
    tuuvm_tuple_t symbol = tuuvm_association_getKey(binding);

    if(environmentObject->symbolTable && tuuvm_identityDictionary_findAssociation(environmentObject->symbolTable, symbol, &existentBinding))
        tuuvm_error("Overriding existent symbol binding.");

    if(!environmentObject->symbolTable)
        environmentObject->symbolTable = tuuvm_identityDictionary_create(context);

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
    if(environmentObject->symbolTable && tuuvm_identityDictionary_findAssociation(environmentObject->symbolTable, symbol, outBinding))
        return true;

    return tuuvm_environment_lookSymbolRecursively(context, environmentObject->parent, symbol, outBinding);
}

TUUVM_API tuuvm_tuple_t tuuvm_environment_evaluateSymbolBinding(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t binding)
{
    if(tuuvm_tuple_getType(context, environment) == context->roots.functionActivationEnvironmentType)
    {
        tuuvm_functionActivationEnvironment_t *activationEnvironment = (tuuvm_functionActivationEnvironment_t*)environment;
        tuuvm_symbolAnalysisBinding_t *analysisBinding = (tuuvm_symbolAnalysisBinding_t*)binding;
        if(analysisBinding->ownerFunction == activationEnvironment->functionDefinition || activationEnvironment->dependentFunctionType)
        {
            size_t vectorIndex = tuuvm_tuple_size_decode(analysisBinding->vectorIndex);
            tuuvm_tuple_t bindingType = tuuvm_tuple_getType(context, binding);
            if(bindingType == context->roots.symbolArgumentBindingType)
                return tuuvm_array_at(activationEnvironment->valueVector, vectorIndex);
            else if(bindingType == context->roots.symbolLocalBindingType)
                return tuuvm_array_at(activationEnvironment->valueVector, tuuvm_tuple_size_decode(activationEnvironment->argumentVectorSize) + vectorIndex);
            else if(bindingType == context->roots.symbolCaptureBindingType)
                return tuuvm_array_at(activationEnvironment->captureVector, vectorIndex);
            abort();
        }
    }

    tuuvm_tuple_t foundBinding = TUUVM_NULL_TUPLE;
    if(!tuuvm_environment_lookSymbolRecursively(context, environment, tuuvm_symbolBinding_getName(binding), &foundBinding))
        tuuvm_error("Failed to evaluate analyzed symbol binding.");

    if(!tuuvm_symbolBinding_isValueQuick(context, foundBinding))
        tuuvm_error("A value binding in the evaluation context is required.");

    return tuuvm_symbolValueBinding_getValue(foundBinding);
}

TUUVM_API void tuuvm_functionActivationEnvironment_setBindingActivationValue(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t binding, tuuvm_tuple_t value, tuuvm_tuple_t sourcePosition)
{
    (void)sourcePosition;
    tuuvm_functionActivationEnvironment_t *activationEnvironment = (tuuvm_functionActivationEnvironment_t*)environment;
    tuuvm_symbolAnalysisBinding_t *analysisBinding = (tuuvm_symbolAnalysisBinding_t*)binding;
    if(analysisBinding->ownerFunction == activationEnvironment->functionDefinition || activationEnvironment->dependentFunctionType)
    {
        size_t vectorIndex = tuuvm_tuple_size_decode(analysisBinding->vectorIndex);
        tuuvm_tuple_t bindingType = tuuvm_tuple_getType(context, binding);
        if(bindingType == context->roots.symbolLocalBindingType)
        {
            tuuvm_array_atPut(activationEnvironment->valueVector, tuuvm_tuple_size_decode(activationEnvironment->argumentVectorSize) + vectorIndex, value);
            return;
        }
        else if(bindingType == context->roots.symbolArgumentBindingType)
        {
            tuuvm_array_atPut(activationEnvironment->valueVector, vectorIndex, value);
            return;
        }
    }
    abort();
}

TUUVM_API tuuvm_tuple_t tuuvm_environment_lookReturnTargetRecursively(tuuvm_context_t *context, tuuvm_tuple_t environment)
{
    if(!tuuvm_environment_isAnalysisAndEvaluationEnvironment(context, environment))
        return TUUVM_NULL_TUPLE;

    tuuvm_analysisAndEvaluationEnvironment_t *environmentObject = (tuuvm_analysisAndEvaluationEnvironment_t*)environment;
    if(environmentObject->returnTarget)
        return environmentObject->returnTarget;

    return tuuvm_environment_lookReturnTargetRecursively(context, environmentObject->super.parent);
}

TUUVM_API tuuvm_tuple_t tuuvm_environment_lookBreakTargetRecursively(tuuvm_context_t *context, tuuvm_tuple_t environment)
{
    if(!tuuvm_environment_isAnalysisAndEvaluationEnvironment(context, environment))
        return TUUVM_NULL_TUPLE;

    tuuvm_analysisAndEvaluationEnvironment_t *environmentObject = (tuuvm_analysisAndEvaluationEnvironment_t*)environment;
    if(environmentObject->breakTarget)
        return environmentObject->breakTarget;

    return tuuvm_environment_lookReturnTargetRecursively(context, environmentObject->super.parent);
}

TUUVM_API tuuvm_tuple_t tuuvm_environment_lookContinueTargetRecursively(tuuvm_context_t *context, tuuvm_tuple_t environment)
{
    if(!tuuvm_environment_isAnalysisAndEvaluationEnvironment(context, environment))
        return TUUVM_NULL_TUPLE;

    tuuvm_analysisAndEvaluationEnvironment_t *environmentObject = (tuuvm_analysisAndEvaluationEnvironment_t*)environment;
    if(environmentObject->continueTarget)
        return environmentObject->continueTarget;

    return tuuvm_environment_lookReturnTargetRecursively(context, environmentObject->super.parent);
}

TUUVM_API void tuuvm_analysisAndEvaluationEnvironment_setBreakTarget(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t breakTarget)
{
    if(!tuuvm_environment_isAnalysisAndEvaluationEnvironment(context, environment))
        return;

    tuuvm_analysisAndEvaluationEnvironment_t *environmentObject = (tuuvm_analysisAndEvaluationEnvironment_t*)environment;
    environmentObject->breakTarget = breakTarget;
}

TUUVM_API void tuuvm_analysisAndEvaluationEnvironment_setContinueTarget(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t continueTarget)
{
    if(!tuuvm_environment_isAnalysisAndEvaluationEnvironment(context, environment))
        return;

    tuuvm_analysisAndEvaluationEnvironment_t *environmentObject = (tuuvm_analysisAndEvaluationEnvironment_t*)environment;
    environmentObject->continueTarget = continueTarget;
}

TUUVM_API void tuuvm_analysisAndEvaluationEnvironment_setReturnTarget(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t returnTarget)
{
    if(!tuuvm_environment_isAnalysisAndEvaluationEnvironment(context, environment))
        return;

    tuuvm_analysisAndEvaluationEnvironment_t *environmentObject = (tuuvm_analysisAndEvaluationEnvironment_t*)environment;
    environmentObject->returnTarget = returnTarget;
}

TUUVM_API tuuvm_tuple_t tuuvm_analysisAndEvaluationEnvironment_getExpectedType(tuuvm_context_t *context, tuuvm_tuple_t environment)
{
    if(!tuuvm_environment_isAnalysisAndEvaluationEnvironment(context, environment))
        return TUUVM_NULL_TUPLE;

    tuuvm_analysisAndEvaluationEnvironment_t *environmentObject = (tuuvm_analysisAndEvaluationEnvironment_t*)environment;
    return environmentObject->expectedType;
}

TUUVM_API void tuuvm_analysisAndEvaluationEnvironment_setExpectedType(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t newExpectedType)
{
    if(!tuuvm_environment_isAnalysisAndEvaluationEnvironment(context, environment))
        return;

    tuuvm_analysisAndEvaluationEnvironment_t *environmentObject = (tuuvm_analysisAndEvaluationEnvironment_t*)environment;
    environmentObject->expectedType = newExpectedType;
}

TUUVM_API void tuuvm_analysisAndEvaluationEnvironment_clearUnwindingRecords(tuuvm_context_t *context, tuuvm_tuple_t environment)
{
    if(!tuuvm_environment_isAnalysisAndEvaluationEnvironment(context, environment))
        return;

    tuuvm_analysisAndEvaluationEnvironment_t *environmentObject = (tuuvm_analysisAndEvaluationEnvironment_t*)environment;
    environmentObject->breakTarget = TUUVM_NULL_TUPLE;
    environmentObject->continueTarget = TUUVM_NULL_TUPLE;
    environmentObject->returnTarget = TUUVM_NULL_TUPLE;
}

TUUVM_API void tuuvm_analysisAndEvaluationEnvironment_clearAnalyzerToken(tuuvm_context_t *context, tuuvm_tuple_t environment)
{
    if(!tuuvm_environment_isAnalysisAndEvaluationEnvironment(context, environment))
        return;

    tuuvm_analysisAndEvaluationEnvironment_t *environmentObject = (tuuvm_analysisAndEvaluationEnvironment_t*)environment;
    environmentObject->analyzerToken = TUUVM_NULL_TUPLE;
}

TUUVM_API tuuvm_tuple_t tuuvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(tuuvm_context_t *context, tuuvm_tuple_t environment)
{
    if(!tuuvm_environment_isAnalysisAndEvaluationEnvironment(context, environment))
        tuuvm_error("Expected an analysis and evaluation environment.");

    tuuvm_analysisAndEvaluationEnvironment_t *environmentObject = (tuuvm_analysisAndEvaluationEnvironment_t*)environment;
    if(!environmentObject->analyzerToken)
        environmentObject->analyzerToken = tuuvm_analyzerToken_create(context);
    return environmentObject->analyzerToken;
}

TUUVM_API bool tuuvm_environment_isAnalysisAndEvaluationEnvironment(tuuvm_context_t *context, tuuvm_tuple_t environment)
{
    return tuuvm_tuple_isKindOf(context, environment, context->roots.analysisAndEvaluationEnvironmentType);
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

TUUVM_API bool tuuvm_analysisEnvironment_lookSymbolRecursively(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t symbol, tuuvm_tuple_t *outBinding)
{
    *outBinding = TUUVM_NULL_TUPLE;
    if(!tuuvm_tuple_isNonNullPointer(environment))
        return false;

    tuuvm_environment_t *environmentObject = (tuuvm_environment_t*)environment;
    if(environmentObject->symbolTable && tuuvm_identityDictionary_findAssociation(environmentObject->symbolTable, symbol, outBinding))
        return true;

    // If the symbol is not found on the local table, we might need to capture it.
    if(tuuvm_environment_isFunctionAnalysisEnvironment(context, environment))
    {
        // Find a previously captured binding.
        tuuvm_functionAnalysisEnvironment_t *functionAnalysisEnvironment = (tuuvm_functionAnalysisEnvironment_t*)environment;
        if(tuuvm_identityDictionary_find(functionAnalysisEnvironment->captureBindingTable, symbol, outBinding))
            return true;

        tuuvm_tuple_t parentSymbol = TUUVM_NULL_TUPLE;
        if(!tuuvm_analysisEnvironment_lookSymbolRecursively(context, environmentObject->parent, symbol, &parentSymbol))
            return false;

        // Should we capture the symbol?
        if(tuuvm_symbolBinding_isAnalysisBinding(context, parentSymbol))
        {
            // Create the capture and store it.
            tuuvm_tuple_t captureBinding = tuuvm_symbolCaptureBinding_create(context, parentSymbol, functionAnalysisEnvironment->functionDefinition, tuuvm_arrayList_getSize(functionAnalysisEnvironment->captureBindingList));
            tuuvm_identityDictionary_atPut(context, functionAnalysisEnvironment->captureBindingTable, symbol, captureBinding);
            tuuvm_arrayList_add(context, functionAnalysisEnvironment->captureBindingList, captureBinding);
            *outBinding = captureBinding;
            return true;
        }

        *outBinding = parentSymbol;
        return true;
    }

    return tuuvm_analysisEnvironment_lookSymbolRecursively(context, environmentObject->parent, symbol, outBinding);
}

TUUVM_API tuuvm_tuple_t tuuvm_analysisEnvironment_setNewSymbolArgumentBinding(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t name, tuuvm_tuple_t type)
{
    tuuvm_tuple_t functionAnalysisEnvironment = tuuvm_environment_lookFunctionAnalysisEnvironmentRecursively(context, environment);
    if(!functionAnalysisEnvironment)
        tuuvm_error("A function analysis environment is required here.");

    tuuvm_functionAnalysisEnvironment_t *functionAnalysisEnvironmentObject = (tuuvm_functionAnalysisEnvironment_t*)functionAnalysisEnvironment;
    tuuvm_tuple_t binding = tuuvm_symbolArgumentBinding_create(context, sourcePosition, name, type, functionAnalysisEnvironmentObject->functionDefinition, tuuvm_arrayList_getSize(functionAnalysisEnvironmentObject->argumentBindingList));
    tuuvm_environment_setNewBinding(context, environment, binding);
    tuuvm_arrayList_add(context, functionAnalysisEnvironmentObject->argumentBindingList, binding);
    return binding;
}

TUUVM_API tuuvm_tuple_t tuuvm_analysisEnvironment_setNewSymbolLocalBinding(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t name, tuuvm_tuple_t type)
{
    tuuvm_tuple_t functionAnalysisEnvironment = tuuvm_environment_lookFunctionAnalysisEnvironmentRecursively(context, environment);
    if(!functionAnalysisEnvironment)
        tuuvm_error("A function analysis environment is required here.");

    tuuvm_functionAnalysisEnvironment_t *functionAnalysisEnvironmentObject = (tuuvm_functionAnalysisEnvironment_t*)functionAnalysisEnvironment;
    tuuvm_tuple_t binding = tuuvm_symbolLocalBinding_create(context, sourcePosition, name, type, functionAnalysisEnvironmentObject->functionDefinition, tuuvm_arrayList_getSize(functionAnalysisEnvironmentObject->localBindingList));
    tuuvm_environment_setNewBinding(context, environment, binding);
    tuuvm_arrayList_add(context, functionAnalysisEnvironmentObject->localBindingList, binding);
    return binding;
}

TUUVM_API void tuuvm_analysisEnvironment_addPragma(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t pragma)
{
    tuuvm_tuple_t functionAnalysisEnvironment = tuuvm_environment_lookFunctionAnalysisEnvironmentRecursively(context, environment);
    if(!functionAnalysisEnvironment)
        tuuvm_error("A function analysis environment is required here.");

    tuuvm_functionAnalysisEnvironment_t *functionAnalysisEnvironmentObject = (tuuvm_functionAnalysisEnvironment_t*)functionAnalysisEnvironment;
    tuuvm_pragma_t *pragmaObject = (tuuvm_pragma_t*)pragma;
    tuuvm_arrayList_add(context, functionAnalysisEnvironmentObject->pragmaList, pragma);

    if(pragmaObject->selector == context->roots.primitiveNamedSelector && !functionAnalysisEnvironmentObject->primitiveName)
        functionAnalysisEnvironmentObject->primitiveName = tuuvm_array_at(pragmaObject->arguments, 0);
}

TUUVM_API void tuuvm_analysisEnvironment_addInnerFunction(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t innerFunction)
{
    tuuvm_tuple_t functionAnalysisEnvironment = tuuvm_environment_lookFunctionAnalysisEnvironmentRecursively(context, environment);
    if(!functionAnalysisEnvironment)
        tuuvm_error("A function analysis environment is required here.");

    tuuvm_functionAnalysisEnvironment_t *functionAnalysisEnvironmentObject = (tuuvm_functionAnalysisEnvironment_t*)functionAnalysisEnvironment;
    tuuvm_arrayList_add(context, functionAnalysisEnvironmentObject->innerFunctionList, innerFunction);
}

TUUVM_API tuuvm_tuple_t tuuvm_environment_setNewMacroValueBinding(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t name, tuuvm_tuple_t expansion)
{
    tuuvm_tuple_t binding = tuuvm_symbolMacroValueBinding_create(context, sourcePosition, name, expansion);
    tuuvm_environment_setNewBinding(context, environment, binding);
    return binding;
}

TUUVM_API tuuvm_tuple_t tuuvm_analysisEnvironment_setNewValueBinding(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t name, tuuvm_tuple_t value)
{
    tuuvm_tuple_t binding = tuuvm_symbolValueBinding_create(context, sourcePosition, name, value);
    tuuvm_environment_setNewBinding(context, environment, binding);
    return binding;
}

static tuuvm_tuple_t tuuvm_environment_primitive_setNewBinding(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_environment_setNewBinding(context, arguments[0], arguments[1]);
    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_environment_primitive_setBinding(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_environment_setBinding(context, arguments[0], arguments[1]);
    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_environment_primitive_setNewSymbolBindingWithValue(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) tuuvm_error_argumentCountMismatch(3, argumentCount);

    tuuvm_environment_setNewSymbolBindingWithValue(context, arguments[0], arguments[1], arguments[2]);
    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_environment_primitive_setSymbolBindingWithValue(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) tuuvm_error_argumentCountMismatch(3, argumentCount);

    tuuvm_environment_setSymbolBindingWithValue(context, arguments[0], arguments[1], arguments[2]);
    return TUUVM_VOID_TUPLE;
}

void tuuvm_environment_registerPrimitives(void)
{
    tuuvm_primitiveTable_registerFunction(tuuvm_environment_primitive_setNewBinding, "Environment::setNewBinding:");
    tuuvm_primitiveTable_registerFunction(tuuvm_environment_primitive_setBinding, "Environment::setBinding:");
    tuuvm_primitiveTable_registerFunction(tuuvm_environment_primitive_setNewSymbolBindingWithValue, "Environment::setNewSymbol:bindingWithValue:");
    tuuvm_primitiveTable_registerFunction(tuuvm_environment_primitive_setSymbolBindingWithValue, "Environment::setSymbol:bindingWithValue:");
}

void tuuvm_environment_setupPrimitives(tuuvm_context_t *context)
{
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Environment::setNewBinding:", context->roots.environmentType, "setBinding:", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_environment_primitive_setNewBinding);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Environment::setBinding:", context->roots.environmentType, "setBinding:", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_environment_primitive_setBinding);
    
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Environment::setNewSymbol:bindingWithValue:", context->roots.environmentType, "setNewSymbol:bindingWithValue:", 3, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_environment_primitive_setNewSymbolBindingWithValue);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Environment::setSymbol:bindingWithValue:", context->roots.environmentType, "setSymbol:bindingWithValue:", 3, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_environment_primitive_setSymbolBindingWithValue);
}