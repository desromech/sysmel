#include "tuuvm/array.h"
#include "tuuvm/dictionary.h"
#include "tuuvm/function.h"
#include "tuuvm/assert.h"
#include "internal/context.h"
#include <stdlib.h>

TUUVM_API tuuvm_tuple_t tuuvm_dictionary_create(tuuvm_context_t *context, tuuvm_tuple_t equalsFunction, tuuvm_tuple_t hashFunction)
{
    tuuvm_dictionary_t *result = (tuuvm_dictionary_t*)tuuvm_context_allocatePointerTuple(context, context->roots.dictionaryType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_dictionary_t));
    result->size = tuuvm_tuple_size_encode(context, 0);
    result->equalsFunction = equalsFunction;
    result->hashFunction = hashFunction;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_identityDictionary_create(tuuvm_context_t *context)
{
    return tuuvm_dictionary_create(context, context->roots.identityEqualsFunction, context->roots.identityHashFunction);
}

static intptr_t tuuvm_dictionary_scanFor(tuuvm_context_t *context, tuuvm_tuple_t dictionary, tuuvm_tuple_t element)
{
    if(!tuuvm_tuple_isNonNullPointer(dictionary))
        return -1;

    tuuvm_dictionary_t *dictionaryObject = (tuuvm_dictionary_t*)dictionary;
    size_t capacity = tuuvm_tuple_getSizeInSlots(dictionaryObject->storage) / 2;
    if(capacity == 0)
        return -1;

    tuuvm_array_t *storage = (tuuvm_array_t*)dictionaryObject->storage;
    size_t hashIndex = tuuvm_tuple_size_decode(tuuvm_function_apply1(context, dictionaryObject->hashFunction, element)) % capacity;
    for(size_t i = hashIndex; i < capacity; ++i)
    {
        tuuvm_tuple_t dictionaryKey = storage->elements[i*2];
        if(dictionaryKey == TUUVM_HASHTABLE_EMPTY_ELEMENT_TUPLE || tuuvm_tuple_boolean_decode(tuuvm_function_apply2(context, dictionaryObject->equalsFunction, element, dictionaryKey)))
            return (intptr_t)i;
    }

    for(size_t i = 0; i < hashIndex; ++i)
    {
        tuuvm_tuple_t dictionaryKey = storage->elements[i*2];
        if(dictionaryKey == TUUVM_HASHTABLE_EMPTY_ELEMENT_TUPLE || tuuvm_tuple_boolean_decode(tuuvm_function_apply2(context, dictionaryObject->equalsFunction, element, dictionaryKey)))
            return (intptr_t)i;
    }

    return -1;
}

TUUVM_API bool tuuvm_dictionary_find(tuuvm_context_t *context, tuuvm_tuple_t dictionary, tuuvm_tuple_t element, tuuvm_tuple_t *outValue)
{
    *outValue = TUUVM_NULL_TUPLE;
    if(!tuuvm_tuple_isNonNullPointer(dictionary))
        return false;

    intptr_t elementIndex = tuuvm_dictionary_scanFor(context, dictionary, element);
    if(elementIndex < 0)
        return false;

    tuuvm_dictionary_t *dictionaryObject = (tuuvm_dictionary_t*)dictionary;
    tuuvm_array_t *storage = (tuuvm_array_t*)dictionaryObject->storage;
    *outValue = storage->elements[elementIndex*2 + 1];
    return true;
}

static void tuuvm_dictionary_insertNoCheck(tuuvm_context_t *context, tuuvm_tuple_t dictionary, tuuvm_tuple_t key, tuuvm_tuple_t value)
{
    intptr_t elementIndex = tuuvm_dictionary_scanFor(context, dictionary, key);
    TUUVM_ASSERT(elementIndex >= 0);

    tuuvm_dictionary_t *dictionaryObject = (tuuvm_dictionary_t*)dictionary;
    tuuvm_array_t *storage = (tuuvm_array_t*)dictionaryObject->storage;
    storage->elements[elementIndex*2] = key;
    storage->elements[elementIndex*2 + 1] = value;
}

static void tuuvm_dictionary_increaseCapacity(tuuvm_context_t *context, tuuvm_tuple_t dictionary)
{
    tuuvm_dictionary_t *dictionaryObject = (tuuvm_dictionary_t*)dictionary;
    tuuvm_array_t *oldStorage = (tuuvm_array_t*)dictionaryObject->storage;

    size_t oldCapacity = tuuvm_tuple_getSizeInSlots(dictionaryObject->storage) / 2;
    size_t newCapacity = oldCapacity * 2;
    if(newCapacity < 8)
        newCapacity = 8;

    // Make the new storage.
    tuuvm_array_t *newStorage = (tuuvm_array_t*)tuuvm_array_create(context, newCapacity*2);
    dictionaryObject->storage = (tuuvm_tuple_t)newStorage;
    for(size_t i = 0; i < newCapacity; ++i)
        newStorage->elements[i*2] = TUUVM_HASHTABLE_EMPTY_ELEMENT_TUPLE;

    // Reinsert the old elements.
    for(size_t i = 0; i < oldCapacity; ++i)
    {
        tuuvm_tuple_t key = oldStorage->elements[i*2];
        tuuvm_tuple_t value = oldStorage->elements[i*2 + 1];
        if(key != TUUVM_HASHTABLE_EMPTY_ELEMENT_TUPLE)
            tuuvm_dictionary_insertNoCheck(context, dictionary, key, value);
    }
}

TUUVM_API void tuuvm_dictionary_atPut(tuuvm_context_t *context, tuuvm_tuple_t dictionary, tuuvm_tuple_t key, tuuvm_tuple_t value)
{
    if(key == TUUVM_HASHTABLE_EMPTY_ELEMENT_TUPLE)
        return;

    if(!tuuvm_tuple_isNonNullPointer(dictionary))
        return;

    intptr_t elementIndex = tuuvm_dictionary_scanFor(context, dictionary, key);
    if(elementIndex < 0)
    {
        tuuvm_dictionary_increaseCapacity(context, dictionary);
        elementIndex = tuuvm_dictionary_scanFor(context, dictionary, key);
        if(elementIndex < 0)
            abort();
    }

    tuuvm_dictionary_t *dictionaryObject = (tuuvm_dictionary_t*)dictionary;
    tuuvm_array_t *storage = (tuuvm_array_t*)dictionaryObject->storage;
    bool isNewElement = storage->elements[elementIndex*2] == TUUVM_HASHTABLE_EMPTY_ELEMENT_TUPLE;
    storage->elements[elementIndex*2] = key;
    storage->elements[elementIndex*2 + 1] = value;

    // Count the newly inserted element.
    if(isNewElement)
    {
        size_t capacity = tuuvm_tuple_getSizeInSlots(dictionaryObject->storage) / 2;
        size_t newSize = tuuvm_tuple_size_decode(dictionaryObject->size) + 1;
        dictionaryObject->size = tuuvm_tuple_size_encode(context, newSize);
        size_t capacityThreshold = capacity * 3 / 4;

        // Make sure the maximum occupancy rate is not greater than 80%.
        if(newSize >= capacityThreshold)
            tuuvm_dictionary_increaseCapacity(context, dictionary);
    }

}
