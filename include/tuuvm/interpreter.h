#ifndef TUUVM_INTERPRETER_H
#define TUUVM_INTERPRETER_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

/**
 * Parses a sequence tokens (given as an array slice).
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_parser_parseTokens(tuuvm_context_t *context, tuuvm_tuple_t sourceCode, tuuvm_tuple_t tokenSequence);

#endif //TUUVM_INTERPRETER_H
