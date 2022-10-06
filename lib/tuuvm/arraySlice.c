#include "tuuvm/arraySlice.h"
#include "internal/context.h"

TUUVM_API tuuvm_tuple_t tuuvm_arraySlice_create(tuuvm_context_t *context, tuuvm_tuple_t elements, tuuvm_tuple_t offset, tuuvm_tuple_t size)
{
    tuuvm_arraySlice_t *result = (tuuvm_arraySlice_t*)tuuvm_context_allocatePointerTuple(context, context->roots.arraySliceType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_arraySlice_t));
    result->elements = elements;
    result->offset = offset;
    result->size = size;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_arraySlice_createWithOffsetAndSize(tuuvm_context_t *context, tuuvm_tuple_t elements, size_t offset, size_t count)
{
    return tuuvm_arraySlice_create(context, elements, tuuvm_tuple_size_encode(context, offset), tuuvm_tuple_size_encode(context, count));
}
