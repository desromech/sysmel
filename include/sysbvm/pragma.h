#ifndef SYSBVM_PRAGMA_H
#define SYSBVM_PRAGMA_H

#include "tuple.h"

typedef struct sysbvm_context_s sysbvm_context_t;

typedef struct sysbvm_pragma_s
{
    sysbvm_tuple_header_t header;
    sysbvm_tuple_t selector;
    sysbvm_tuple_t arguments;
} sysbvm_pragma_t;

/**
 * Creates a macro context.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_pragma_create(sysbvm_context_t *context, sysbvm_tuple_t selector, sysbvm_tuple_t arguments);

#endif //SYSBVM_PRAGMA_H