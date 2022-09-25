#ifndef TUUVM_TYPE_H
#define TUUVM_TYPE_H

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef struct tuuvm_type_tuple_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t name;
} tuuvm_type_tuple_t;

/**
 * Creates an anonymous type.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_createAnonymous(tuuvm_context_t *context);

/**
 * Creates a type with the specified name.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_createWithName(tuuvm_context_t *context, tuuvm_tuple_t name);

/**
 * Sets the name of a type
 */
TUUVM_INLINE void tuuvm_type_setName(tuuvm_tuple_t type, tuuvm_tuple_t newName)
{
    ((tuuvm_type_tuple_t*)type)->name = newName;
}
#endif //TUUVM_TYPE_H