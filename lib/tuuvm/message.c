#include "tuuvm/message.h"
#include "internal/context.h"
#include <stdlib.h>

TUUVM_API tuuvm_tuple_t tuuvm_message_create(tuuvm_context_t *context, tuuvm_tuple_t selector, tuuvm_tuple_t arguments)
{
    tuuvm_message_t *result = (tuuvm_message_t*)tuuvm_context_allocatePointerTuple(context, context->roots.messageType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_message_t));
    result->selector = selector;
    result->arguments = arguments;
    return (tuuvm_tuple_t)result;
}
