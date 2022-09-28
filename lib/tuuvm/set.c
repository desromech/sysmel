#include "tuuvm/set.h"
#include "internal/context.h"

TUUVM_API tuuvm_tuple_t tuuvm_set_create(tuuvm_context_t *context, tuuvm_tuple_t equalsFunction, tuuvm_tuple_t hashFunction)
{
    tuuvm_set_t *result = (tuuvm_set_t*)tuuvm_context_allocatePointerTuple(context, context->roots.setType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_set_t));
    result->size = tuuvm_tuple_size_encode(context, 0);
    result->equalsFunction = equalsFunction;
    result->hashFunction = hashFunction;
    return (tuuvm_tuple_t)result;
}
