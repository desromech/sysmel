#include "sysbvm/association.h"
#include "internal/context.h"
#include <stdlib.h>

SYSBVM_API sysbvm_tuple_t sysbvm_association_create(sysbvm_context_t *context, sysbvm_tuple_t key, sysbvm_tuple_t value)
{
    sysbvm_association_t *result = (sysbvm_association_t*)sysbvm_context_allocatePointerTuple(context, context->roots.associationType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_association_t));
    result->key = key;
    result->value = value;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_weakValueAssociation_create(sysbvm_context_t *context, sysbvm_tuple_t key, sysbvm_tuple_t value)
{
    sysbvm_weakValueAssociation_t *result = (sysbvm_weakValueAssociation_t*)sysbvm_context_allocatePointerTuple(context, context->roots.weakValueAssociationType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_weakValueAssociation_t));
    sysbvm_tuple_markWeakObject((sysbvm_tuple_t)result);
    result->key = key;
    result->value = value;
    return (sysbvm_tuple_t)result;
}