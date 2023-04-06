#ifndef SYSBVM_PARSER_H
#define SYSBVM_PARSER_H

#pragma once

#include "tuple.h"

typedef struct sysbvm_context_s sysbvm_context_t;

/**
 * Parses a sequence tokens (given as an array slice).
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_parser_parseTokens(sysbvm_context_t *context, sysbvm_tuple_t sourceCode, sysbvm_tuple_t tokenSequence);

/**
 * Parses a source code
 */
SYSBVM_API sysbvm_tuple_t sysbvm_parser_parseSourceCode(sysbvm_context_t *context, sysbvm_tuple_t sourceCode);

/**
 * Parses a source code c string
 */
SYSBVM_API sysbvm_tuple_t sysbvm_parser_parseCString(sysbvm_context_t *context, const char *sourceCodeText, const char *sourceCodeName);

#endif //SYSBVM_PARSER_H
