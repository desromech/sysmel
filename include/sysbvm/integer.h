#ifndef SYSBVM_INTEGER_H
#define SYSBVM_INTEGER_H

#pragma once

#include "tuple.h"

typedef struct sysbvm_context_s sysbvm_context_t;

typedef struct sysbvm_integer_s
{
    sysbvm_tuple_header_t header;
    uint32_t words[];
} sysbvm_integer_t;

/**
 * Parses an integer from a string.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_integer_parseString(sysbvm_context_t *context, size_t stringSize, const char *string);

/**
 * Parses an integer from a C string.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_integer_parseCString(sysbvm_context_t *context, const char *cstring);

/**
 * Adds two integers.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_integer_add(sysbvm_context_t *context, sysbvm_tuple_t left, sysbvm_tuple_t right);

/**
 * Subtracts two integers.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_integer_subtract(sysbvm_context_t *context, sysbvm_tuple_t left, sysbvm_tuple_t right);

/**
 * Negates an integer.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_integer_negated(sysbvm_context_t *context, sysbvm_tuple_t integer);

/**
 * Multiplies two integers.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_integer_multiply(sysbvm_context_t *context, sysbvm_tuple_t left, sysbvm_tuple_t right);

/**
 * Divides two integers.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_integer_divide(sysbvm_context_t *context, sysbvm_tuple_t left, sysbvm_tuple_t right);

/**
 * Remainder of two integers.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_integer_remainder(sysbvm_context_t *context, sysbvm_tuple_t left, sysbvm_tuple_t right);

/**
 * Factorial of an integer. Used for testing purposes.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_integer_factorial(sysbvm_context_t *context, sysbvm_tuple_t integer);

/**
 * Comparison of two integers.
 */ 
SYSBVM_API int sysbvm_integer_compare(sysbvm_context_t *context, sysbvm_tuple_t left, sysbvm_tuple_t right);

/**
 * Equals comparison of two integers.
 */ 
SYSBVM_API bool sysbvm_integer_equals(sysbvm_context_t *context, sysbvm_tuple_t left, sysbvm_tuple_t right);

/**
 * Not equals comparison of two integers.
 */ 
SYSBVM_API bool sysbvm_integer_notEquals(sysbvm_context_t *context, sysbvm_tuple_t left, sysbvm_tuple_t right);

/**
 * Less than comparison of two integers.
 */ 
SYSBVM_API bool sysbvm_integer_lessThan(sysbvm_context_t *context, sysbvm_tuple_t left, sysbvm_tuple_t right);

/**
 * Less or equals comparison of two integers.
 */ 
SYSBVM_API bool sysbvm_integer_lessEquals(sysbvm_context_t *context, sysbvm_tuple_t left, sysbvm_tuple_t right);

/**
 * Greater than comparison of two integers.
 */ 
SYSBVM_API bool sysbvm_integer_greaterThan(sysbvm_context_t *context, sysbvm_tuple_t left, sysbvm_tuple_t right);

/**
 * Greater or equals comparison of two integers.
 */ 
SYSBVM_API bool sysbvm_integer_greaterEquals(sysbvm_context_t *context, sysbvm_tuple_t left, sysbvm_tuple_t right);

/**
 * Converts an integer into a string.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_integer_printString(sysbvm_context_t *context, sysbvm_tuple_t integer);

/**
 * Converts an integer into an hex string.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_integer_toHexString(sysbvm_context_t *context, sysbvm_tuple_t integer);

#endif //SYSBVM_INTERPRETER_H
