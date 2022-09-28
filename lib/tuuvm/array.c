#include "tuuvm/array.h"
#include "internal/context.h"

TUUVM_API tuuvm_tuple_t tuuvm_array_create(tuuvm_context_t *context, tuuvm_tuple_t slotCount)
{
    return (tuuvm_tuple_t)tuuvm_context_allocatePointerTuple(context, context->roots.arrayType, slotCount);
}

TUUVM_API tuuvm_tuple_t tuuvm_byteArray_create(tuuvm_context_t *context, tuuvm_tuple_t size)
{
    return (tuuvm_tuple_t)tuuvm_context_allocateByteTuple(context, context->roots.byteArrayType, size);
}
