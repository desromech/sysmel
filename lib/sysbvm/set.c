#include "sysbvm/array.h"
#include "sysbvm/set.h"
#include "sysbvm/errors.h"
#include "sysbvm/function.h"
#include "sysbvm/assert.h"
#include "internal/context.h"
#include <stdlib.h>

SYSBVM_API sysbvm_tuple_t sysbvm_identitySet_create(sysbvm_context_t *context)
{
    sysbvm_identitySet_t *result = (sysbvm_identitySet_t*)sysbvm_context_allocatePointerTuple(context, context->roots.setType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_identitySet_t));
    result->size = sysbvm_tuple_size_encode(context, 0);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API bool sysbvm_identitySet_findWithExplicitHash(sysbvm_tuple_t set, void *element, sysbvm_identitySet_explicitHashFunction_t hashFunction, sysbvm_identitySet_explicitEqualsFunction_t equalsFunction, sysbvm_tuple_t *outFoundElement)
{
    *outFoundElement = SYSBVM_NULL_TUPLE;
    if(!sysbvm_tuple_isNonNullPointer(set))
        return false;

    sysbvm_identitySet_t *setObject = (sysbvm_identitySet_t*)set;
    size_t capacity = sysbvm_tuple_getSizeInSlots(setObject->storage);
    if(capacity == 0)
        return false;

    sysbvm_array_t *storage = (sysbvm_array_t*)setObject->storage;
    size_t hashIndex = hashFunction(element) % capacity;
    for(size_t i = hashIndex; i < capacity; ++i)
    {
        sysbvm_tuple_t setElement = storage->elements[i];
        if(setElement == SYSBVM_HASHTABLE_EMPTY_ELEMENT_TUPLE)
            return false;
        
        if(equalsFunction(element, setElement))
        {
            *outFoundElement = setElement;
            return true;
        }
    }

    for(size_t i = 0; i < hashIndex; ++i)
    {
        sysbvm_tuple_t setElement = storage->elements[i];
        if(setElement == SYSBVM_HASHTABLE_EMPTY_ELEMENT_TUPLE)
            return false;
        
        if(equalsFunction(element, setElement))
        {
            *outFoundElement = setElement;
            return true;
        }
    }

    return false;
}

static intptr_t sysbvm_identitySet_scanFor(sysbvm_tuple_t set, sysbvm_tuple_t element)
{
    if(!sysbvm_tuple_isNonNullPointer(set))
        return -1;

    sysbvm_identitySet_t *setObject = (sysbvm_identitySet_t*)set;
    size_t capacity = sysbvm_tuple_getSizeInSlots(setObject->storage);
    if(capacity == 0)
        return -1;

    sysbvm_array_t *storage = (sysbvm_array_t*)setObject->storage;
    size_t hashIndex = sysbvm_tuple_identityHash(element) % capacity;
    for(size_t i = hashIndex; i < capacity; ++i)
    {
        sysbvm_tuple_t setElement = storage->elements[i];
        if(setElement == SYSBVM_HASHTABLE_EMPTY_ELEMENT_TUPLE || sysbvm_tuple_identityEquals(element, setElement))
            return (intptr_t)i;
    }

    for(size_t i = 0; i < hashIndex; ++i)
    {
        sysbvm_tuple_t setElement = storage->elements[i];
        if(setElement == SYSBVM_HASHTABLE_EMPTY_ELEMENT_TUPLE || sysbvm_tuple_identityEquals(element, setElement))
            return (intptr_t)i;
    }

    return -1;
}

SYSBVM_API bool sysbvm_identitySet_find(sysbvm_tuple_t set, sysbvm_tuple_t element, sysbvm_tuple_t *outFoundElement)
{
    *outFoundElement = SYSBVM_NULL_TUPLE;
    if(!sysbvm_tuple_isNonNullPointer(set))
        return false;

    intptr_t elementIndex = sysbvm_identitySet_scanFor(set, element);
    if(elementIndex < 0)
        return false;

    sysbvm_identitySet_t *setObject = (sysbvm_identitySet_t*)set;
    sysbvm_array_t *storage = (sysbvm_array_t*)setObject->storage;
    if(storage->elements[elementIndex] == SYSBVM_HASHTABLE_EMPTY_ELEMENT_TUPLE)
        return false;

    *outFoundElement = storage->elements[elementIndex];
    return true;
}

static void sysbvm_identitySet_insertNoCheck(sysbvm_tuple_t set, sysbvm_tuple_t element)
{
    intptr_t elementIndex = sysbvm_identitySet_scanFor(set, element);
    SYSBVM_ASSERT(elementIndex >= 0);

    sysbvm_identitySet_t *setObject = (sysbvm_identitySet_t*)set;
    sysbvm_array_t *storage = (sysbvm_array_t*)setObject->storage;
    storage->elements[elementIndex] = element;
}

static void sysbvm_identitySet_increaseCapacity(sysbvm_context_t *context, sysbvm_tuple_t set)
{
    sysbvm_identitySet_t *setObject = (sysbvm_identitySet_t*)set;
    sysbvm_array_t *oldStorage = (sysbvm_array_t*)setObject->storage;

    size_t oldCapacity = sysbvm_tuple_getSizeInSlots(setObject->storage);
    size_t newCapacity = oldCapacity * 2;
    if(newCapacity < 8)
        newCapacity = 8;

    // Make the new storage.
    sysbvm_array_t *newStorage = (sysbvm_array_t*)sysbvm_array_create(context, newCapacity);
    setObject->storage = (sysbvm_tuple_t)newStorage;
    for(size_t i = 0; i < newCapacity; ++i)
        newStorage->elements[i] = SYSBVM_HASHTABLE_EMPTY_ELEMENT_TUPLE;

    // Reinsert the old elements.
    for(size_t i = 0; i < oldCapacity; ++i)
    {
        sysbvm_tuple_t element = oldStorage->elements[i];
        if(element != SYSBVM_HASHTABLE_EMPTY_ELEMENT_TUPLE)
            sysbvm_identitySet_insertNoCheck(set, element);
    }
}

SYSBVM_API void sysbvm_identitySet_insert(sysbvm_context_t *context, sysbvm_tuple_t set, sysbvm_tuple_t element)
{
    if(element == SYSBVM_HASHTABLE_EMPTY_ELEMENT_TUPLE)
        return;

    if(!sysbvm_tuple_isNonNullPointer(set))
        return;

    intptr_t elementIndex = sysbvm_identitySet_scanFor(set, element);
    if(elementIndex < 0)
    {
        sysbvm_identitySet_increaseCapacity(context, set);
        elementIndex = sysbvm_identitySet_scanFor(set, element);
        if(elementIndex < 0)
            sysbvm_error("Set out of memory.");
    }

    sysbvm_identitySet_t *setObject = (sysbvm_identitySet_t*)set;
    sysbvm_array_t *storage = (sysbvm_array_t*)setObject->storage;
    bool isNewElement = storage->elements[elementIndex] == SYSBVM_HASHTABLE_EMPTY_ELEMENT_TUPLE;
    storage->elements[elementIndex] = element;

    // Count the newly inserted element.
    if(isNewElement)
    {
        size_t capacity = sysbvm_tuple_getSizeInSlots(setObject->storage);
        size_t newSize = sysbvm_tuple_size_decode(setObject->size) + 1;
        setObject->size = sysbvm_tuple_size_encode(context, newSize);
        size_t capacityThreshold = capacity * 3 / 4;

        // Make sure the maximum occupancy rate is not greater than 80%.
        if(newSize >= capacityThreshold)
            sysbvm_identitySet_increaseCapacity(context, set);
    }
}
