#ifndef TUUVM_PRAGMA_H
#define TUUVM_PRAGMA_H

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef struct tuuvm_pragma_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t selector;
    tuuvm_tuple_t arguments;
} tuuvm_pragma_t;

/**
 * Creates a macro context.
 */
TUUVM_API tuuvm_tuple_t tuuvm_pragma_create(tuuvm_context_t *context, tuuvm_tuple_t selector, tuuvm_tuple_t arguments);

#endif //TUUVM_PRAGMA_H