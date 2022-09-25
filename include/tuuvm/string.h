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

#endif //TUUVM_STRING_H
