#include "tuuvm/array.h"
#include "tuuvm/arraySlice.h"
#include "tuuvm/errors.h"
#include "internal/context.h"

TUUVM_API tuuvm_tuple_t tuuvm_array_create(tuuvm_context_t *context, tuuvm_tuple_t slotCount)
{
    if(slotCount == 0)
    {
        if(!context->roots.emptyArrayConstant)
            context->roots.emptyArrayConstant = (tuuvm_tuple_t)tuuvm_context_allocatePointerTuple(context, context->roots.arrayType, 0);
        return context->roots.emptyArrayConstant;
    }

    return (tuuvm_tuple_t)tuuvm_context_allocatePointerTuple(context, context->roots.arrayType, slotCount);
}

TUUVM_API tuuvm_tuple_t tuuvm_byteArray_create(tuuvm_context_t *context, tuuvm_tuple_t size)
{
    if(size == 0)
    {
        if(!context->roots.emptyByteArrayConstant)
            context->roots.emptyByteArrayConstant = (tuuvm_tuple_t)tuuvm_context_allocateByteTuple(context, context->roots.byteArrayType, 0);
        return context->roots.emptyByteArrayConstant;
    }

    return (tuuvm_tuple_t)tuuvm_context_allocateByteTuple(context, context->roots.byteArrayType, size);
}

TUUVM_API tuuvm_tuple_t tuuvm_array_asArraySlice(tuuvm_context_t *context, tuuvm_tuple_t array)
{
    if(!tuuvm_tuple_isNonNullPointer(array)) return TUUVM_NULL_TUPLE;

    return tuuvm_arraySlice_createWithOffsetAndSize(context, array, 0, tuuvm_tuple_getSizeInSlots(array));
}

TUUVM_API tuuvm_tuple_t tuuvm_array_getFirstElements(tuuvm_context_t *context, tuuvm_tuple_t array, size_t size)
{
    if(!tuuvm_tuple_isNonNullPointer(array)) return TUUVM_NULL_TUPLE;

    size_t resultSize = tuuvm_tuple_getSizeInSlots(array);
    if(size < resultSize)
        resultSize = size;

    tuuvm_array_t *source = (tuuvm_array_t*)array;
    tuuvm_array_t *result = (tuuvm_array_t*)tuuvm_array_create(context, resultSize);
    for(size_t i = 0; i < resultSize; ++i)
        result->elements[i] = source->elements[i];

    return (tuuvm_tuple_t)result;
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

TUUVM_API tuuvm_tuple_t tuuvm_array_fromOffset(tuuvm_context_t *context, tuuvm_tuple_t array, size_t fromOffset)
{
    if(!tuuvm_tuple_isNonNullPointer(array)) return TUUVM_NULL_TUPLE;
    
    size_t size = tuuvm_tuple_getSizeInSlots(array);
    if(fromOffset >= size)
        return tuuvm_array_create(context, 0);

    size_t resultSize = size - fromOffset;
    tuuvm_tuple_t result = tuuvm_array_create(context, resultSize);
    tuuvm_tuple_t *sourceElements = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(array)->pointers;
    tuuvm_tuple_t *resultElements = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(result)->pointers;
    for(size_t i = 0; i < resultSize; ++i)
        resultElements[i] = sourceElements[fromOffset + i];

    return result;
}