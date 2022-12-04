#ifndef TUUVM_INTEGER_H
#define TUUVM_INTEGER_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

/**
 * Parses an integer from a string.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_integer_parseString(tuuvm_context_t *context, size_t stringSize, const char *string);

/**
 * Parses an integer from a C string.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_integer_parseCString(tuuvm_context_t *context, const char *cstring);

/**
 * Adds two integers.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_integer_add(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right);

/**
 * Subtracts two integers.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_integer_subtract(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right);

/**
 * Negates an integer.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_integer_negate(tuuvm_context_t *context, tuuvm_tuple_t integer);

/**
 * Multiplies two integers.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_integer_multiply(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right);

/**
 * Divides two integers.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_integer_divide(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right);

/**
 * Remainder of two integers.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_integer_remainder(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right);

/**
 * Converts an integer into a string.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_integer_printString(tuuvm_context_t *context, tuuvm_tuple_t integer);

#endif //TUUVM_INTERPRETER_H
