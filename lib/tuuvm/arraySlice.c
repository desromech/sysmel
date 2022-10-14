#include "tuuvm/arraySlice.h"
#include "tuuvm/array.h"
#include "tuuvm/errors.h"
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

TUUVM_API tuuvm_tuple_t tuuvm_arraySlice_createWithArrayOfSize(tuuvm_context_t *context, size_t size)
{
    return tuuvm_arraySlice_createWithOffsetAndSize(context, tuuvm_array_create(context, size), 0, size);
}

TUUVM_API size_t tuuvm_arraySlice_getSize(tuuvm_tuple_t arraySlice)
{
    if(!tuuvm_tuple_isNonNullPointer(arraySlice)) return 0;
    return tuuvm_tuple_size_decode(((tuuvm_arraySlice_t*)arraySlice)->size);
}

TUUVM_API tuuvm_tuple_t tuuvm_arraySlice_at(tuuvm_tuple_t arraySlice, size_t index)
{
    if(!tuuvm_tuple_isNonNullPointer(arraySlice)) return TUUVM_NULL_TUPLE;

    tuuvm_arraySlice_t *arraySliceObject = (tuuvm_arraySlice_t*)arraySlice;
    size_t offset = tuuvm_tuple_size_decode(arraySliceObject->offset);
    size_t size = tuuvm_tuple_size_decode(arraySliceObject->size);
    if(index >= size)
        tuuvm_error_indexOutOfBounds();

    return tuuvm_arrayOrByteArray_at(arraySliceObject->elements, offset + index);
}

TUUVM_API void tuuvm_arraySlice_atPut(tuuvm_tuple_t arraySlice, size_t index, tuuvm_tuple_t value)
{
    if(!tuuvm_tuple_isNonNullPointer(arraySlice))
        return;

    tuuvm_arraySlice_t *arraySliceObject = (tuuvm_arraySlice_t*)arraySlice;
    size_t offset = tuuvm_tuple_size_decode(arraySliceObject->offset);
    size_t size = tuuvm_tuple_size_decode(arraySliceObject->size);
    if(index >= size)
        tuuvm_error_indexOutOfBounds();
 
    return tuuvm_arrayOrByteArray_atPut(arraySliceObject->elements, offset + index, value);
}
