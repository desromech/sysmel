#ifndef SYSBVM_INTERPRETER_H
#define SYSBVM_INTERPRETER_H

#pragma once

#include "tuple.h"

typedef struct sysbvm_context_s sysbvm_context_t;

/**
 * Analyzes an AST node with the given environment.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_interpreter_analyzeASTWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t astNode, sysbvm_tuple_t environment);

/**
 * Evaluates an AST node with the given environment.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_interpreter_evaluateASTWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t astNode, sysbvm_tuple_t environment);

/**
 * Analyzes and evaluates an AST node with the given environment.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t astNode, sysbvm_tuple_t environment);

/**
 * Analyzes and evaluates an AST node with the given environment.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_interpreter_validateThenAnalyzeAndEvaluateASTWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t astNode, sysbvm_tuple_t environment);


/**
 * Analyzes and evaluates the source code with the given environment.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_interpreter_analyzeAndEvaluateSourceCodeWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t sourceCode);

/**
 * Validates, then analyzes and evaluates the source code with the given environment.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_interpreter_validateThenAnalyzeAndEvaluateSourceCodeWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t sourceCode);

/**
 * Analyzes and evaluates a C String with the given environment.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_interpreter_analyzeAndEvaluateCStringWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t environment, const char *sourceCodeText, const char *sourceCodeName, const char *sourceCodeLanguage);

/**
 * Analyzes and evaluates a C String with the given environment.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_interpreter_analyzeAndEvaluateStringWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t sourceCodeText, sysbvm_tuple_t sourceCodeDirectory, sysbvm_tuple_t sourceCodeName, sysbvm_tuple_t sourceCodeLanguage);

/**
 * Applies a closure AST function with the specified arguments.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_interpreter_applyClosureASTFunction(sysbvm_context_t *context, sysbvm_tuple_t function, size_t argumentCount, sysbvm_tuple_t *arguments, sysbvm_bitflags_t applicationFlags);

/**
 * Loads the specified file with the given name.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_interpreter_loadSourceNamedWithSolvedPath(sysbvm_context_t *context, sysbvm_tuple_t filename);

#endif //SYSBVM_INTERPRETER_H
