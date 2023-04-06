#include "sysbvm/arraySlice.h"
#include "sysbvm/array.h"
#include "sysbvm/errors.h"
#include "internal/context.h"

SYSBVM_API sysbvm_tuple_t sysbvm_arraySlice_create(sysbvm_context_t *context, sysbvm_tuple_t elements, sysbvm_tuple_t offset, sysbvm_tuple_t size)
{
    sysbvm_arraySlice_t *result = (sysbvm_arraySlice_t*)sysbvm_context_allocatePointerTuple(context, context->roots.arraySliceType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_arraySlice_t));
    result->elements = elements;
    result->offset = offset;
    result->size = size;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_arraySlice_createWithOffsetAndSize(sysbvm_context_t *context, sysbvm_tuple_t elements, size_t offset, size_t count)
{
    return sysbvm_arraySlice_create(context, elements, sysbvm_tuple_size_encode(context, offset), sysbvm_tuple_size_encode(context, count));
}

SYSBVM_API sysbvm_tuple_t sysbvm_arraySlice_createWithArrayOfSize(sysbvm_context_t *context, size_t size)
{
    return sysbvm_arraySlice_createWithOffsetAndSize(context, sysbvm_array_create(context, size), 0, size);
}

SYSBVM_API size_t sysbvm_arraySlice_getSize(sysbvm_tuple_t arraySlice)
{
    if(!sysbvm_tuple_isNonNullPointer(arraySlice)) return 0;
    return sysbvm_tuple_size_decode(((sysbvm_arraySlice_t*)arraySlice)->size);
}

SYSBVM_API sysbvm_tuple_t sysbvm_arraySlice_at(sysbvm_tuple_t arraySlice, size_t index)
{
    if(!sysbvm_tuple_isNonNullPointer(arraySlice)) return SYSBVM_NULL_TUPLE;

    sysbvm_arraySlice_t *arraySliceObject = (sysbvm_arraySlice_t*)arraySlice;
    size_t offset = sysbvm_tuple_size_decode(arraySliceObject->offset);
    size_t size = sysbvm_tuple_size_decode(arraySliceObject->size);
    if(index >= size)
        sysbvm_error_indexOutOfBounds();

    return sysbvm_arrayOrByteArray_at(arraySliceObject->elements, offset + index);
}

SYSBVM_API void sysbvm_arraySlice_atPut(sysbvm_tuple_t arraySlice, size_t index, sysbvm_tuple_t value)
{
    if(!sysbvm_tuple_isNonNullPointer(arraySlice))
        return;

    sysbvm_arraySlice_t *arraySliceObject = (sysbvm_arraySlice_t*)arraySlice;
    size_t offset = sysbvm_tuple_size_decode(arraySliceObject->offset);
    size_t size = sysbvm_tuple_size_decode(arraySliceObject->size);
    if(index >= size)
        sysbvm_error_indexOutOfBounds();
 
    sysbvm_arrayOrByteArray_atPut(arraySliceObject->elements, offset + index, value);
}

SYSBVM_API sysbvm_tuple_t sysbvm_arraySlice_fromOffset(sysbvm_context_t *context, sysbvm_tuple_t arraySlice, size_t fromOffset)
{
   if(!sysbvm_tuple_isNonNullPointer(arraySlice)) return SYSBVM_NULL_TUPLE;

    sysbvm_arraySlice_t *arraySliceObject = (sysbvm_arraySlice_t*)arraySlice;
    size_t offset = sysbvm_tuple_size_decode(arraySliceObject->offset);
    size_t size = sysbvm_tuple_size_decode(arraySliceObject->size);

    if(fromOffset >= size)
        return sysbvm_arraySlice_createWithOffsetAndSize(context, SYSBVM_NULL_TUPLE, 0, 0);
    return sysbvm_arraySlice_createWithOffsetAndSize(context, arraySliceObject->elements, offset + fromOffset, size - fromOffset);
}

SYSBVM_API sysbvm_tuple_t sysbvm_arraySlice_asArray(sysbvm_context_t *context, sysbvm_tuple_t arraySlice)
{
   if(!sysbvm_tuple_isNonNullPointer(arraySlice)) return SYSBVM_NULL_TUPLE;

    sysbvm_arraySlice_t *arraySliceObject = (sysbvm_arraySlice_t*)arraySlice;
    size_t offset = sysbvm_tuple_size_decode(arraySliceObject->offset);
    size_t size = sysbvm_tuple_size_decode(arraySliceObject->size);

    sysbvm_tuple_t result = sysbvm_array_create(context, size);
    sysbvm_tuple_t *destination = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(result)->pointers;
    for(size_t i = 0; i < size; ++i)
        destination[i] = sysbvm_arrayOrByteArray_at(arraySliceObject->elements, offset + i);
    return result;
}
