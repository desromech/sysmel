#include "tuuvm/association.h"
#include "internal/context.h"
#include <stdlib.h>

TUUVM_API tuuvm_tuple_t tuuvm_association_create(tuuvm_context_t *context, tuuvm_tuple_t key, tuuvm_tuple_t value)
{
    tuuvm_association_t *result = (tuuvm_association_t*)tuuvm_context_allocatePointerTuple(context, context->roots.associationType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_association_t));
    result->key = key;
    result->value = value;
    return (tuuvm_tuple_t)result;
}
