#include "tuuvm/array.h"
#include "tuuvm/errors.h"
#include "internal/context.h"

TUUVM_API tuuvm_tuple_t tuuvm_array_create(tuuvm_context_t *context, tuuvm_tuple_t slotCount)
{
    return (tuuvm_tuple_t)tuuvm_context_allocatePointerTuple(context, context->roots.arrayType, slotCount);
}

TUUVM_API tuuvm_tuple_t tuuvm_byteArray_create(tuuvm_context_t *context, tuuvm_tuple_t size)
{
    return (tuuvm_tuple_t)tuuvm_context_allocateByteTuple(context, context->roots.byteArrayType, size);
}

TUUVM_API tuuvm_tuple_t tuuvm_arrayOrByteArray_at(tuuvm_tuple_t array, size_t index)
{
    if(!tuuvm_tuple_isNonNullPointer(array)) return TUUVM_NULL_TUPLE;

    if(tuuvm_tuple_isBytes(array))
    {
        size_t size = tuuvm_tuple_getSizeInBytes(array);
        if(index >= size) tuuvm_error_indexOutOfBounds();
        return tuuvm_tuple_uint8_encode(TUUVM_CAST_OOP_TO_OBJECT_TUPLE(array)->bytes[index]);
    }
    else
    {
        size_t size = tuuvm_tuple_getSizeInSlots(array);
        if(index >= size) tuuvm_error_indexOutOfBounds();
        return TUUVM_CAST_OOP_TO_OBJECT_TUPLE(array)->pointers[index];
    }
}

TUUVM_API void tuuvm_arrayOrByteArray_atPut(tuuvm_tuple_t array, size_t index, tuuvm_tuple_t value)
{
    if(!tuuvm_tuple_isNonNullPointer(array))
        return;

    if(tuuvm_tuple_isBytes(array))
    {
        size_t size = tuuvm_tuple_getSizeInBytes(array);
        if(index >= size)
            tuuvm_error_indexOutOfBounds();
        TUUVM_CAST_OOP_TO_OBJECT_TUPLE(array)->bytes[index] = tuuvm_tuple_uint8_decode(value);
    }
    else
    {
        size_t size = tuuvm_tuple_getSizeInSlots(array);
        if(index >= size)
            tuuvm_error_indexOutOfBounds();
        TUUVM_CAST_OOP_TO_OBJECT_TUPLE(array)->pointers[index] = value;
    }
}