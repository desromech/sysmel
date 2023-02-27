#ifndef TUUVM_STRING_H
#define TUUVM_STRING_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

/**
 * Creates a tuuvm string
 */
TUUVM_API tuuvm_tuple_t tuuvm_string_createWithString(tuuvm_context_t *context, size_t stringSize, const char *string);

/**
 * Creates a tuuvm string with reversed content of.
 */
TUUVM_API tuuvm_tuple_t tuuvm_string_createWithReversedString(tuuvm_context_t *context, size_t stringSize, const char *string);

/**
 * Creates an empty with the specified size.
 */
TUUVM_API tuuvm_tuple_t tuuvm_string_createEmptyWithSize(tuuvm_context_t *context, size_t stringSize);

/**
 * Creates a string by prepending the specified prefix.
 */
TUUVM_API tuuvm_tuple_t tuuvm_string_createWithPrefix(tuuvm_context_t *context, const char *prefix, tuuvm_tuple_t string);

/**
 * Creates a string by appending the specified suffix.
 */
TUUVM_API tuuvm_tuple_t tuuvm_string_createWithSuffix(tuuvm_context_t *context, tuuvm_tuple_t string, const char *suffix);

/**
 * Creates a tuuvm string
 */
TUUVM_API tuuvm_tuple_t tuuvm_string_createWithCString(tuuvm_context_t *context, const char *cstring);

/**
 * Creates a tuuvm by concatenating two of them.
 */
TUUVM_API tuuvm_tuple_t tuuvm_string_concat(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right);

/**
 * Gets or create a tuuvm symbol from a string.
 */
TUUVM_API tuuvm_tuple_t tuuvm_symbol_internWithString(tuuvm_context_t *context, size_t stringSize, const char *string);

/**
 * Gets or create a tuuvm symbol from a string.
 */
TUUVM_API tuuvm_tuple_t tuuvm_symbol_internFromTuple(tuuvm_context_t *context, tuuvm_tuple_t byteTuple);

/**
 * Gets or create a tuuvm symbol from a C string.
 */
TUUVM_API tuuvm_tuple_t tuuvm_symbol_internWithCString(tuuvm_context_t *context, const char *cstring);

/**
 * Computes the hash for a string with the specified bytes.
 */
TUUVM_API size_t tuuvm_string_computeHashWithBytes(size_t size, const uint8_t *bytes);

/**
 * The hash for string.
 */
TUUVM_API size_t tuuvm_string_hash(tuuvm_tuple_t string);

/**
 * The equals comparison for a string.
 */
TUUVM_API bool tuuvm_string_equals(tuuvm_tuple_t a, tuuvm_tuple_t b);

/**
 * The equals comparison for a string with a c-string.
 */
TUUVM_API bool tuuvm_string_equalsCString(tuuvm_tuple_t string, const char *cstring);

/**
 * Does the string end with the specified c string?
 */
TUUVM_API bool tuuvm_string_endsWithCString(tuuvm_tuple_t string, const char *cstring);

/**
 * The primitive string hash function.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_string_primitive_hash(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments);

/**
 * The primitive string equals function.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_string_primitive_equals(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments);

/**
 * A default implementation of the asString function.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_tuple_defaultAsString(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * A default implementation of the printString function.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_tuple_defaultPrintString(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Turns any tuple into a string representation.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_tuple_asString(tuuvm_context_t *context, tuuvm_tuple_t tuple);

/**
 * Turns any tuple into a string representation that can be used for source code input.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_tuple_printString(tuuvm_context_t *context, tuuvm_tuple_t tuple);

#endif //TUUVM_STRING_H
