#include "tuuvm/arrayList.h"
#include "tuuvm/array.h"
#include "tuuvm/arraySlice.h"
#include "internal/context.h"

TUUVM_API tuuvm_tuple_t tuuvm_arrayList_create(tuuvm_context_t *context)
{
    tuuvm_arrayList_t *result = (tuuvm_arrayList_t*)tuuvm_context_allocatePointerTuple(context, context->roots.arrayListType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_arrayList_t));
    result->size = tuuvm_tuple_size_encode(context, 0);
    return (tuuvm_tuple_t)result;

}

static void tuuvm_arrayList_increaseCapacity(tuuvm_context_t *context, tuuvm_tuple_t arrayList)
{
    tuuvm_arrayList_t *arrayListObject = (tuuvm_arrayList_t*)arrayList;
    size_t size = tuuvm_tuple_size_decode(arrayListObject->size);
    size_t oldCapacity = tuuvm_tuple_getSizeInSlots(arrayListObject->storage);
    size_t newCapacity = oldCapacity * 2;
    if(newCapacity < 16)
        newCapacity = 16;

    tuuvm_array_t *newStorage = (tuuvm_array_t*)tuuvm_array_create(context, newCapacity);
    tuuvm_array_t *oldStorage = (tuuvm_array_t*)arrayListObject->storage;
    for(size_t i = 0; i < size; ++i)
        newStorage->elements[i] = oldStorage->elements[i];
    arrayListObject->storage = (tuuvm_tuple_t)newStorage;
}

TUUVM_API void tuuvm_arrayList_add(tuuvm_context_t *context, tuuvm_tuple_t arrayList, tuuvm_tuple_t element)
{
    if(!tuuvm_tuple_isNonNullPointer(arrayList))
        return;

    tuuvm_arrayList_t *arrayListObject = (tuuvm_arrayList_t*)arrayList;
    size_t size = tuuvm_tuple_size_decode(arrayListObject->size);
    size_t capacity = tuuvm_tuple_getSizeInSlots(arrayListObject->storage);
    if(size >= capacity)
        tuuvm_arrayList_increaseCapacity(context, arrayList);

    tuuvm_array_t *storage = (tuuvm_array_t*)arrayListObject->storage;
    storage->elements[size] = element;
    arrayListObject->size = tuuvm_tuple_size_encode(context, size + 1);
}

TUUVM_API tuuvm_tuple_t tuuvm_arrayList_asArraySlice(tuuvm_context_t *context, tuuvm_tuple_t arrayList)
{
    if(!tuuvm_tuple_isNonNullPointer(arrayList))
        return TUUVM_NULL_TUPLE;

    tuuvm_arrayList_t *arrayListObject = (tuuvm_arrayList_t*)arrayList;
    return tuuvm_arraySlice_create(context, arrayListObject->storage, tuuvm_tuple_size_encode(context, 0), arrayListObject->size);
}

TUUVM_API size_t tuuvm_arrayList_getSize(tuuvm_tuple_t arrayList)
{
    if(!tuuvm_tuple_isNonNullPointer(arrayList)) return 0;
    return tuuvm_tuple_size_decode(((tuuvm_arrayList_t*)arrayList)->size);
}

TUUVM_API size_t tuuvm_arrayList_getCapacity(tuuvm_tuple_t arrayList)
{
    if(!tuuvm_tuple_isNonNullPointer(arrayList)) return 0;
    return tuuvm_tuple_getSizeInSlots(((tuuvm_arrayList_t*)arrayList)->storage);
}

/**
 * Gets the array list element at the specified index.
 */
TUUVM_API tuuvm_tuple_t tuuvm_arrayList_at(tuuvm_tuple_t arrayList, size_t index)
{
    if(!tuuvm_tuple_isNonNullPointer(arrayList)) return TUUVM_NULL_TUPLE;

    tuuvm_arrayList_t *arrayListObject = (tuuvm_arrayList_t*)arrayList;
    size_t size = tuuvm_tuple_size_decode(arrayListObject->size);
    if(index >= size) return TUUVM_NULL_TUPLE; // FIXME: Raise out of bounds error.

    return ((tuuvm_array_t*)arrayListObject->storage)->elements[index];
}
