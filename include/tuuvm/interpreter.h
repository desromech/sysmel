#ifndef TUUVM_INTERPRETER_H
#define TUUVM_INTERPRETER_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

/**
 * Analyzes an AST node with the given environment.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_interpreter_analyzeASTWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t astNode, tuuvm_tuple_t environment);

/**
 * Evaluates an AST node with the given environment.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_interpreter_evaluateASTWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t astNode, tuuvm_tuple_t environment);

/**
 * Analyzes and evaluates an AST node with the given environment.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_interpreter_analyzeAndEvaluateASTWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t astNode, tuuvm_tuple_t environment);

/**
 * Analyzes and evaluates an AST node with the given environment.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_interpreter_validateThenAnalyzeAndEvaluateASTWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t astNode, tuuvm_tuple_t environment);


/**
 * Analyzes and evaluates the source code with the given environment.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_interpreter_analyzeAndEvaluateSourceCodeWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t sourceCode);

/**
 * Validates, then analyzes and evaluates the source code with the given environment.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_interpreter_validateThenAnalyzeAndEvaluateSourceCodeWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t sourceCode);

/**
 * Analyzes and evaluates a C String with the given environment.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_interpreter_analyzeAndEvaluateCStringWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t environment, const char *sourceCodeText, const char *sourceCodeName, const char *sourceCodeLanguage);

/**
 * Analyzes and evaluates a C String with the given environment.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_interpreter_analyzeAndEvaluateStringWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t environment, tuuvm_tuple_t sourceCodeText, tuuvm_tuple_t sourceCodeDirectory, tuuvm_tuple_t sourceCodeName, tuuvm_tuple_t sourceCodeLanguage);

/**
 * Applies a closure AST function with the specified arguments.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_interpreter_applyClosureASTFunction(tuuvm_context_t *context, tuuvm_tuple_t function, size_t argumentCount, tuuvm_tuple_t *arguments, uint32_t applicationFlags);

/**
 * Loads the specified file with the given name.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_interpreter_loadSourceNamedWithSolvedPath(tuuvm_context_t *context, tuuvm_tuple_t filename);

#endif //TUUVM_INTERPRETER_H
