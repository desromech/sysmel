#ifndef SYSBVM_STRING_H
#define SYSBVM_STRING_H

#pragma once

#include "tuple.h"

typedef struct sysbvm_context_s sysbvm_context_t;

/**
 * Creates a sysbvm string
 */
SYSBVM_API sysbvm_tuple_t sysbvm_string_createWithString(sysbvm_context_t *context, size_t stringSize, const char *string);

/**
 * Creates a sysbvm string with reversed content of.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_string_createWithReversedString(sysbvm_context_t *context, size_t stringSize, const char *string);

/**
 * Creates an empty with the specified size.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_string_createEmptyWithSize(sysbvm_context_t *context, size_t stringSize);

/**
 * Creates a string by prepending the specified prefix.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_string_createWithPrefix(sysbvm_context_t *context, const char *prefix, sysbvm_tuple_t string);

/**
 * Creates a string by appending the specified suffix.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_string_createWithSuffix(sysbvm_context_t *context, sysbvm_tuple_t string, const char *suffix);

/**
 * Creates a string by removing the specified suffix.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_string_createWithoutSuffix(sysbvm_context_t *context, sysbvm_tuple_t string, const char *suffix);

/**
 * Creates a sysbvm string
 */
SYSBVM_API sysbvm_tuple_t sysbvm_string_createWithCString(sysbvm_context_t *context, const char *cstring);

/**
 * Creates a sysbvm by concatenating two of them.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_string_concat(sysbvm_context_t *context, sysbvm_tuple_t left, sysbvm_tuple_t right);

/**
 * Gets or create a sysbvm symbol from a string.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_symbol_internWithString(sysbvm_context_t *context, size_t stringSize, const char *string);

/**
 * Gets or create a sysbvm symbol from a string.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_symbol_internFromTuple(sysbvm_context_t *context, sysbvm_tuple_t byteTuple);

/**
 * Gets or create a sysbvm symbol from a C string.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_symbol_internWithCString(sysbvm_context_t *context, const char *cstring);

/**
 * Computes the hash for a string with the specified bytes.
 */
SYSBVM_API size_t sysbvm_string_computeHashWithBytes(size_t size, const uint8_t *bytes);

/**
 * The hash for string.
 */
SYSBVM_API size_t sysbvm_string_hash(sysbvm_tuple_t string);

/**
 * The equals comparison for a string.
 */
SYSBVM_API bool sysbvm_string_equals(sysbvm_tuple_t a, sysbvm_tuple_t b);

/**
 * The equals comparison for a string with a c-string.
 */
SYSBVM_API bool sysbvm_string_equalsCString(sysbvm_tuple_t string, const char *cstring);

/**
 * Does the string end with the specified c string?
 */
SYSBVM_API bool sysbvm_string_endsWithCString(sysbvm_tuple_t string, const char *cstring);

/**
 * The primitive string hash function.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_string_primitive_hash(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments);

/**
 * The primitive string equals function.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_string_primitive_equals(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments);

/**
 * A default implementation of the asString function.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_tuple_defaultAsString(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * A default implementation of the printString function.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_tuple_defaultPrintString(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Turns any tuple into a string representation.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_tuple_asString(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Turns any tuple into a string representation that can be used for source code input.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_tuple_printString(sysbvm_context_t *context, sysbvm_tuple_t tuple);

#endif //SYSBVM_STRING_H
