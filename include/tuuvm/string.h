#ifndef TUUVM_STRING_H
#define TUUVM_STRING_H

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

/**
 * Creates a tuuvm string
 */
TUUVM_API tuuvm_tuple_t tuuvm_string_createWithString(tuuvm_context_t *context, size_t stringSize, const char *string);

/**
 * Creates a tuuvm string
 */
TUUVM_API tuuvm_tuple_t tuuvm_string_createWithCString(tuuvm_context_t *context, const char *cstring);

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
 * The hash for string.
 */
TUUVM_API size_t tuuvm_string_hash(tuuvm_tuple_t string);

/**
 * The equals comparison for a string.
 */
TUUVM_API bool tuuvm_string_equals(tuuvm_tuple_t a, tuuvm_tuple_t b);

/**
 * The primitive string hash function.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_string_primitive_hash(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments);

/**
 * The primitive string equals function.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_string_primitive_equals(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments);

#endif //TUUVM_STRING_H
