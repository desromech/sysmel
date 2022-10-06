#include "tuuvm/array.h"
#include "tuuvm/set.h"
#include "tuuvm/function.h"
#include "tuuvm/assert.h"
#include "internal/context.h"
#include <stdlib.h>

TUUVM_API tuuvm_tuple_t tuuvm_set_create(tuuvm_context_t *context, tuuvm_tuple_t equalsFunction, tuuvm_tuple_t hashFunction)
{
    tuuvm_set_t *result = (tuuvm_set_t*)tuuvm_context_allocatePointerTuple(context, context->roots.setType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_set_t));
    result->size = tuuvm_tuple_size_encode(context, 0);
    result->equalsFunction = equalsFunction;
    result->hashFunction = hashFunction;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_set_findOrNilWithExplicitHash(tuuvm_tuple_t set, void *element, tuuvm_set_explicitHashFunction_t hashFunction, tuuvm_set_explicitEqualsFunction_t equalsFunction)
{
    if(!tuuvm_tuple_isNonNullPointer(set))
        return TUUVM_NULL_TUPLE;

    tuuvm_set_t *setObject = (tuuvm_set_t*)set;
    size_t capacity = tuuvm_tuple_getSizeInSlots(setObject->storage);
    if(capacity == 0)
        return TUUVM_NULL_TUPLE;

    tuuvm_array_t *storage = (tuuvm_array_t*)setObject->storage;
    size_t hashIndex = hashFunction(element) % capacity;
    for(size_t i = hashIndex; i < capacity; ++i)
    {
        tuuvm_tuple_t setElement = storage->elements[i];
        if(setElement == TUUVM_NULL_TUPLE)
            return TUUVM_NULL_TUPLE;
        
        if(equalsFunction(element, setElement))
            return setElement;
    }

    for(size_t i = 0; i < hashIndex; ++i)
    {
        tuuvm_tuple_t setElement = storage->elements[i];
        if(setElement == TUUVM_NULL_TUPLE)
            return TUUVM_NULL_TUPLE;
        
        if(equalsFunction(element, setElement))
            return setElement;
    }

    return TUUVM_NULL_TUPLE;
}

static intptr_t tuuvm_set_scanFor(tuuvm_context_t *context, tuuvm_tuple_t set, tuuvm_tuple_t element)
{
    if(!tuuvm_tuple_isNonNullPointer(set))
        return -1;

    tuuvm_set_t *setObject = (tuuvm_set_t*)set;
    size_t capacity = tuuvm_tuple_getSizeInSlots(setObject->storage);
    if(capacity == 0)
        return -1;

    tuuvm_array_t *storage = (tuuvm_array_t*)setObject->storage;
    size_t hashIndex = tuuvm_function_apply1(context, setObject->hashFunction, element) % capacity;
    for(size_t i = hashIndex; i < capacity; ++i)
    {
        tuuvm_tuple_t setElement = storage->elements[i];
        if(setElement == TUUVM_NULL_TUPLE || tuuvm_tuple_boolean_decode(tuuvm_function_apply2(context, setObject->equalsFunction, element, setElement)))
            return (intptr_t)i;
    }

    for(size_t i = 0; i < hashIndex; ++i)
    {
        tuuvm_tuple_t setElement = storage->elements[i];
        if(setElement == TUUVM_NULL_TUPLE || tuuvm_tuple_boolean_decode(tuuvm_function_apply2(context, setObject->equalsFunction, element, setElement)))
            return setElement;
    }

    return -1;
}

TUUVM_API tuuvm_tuple_t tuuvm_set_findOrNil(tuuvm_context_t *context, tuuvm_tuple_t set, tuuvm_tuple_t element)
{
    if(!tuuvm_tuple_isNonNullPointer(set))
        return TUUVM_NULL_TUPLE;

    intptr_t elementIndex = tuuvm_set_scanFor(context, set, element);
    if(elementIndex < 0)
        return TUUVM_NULL_TUPLE;

    tuuvm_set_t *setObject = (tuuvm_set_t*)set;
    tuuvm_array_t *storage = (tuuvm_array_t*)setObject->storage;
    return storage->elements[elementIndex];
}

static void tuuvm_set_insertNoCheck(tuuvm_context_t *context, tuuvm_tuple_t set, tuuvm_tuple_t element)
{
    intptr_t elementIndex = tuuvm_set_scanFor(context, set, element);
    TUUVM_ASSERT(elementIndex >= 0);

    tuuvm_set_t *setObject = (tuuvm_set_t*)set;
    tuuvm_array_t *storage = (tuuvm_array_t*)setObject->storage;
    storage->elements[elementIndex] = element;
}

static void tuuvm_set_increaseCapacity(tuuvm_context_t *context, tuuvm_tuple_t set)
{
    tuuvm_set_t *setObject = (tuuvm_set_t*)set;
    tuuvm_array_t *oldStorage = (tuuvm_array_t*)setObject->storage;

    size_t oldCapacity = tuuvm_tuple_getSizeInSlots(setObject->storage);
    size_t newCapacity = oldCapacity * 2;
    if(newCapacity < 8)
        newCapacity = 8;

    // Make the new storage.
    tuuvm_array_t *newStorage = (tuuvm_array_t*)tuuvm_array_create(context, newCapacity);
    setObject->storage = (tuuvm_tuple_t)newStorage;

    // Reinsert the old elements.
    for(size_t i = 0; i < oldCapacity; ++i)
    {
        tuuvm_tuple_t element = oldStorage->elements[i];
        if(element != TUUVM_NULL_TUPLE)
            tuuvm_set_insertNoCheck(context, set, element);
    }
}

TUUVM_API void tuuvm_set_insert(tuuvm_context_t *context, tuuvm_tuple_t set, tuuvm_tuple_t element)
{
    if(element == TUUVM_NULL_TUPLE)
        return;

    if(!tuuvm_tuple_isNonNullPointer(set))
        return;

    intptr_t elementIndex = tuuvm_set_scanFor(context, set, element);
    if(elementIndex < 0)
    {
        tuuvm_set_increaseCapacity(context, set);
        elementIndex = tuuvm_set_scanFor(context, set, element);
        if(elementIndex < 0)
            abort();
    }

    tuuvm_set_t *setObject = (tuuvm_set_t*)set;
    tuuvm_array_t *storage = (tuuvm_array_t*)setObject->storage;
    bool isNewElement = storage->elements[elementIndex] == TUUVM_NULL_TUPLE;
    storage->elements[elementIndex] = element;

    // Count the newly inserted element.
    if(isNewElement)
    {
        size_t capacity = tuuvm_tuple_getSizeInSlots(setObject->storage);
        size_t newSize = tuuvm_tuple_size_decode(setObject->size) + 1;
        setObject->size = tuuvm_tuple_size_encode(context, newSize);

        // Make sure the maximum occupancy rate is not greater than 80%.
        if(newSize >= capacity * 3 / 4)
            tuuvm_set_increaseCapacity(context, set);
    }

}