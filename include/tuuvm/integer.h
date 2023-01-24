#ifndef TUUVM_INTEGER_H
#define TUUVM_INTEGER_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef struct tuuvm_integer_s
{
    tuuvm_tuple_header_t header;
    uint32_t words[];
} tuuvm_integer_t;

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
 * Factorial of an integer. Used for testing purposes.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_integer_factorial(tuuvm_context_t *context, tuuvm_tuple_t integer);

/**
 * Comparison of two integers.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_integer_compare(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right);

/**
 * Equals comparison of two integers.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_integer_equals(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right);

/**
 * Not equals comparison of two integers.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_integer_notEquals(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right);

/**
 * Less than comparison of two integers.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_integer_lessThan(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right);

/**
 * Less or equals comparison of two integers.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_integer_lessEquals(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right);

/**
 * Greater than comparison of two integers.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_integer_greaterThan(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right);

/**
 * Greater or equals comparison of two integers.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_integer_greaterEquals(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right);

/**
 * Converts an integer into a string.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_integer_printString(tuuvm_context_t *context, tuuvm_tuple_t integer);

#endif //TUUVM_INTERPRETER_H
