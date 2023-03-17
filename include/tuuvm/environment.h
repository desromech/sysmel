#ifndef TUUVM_ENVIRONMENT_H
#define TUUVM_ENVIRONMENT_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef struct tuuvm_environment_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t parent;
    tuuvm_tuple_t symbolTable;
} tuuvm_environment_t;

typedef tuuvm_environment_t tuuvm_namespace_t;

typedef struct tuuvm_analysisAndEvaluationEnvironment_s
{
    tuuvm_environment_t super;
    tuuvm_tuple_t analyzerToken;
    tuuvm_tuple_t expectedType;
    tuuvm_tuple_t returnTarget;
    tuuvm_tuple_t breakTarget;
    tuuvm_tuple_t continueTarget;
} tuuvm_analysisAndEvaluationEnvironment_t;

typedef struct tuuvm_functionActivationEnvironment_s
{
    tuuvm_analysisAndEvaluationEnvironment_t super;
    tuuvm_tuple_t function;
    tuuvm_tuple_t functionDefinition;
    tuuvm_tuple_t dependentFunctionType;
    tuuvm_tuple_t argumentVectorSize;
    tuuvm_tuple_t captureVector;
    tuuvm_tuple_t valueVector;
} tuuvm_functionActivationEnvironment_t;

typedef struct tuuvm_functionAnalysisEnvironment_s
{
    tuuvm_analysisAndEvaluationEnvironment_t super;
    tuuvm_tuple_t functionDefinition;
    tuuvm_tuple_t captureBindingTable;
    tuuvm_tuple_t captureBindingList;
    tuuvm_tuple_t argumentBindingList;
    tuuvm_tuple_t localBindingList;
    tuuvm_tuple_t pragmaList;
    tuuvm_tuple_t primitiveName;
    tuuvm_tuple_t hasBreakTarget;
    tuuvm_tuple_t hasContinueTarget;
} tuuvm_functionAnalysisEnvironment_t;

typedef struct tuuvm_localAnalysisEnvironment_s
{
    tuuvm_analysisAndEvaluationEnvironment_t super;
} tuuvm_localAnalysisEnvironment_t;

typedef struct tuuvm_symbolBinding_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t name;
    tuuvm_tuple_t sourcePosition;
    tuuvm_tuple_t type;
} tuuvm_symbolBinding_t;

typedef struct tuuvm_symbolAnalysisBinding_s
{
    tuuvm_symbolBinding_t super;
    tuuvm_tuple_t ownerFunction;
    tuuvm_tuple_t vectorIndex;
} tuuvm_symbolAnalysisBinding_t;

typedef struct tuuvm_symbolArgumentBinding_s
{
    tuuvm_symbolAnalysisBinding_t super;
} tuuvm_symbolArgumentBinding_t;

typedef struct tuuvm_symbolLocalBinding_s
{
    tuuvm_symbolAnalysisBinding_t super;
} tuuvm_symbolLocalBinding_t;

typedef struct tuuvm_symbolCaptureBinding_s
{
    tuuvm_symbolAnalysisBinding_t super;
    tuuvm_tuple_t capturedBinding;
} tuuvm_symbolCaptureBinding_t;

typedef struct tuuvm_symbolMacroValueBinding_s
{
    tuuvm_symbolBinding_t super;
    tuuvm_tuple_t expansion;
} tuuvm_symbolMacroValueBinding_t;

typedef struct tuuvm_symbolValueBinding_s
{
    tuuvm_symbolBinding_t super;
    tuuvm_tuple_t value;
} tuuvm_symbolValueBinding_t;

/**
 * Creates an analyzer token.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_analyzerToken_create(tuuvm_context_t *context);

/**
 * Is the symbol binding a value?
 */
TUUVM_API bool tuuvm_symbolBinding_isValue(tuuvm_context_t *context, tuuvm_tuple_t binding);

/**
 * Is the symbol binding a value?
 */
TUUVM_API bool tuuvm_symbolBinding_isMacroValue(tuuvm_context_t *context, tuuvm_tuple_t binding);

/**
 * Is the symbol binding an analysis specific binding?
 */
TUUVM_API bool tuuvm_symbolBinding_isAnalysisBinding(tuuvm_context_t *context, tuuvm_tuple_t binding);

/**
 * Gets the name from the symbol value binding.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_symbolBinding_getName(tuuvm_tuple_t binding)
{
    if(!tuuvm_tuple_isNonNullPointer(binding)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_symbolBinding_t*)binding)->name;
}

/**
 * Gets the value from the symbol value binding.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_symbolBinding_getType(tuuvm_tuple_t binding)
{
    if(!tuuvm_tuple_isNonNullPointer(binding)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_symbolBinding_t*)binding)->type;
}

/**
 * Creates a symbol argument binding.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_symbolArgumentBinding_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t name, tuuvm_tuple_t type, tuuvm_tuple_t ownerFunction, size_t vectorIndex);

/**
 * Creates a symbol local binding.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_symbolLocalBinding_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t name, tuuvm_tuple_t type, tuuvm_tuple_t ownerFunction, size_t vectorIndex);

/**
 * Creates a symbol capture binding.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_symbolCaptureBinding_create(tuuvm_context_t *context, tuuvm_tuple_t capturedBinding, tuuvm_tuple_t ownerFunction, size_t vectorIndex);

/**
 * Gets the value from the symbol value binding.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_symbolCaptureBinding_getSourceBinding(tuuvm_tuple_t binding)
{
    if(!tuuvm_tuple_isNonNullPointer(binding)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_symbolCaptureBinding_t*)binding)->capturedBinding;
}

/**
 * Creates a symbol macro value binding.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_symbolMacroValueBinding_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t name, tuuvm_tuple_t expansion);

/**
 * Gets the expansion from the symbol macro value binding.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_symbolMacroValueBinding_getExpansion(tuuvm_tuple_t binding)
{
    if(!tuuvm_tuple_isNonNullPointer(binding) || tuuvm_tuple_getSizeInSlots(binding) < TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_symbolValueBinding_t)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_symbolMacroValueBinding_t*)binding)->expansion;
}

/**
 * Creates a symbol value binding.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_symbolValueBinding_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t name, tuuvm_tuple_t value);

/**
 * Gets the value from the symbol value binding.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_symbolValueBinding_getValue(tuuvm_tuple_t binding)
{
    if(!tuuvm_tuple_isNonNullPointer(binding) || tuuvm_tuple_getSizeInSlots(binding) < TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_symbolValueBinding_t)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_symbolValueBinding_t*)binding)->value;
}

/**
 * Creates an environment.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_environment_create(tuuvm_context_t *context, tuuvm_tuple_t parent);

/**
 * Creates an environment that is used for simultaneous analysis and evaluation.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_analysisAndEvaluationEnvironment_create(tuuvm_context_t *context, tuuvm_tuple_t parent);

/**
 * Creates a namespace.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_namespace_create(tuuvm_context_t *context, tuuvm_tuple_t parent);

/**
 * Creates an environment used for function action.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_functionActivationEnvironment_create(tuuvm_context_t *context, tuuvm_tuple_t parent, tuuvm_tuple_t function);

/**
 * Creates an environment used for dependent function type activation and evaluation.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_functionActivationEnvironment_createForDependentFunctionType(tuuvm_context_t *context, tuuvm_tuple_t parent, tuuvm_tuple_t dependentFunctionType);

/**
 * Creates an environment used for function definition analysis.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_functionAnalysisEnvironment_create(tuuvm_context_t *context, tuuvm_tuple_t parent, tuuvm_tuple_t functionDefinition);

/**
 * Creates an environment used for lexical scope analysis.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_localAnalysisEnvironment_create(tuuvm_context_t *context, tuuvm_tuple_t parent);

/**
 * Creates an environment.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_environment_getIntrinsicsBuiltInEnvironment(tuuvm_context_t *context);

/**
 * Creates an environment.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_environment_createDefaultForEvaluation(tuuvm_context_t *context);

/**
 * Creates an environment.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_environment_createDefaultForSourceCodeEvaluation(tuuvm_context_t *context, tuuvm_tuple_t sourceCode);

/**
 * Sets a new symbol binding in the environment.
 */ 
TUUVM_API void tuuvm_environment_setNewBinding(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t binding);

/**
 * Sets a symbol binding in the environment.
 */ 
TUUVM_API void tuuvm_environment_setBinding(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t binding);

/**
 * Sets a new symbol binding with value the environment.
 */ 
TUUVM_API void tuuvm_environment_setNewSymbolBindingWithValue(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t symbol, tuuvm_tuple_t value);

/**
 * Sets a new symbol binding with value in the environment.
 */ 
TUUVM_API void tuuvm_environment_setNewSymbolBindingWithValueAtSourcePosition(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t symbol, tuuvm_tuple_t value, tuuvm_tuple_t sourcePosition);

/**
 * Sets a symbol binding with value in the environment.
 */ 
TUUVM_API void tuuvm_environment_setSymbolBindingWithValue(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t symbol, tuuvm_tuple_t value);

/**
 * Looks a symbol recursively on an environment.
 */ 
TUUVM_API bool tuuvm_environment_lookSymbolRecursively(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t symbol, tuuvm_tuple_t *outBinding);

/**
 * Evaluates the given symbol binding in this environment.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_environment_evaluateSymbolBinding(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t binding);

/**
 * Sets the activation value for the given binding in this environment.
 */ 
TUUVM_API void tuuvm_functionActivationEnvironment_setBindingActivationValue(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t binding, tuuvm_tuple_t value, tuuvm_tuple_t sourcePosition);

/**
 * Looks for the return target on an environment.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_environment_lookReturnTargetRecursively(tuuvm_context_t *context, tuuvm_tuple_t environment);

/**
 * Looks for the break target on an environment.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_environment_lookBreakTargetRecursively(tuuvm_context_t *context, tuuvm_tuple_t environment);

/**
 * Looks for the continue target on an environment.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_environment_lookContinueTargetRecursively(tuuvm_context_t *context, tuuvm_tuple_t environment);

/**
 * Sets the break target.
 */ 
TUUVM_API void tuuvm_analysisAndEvaluationEnvironment_setBreakTarget(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t breakTarget);

/**
 * Sets the continue target.
 */ 
TUUVM_API void tuuvm_analysisAndEvaluationEnvironment_setContinueTarget(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t continueTarget);

/**
 * Sets the return target.
 */ 
TUUVM_API void tuuvm_analysisAndEvaluationEnvironment_setReturnTarget(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t returnTarget);

/**
 * Gets the expected type.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_analysisAndEvaluationEnvironment_getExpectedType(tuuvm_context_t *context, tuuvm_tuple_t environment);

/**
 * Sets the expected type.
 */ 
TUUVM_API void tuuvm_analysisAndEvaluationEnvironment_setExpectedType(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t newExpectedType);

/**
 * Clears the unwinding record fields in the environment
 */ 

TUUVM_API void tuuvm_analysisAndEvaluationEnvironment_clearUnwindingRecords(tuuvm_context_t *context, tuuvm_tuple_t environment);

/**
 * Clears the analyzer token.
 */ 
TUUVM_API void tuuvm_analysisAndEvaluationEnvironment_clearAnalyzerToken(tuuvm_context_t *context, tuuvm_tuple_t environment);

/**
 * Ensures a valid analyzer token
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(tuuvm_context_t *context, tuuvm_tuple_t environment);

/**
 * Is this an analysis and evaluation environment?
 */
TUUVM_API bool tuuvm_environment_isAnalysisAndEvaluationEnvironment(tuuvm_context_t *context, tuuvm_tuple_t environment);

/**
 * Is this an analysis environment?
 */
TUUVM_API bool tuuvm_environment_isAnalysisEnvironment(tuuvm_context_t *context, tuuvm_tuple_t environment);

/**
 * Is this an analysis environment?
 */
TUUVM_API bool tuuvm_environment_isFunctionAnalysisEnvironment(tuuvm_context_t *context, tuuvm_tuple_t environment);

/**
 * Look recursively for the function analysis environment.
 */
TUUVM_API tuuvm_tuple_t tuuvm_environment_lookFunctionAnalysisEnvironmentRecursively(tuuvm_context_t *context, tuuvm_tuple_t environment);

/**
 * Looks a symbol recursively on an analysis environment.
 */ 
TUUVM_API bool tuuvm_analysisEnvironment_lookSymbolRecursively(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t symbol, tuuvm_tuple_t *outBinding);

/**
 * Makes a new argument binding in the analysis environment.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_analysisEnvironment_setNewSymbolArgumentBinding(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t name, tuuvm_tuple_t type);

/**
 * Makes a new local binding in the analysis environment.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_analysisEnvironment_setNewSymbolLocalBinding(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t name, tuuvm_tuple_t type);

/**
 * Adds a pragma
 */ 
TUUVM_API void tuuvm_analysisEnvironment_addPragma(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t pragma);

/**
 * Makes a new macro value binding in the analysis environment.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_environment_setNewMacroValueBinding(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t name, tuuvm_tuple_t expansion);

/**
 * Makes a new value binding in the analysis environment.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_analysisEnvironment_setNewValueBinding(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t name, tuuvm_tuple_t value);

#endif //TUUVM_ENVIRONMENT_H
