#ifndef TUUVM_MESSAGE_H
#define TUUVM_MESSAGE_H

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef struct tuuvm_message_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t selector;
    tuuvm_tuple_t arguments;
} tuuvm_message_t;

/**
 * Creates a message.
 */
TUUVM_API tuuvm_tuple_t tuuvm_message_create(tuuvm_context_t *context, tuuvm_tuple_t selector, tuuvm_tuple_t arguments);

#endif //TUUVM_MESSAGE_H