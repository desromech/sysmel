#ifndef TUUVM_PARSER_H
#define TUUVM_PARSER_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

/**
 * Parses a sequence tokens (given as an array slice).
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_parser_parseTokens(tuuvm_context_t *context, tuuvm_tuple_t sourceCode, tuuvm_tuple_t tokenSequence);

/**
 * Parses a source code
 */
TUUVM_API tuuvm_tuple_t tuuvm_parser_parseSourceCode(tuuvm_context_t *context, tuuvm_tuple_t sourceCode);

/**
 * Parses a source code c string
 */
TUUVM_API tuuvm_tuple_t tuuvm_parser_parseCString(tuuvm_context_t *context, const char *sourceCodeText, const char *sourceCodeName);

#endif //TUUVM_PARSER_H
