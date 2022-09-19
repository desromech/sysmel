#ifndef TUUVM_STRING_H
#define TUUVM_STRING_H

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

/**
 * Creates a tuuvm string
 */
TUUVM_API tuuvm_object_tuple_t *tuuvm_string_fromCString(tuuvm_context_t *context, const char *cstring);

/**
 * Gets or create a tuuvm symbol from a C string.
 */
TUUVM_API tuuvm_object_tuple_t *tuuvm_symbol_intern(tuuvm_context_t *context, tuuvm_object_tuple_t *byteTuple);

/**
 * Gets or create a tuuvm symbol from a C string.
 */
TUUVM_API tuuvm_object_tuple_t *tuuvm_symbol_fromCString(tuuvm_context_t *context, const char *cstring);

#endif //TUUVM_STRING_H
