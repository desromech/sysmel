#include "tuuvm/array.h"
#include "tuuvm/assert.h"
#include "tuuvm/association.h"
#include "tuuvm/dictionary.h"
#include "tuuvm/errors.h"
#include "tuuvm/function.h"
#include "internal/context.h"
#include <stdlib.h>

TUUVM_API tuuvm_tuple_t tuuvm_identityDictionary_create(tuuvm_context_t *context)
{
    tuuvm_identityDictionary_t *result = (tuuvm_identityDictionary_t*)tuuvm_context_allocatePointerTuple(context, context->roots.identityDictionaryType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_identityDictionary_t));
    result->size = tuuvm_tuple_size_encode(context, 0);
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_methodDictionary_create(tuuvm_context_t *context)
{
    tuuvm_methodDictionary_t *result = (tuuvm_methodDictionary_t*)tuuvm_context_allocatePointerTuple(context, context->roots.methodDictionaryType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_methodDictionary_t));
    result->size = tuuvm_tuple_size_encode(context, 0);
    return (tuuvm_tuple_t)result;
}

static intptr_t tuuvm_identityDictionary_scanFor(tuuvm_tuple_t dictionary, tuuvm_tuple_t element)
{
    if(!tuuvm_tuple_isNonNullPointer(dictionary))
        return -1;

    tuuvm_dictionary_t *dictionaryObject = (tuuvm_dictionary_t*)dictionary;
    size_t capacity = tuuvm_tuple_getSizeInSlots(dictionaryObject->storage);
    if(capacity == 0)
        return -1;

    tuuvm_array_t *storage = (tuuvm_array_t*)dictionaryObject->storage;
    size_t hashIndex = tuuvm_tuple_identityHash(element) % capacity;
    for(size_t i = hashIndex; i < capacity; ++i)
    {
        tuuvm_tuple_t association = storage->elements[i];
        if(!association || tuuvm_tuple_identityEquals(element, tuuvm_association_getKey(association)))
            return (intptr_t)i;
    }

    for(size_t i = 0; i < hashIndex; ++i)
    {
        tuuvm_tuple_t association = storage->elements[i];
        if(!association || tuuvm_tuple_identityEquals(element, tuuvm_association_getKey(association)))
            return (intptr_t)i;
    }

    return -1;
}

TUUVM_API bool tuuvm_identityDictionary_findAssociation(tuuvm_tuple_t dictionary, tuuvm_tuple_t key, tuuvm_tuple_t *outAssociation)
{
    *outAssociation = TUUVM_NULL_TUPLE;
    if(!tuuvm_tuple_isNonNullPointer(dictionary))
        return false;

    intptr_t elementIndex = tuuvm_identityDictionary_scanFor(dictionary, key);
    if(elementIndex < 0)
        return false;

    tuuvm_dictionary_t *dictionaryObject = (tuuvm_dictionary_t*)dictionary;
    tuuvm_array_t *storage = (tuuvm_array_t*)dictionaryObject->storage;
    if(!storage->elements[elementIndex])
        return false;

    *outAssociation = storage->elements[elementIndex];
    return true;
}

TUUVM_API bool tuuvm_identityDictionary_find(tuuvm_tuple_t dictionary, tuuvm_tuple_t element, tuuvm_tuple_t *outValue)
{
    *outValue = TUUVM_NULL_TUPLE;
    if(!tuuvm_tuple_isNonNullPointer(dictionary))
        return false;

    intptr_t elementIndex = tuuvm_identityDictionary_scanFor(dictionary, element);
    if(elementIndex < 0)
        return false;

    tuuvm_dictionary_t *dictionaryObject = (tuuvm_dictionary_t*)dictionary;
    tuuvm_array_t *storage = (tuuvm_array_t*)dictionaryObject->storage;
    if(!storage->elements[elementIndex])
        return false;

    *outValue = tuuvm_association_getValue(storage->elements[elementIndex]);
    return true;
}

static void tuuvm_identityDictionary_insertNoCheck(tuuvm_tuple_t dictionary, tuuvm_tuple_t association)
{
    intptr_t elementIndex = tuuvm_identityDictionary_scanFor(dictionary, tuuvm_association_getKey(association));
    TUUVM_ASSERT(elementIndex >= 0);

    tuuvm_dictionary_t *dictionaryObject = (tuuvm_dictionary_t*)dictionary;
    tuuvm_array_t *storage = (tuuvm_array_t*)dictionaryObject->storage;
    storage->elements[elementIndex] = association;
}

static void tuuvm_identityDictionary_increaseCapacity(tuuvm_context_t *context, tuuvm_tuple_t dictionary)
{
    tuuvm_dictionary_t *dictionaryObject = (tuuvm_dictionary_t*)dictionary;
    tuuvm_array_t *oldStorage = (tuuvm_array_t*)dictionaryObject->storage;

    size_t oldCapacity = tuuvm_tuple_getSizeInSlots(dictionaryObject->storage);
    size_t newCapacity = oldCapacity * 2;
    if(newCapacity < 8)
        newCapacity = 8;

    // Make the new storage.
    tuuvm_array_t *newStorage = (tuuvm_array_t*)tuuvm_array_create(context, newCapacity);
    dictionaryObject->storage = (tuuvm_tuple_t)newStorage;

    // Reinsert the old elements.
    for(size_t i = 0; i < oldCapacity; ++i)
    {
        tuuvm_tuple_t association = oldStorage->elements[i];
        if(association)
            tuuvm_identityDictionary_insertNoCheck(dictionary, association);
    }
}

TUUVM_API void tuuvm_identityDictionary_addAssociation(tuuvm_context_t *context, tuuvm_tuple_t dictionary, tuuvm_tuple_t association)
{
    if(!tuuvm_tuple_isNonNullPointer(dictionary))
        return;

    tuuvm_tuple_t key = tuuvm_association_getKey(association);
    intptr_t elementIndex = tuuvm_identityDictionary_scanFor(dictionary, key);
    if(elementIndex < 0)
    {
        tuuvm_identityDictionary_increaseCapacity(context, dictionary);
        elementIndex = tuuvm_identityDictionary_scanFor(dictionary, key);
        if(elementIndex < 0)
           tuuvm_error("Dictionary out of memory.");
    }

    tuuvm_dictionary_t *dictionaryObject = (tuuvm_dictionary_t*)dictionary;
    tuuvm_array_t *storage = (tuuvm_array_t*)dictionaryObject->storage;
    if(storage->elements[elementIndex])
    {
        storage->elements[elementIndex] = association;
    }
    else
    {
        storage->elements[elementIndex] = association;
        size_t capacity = tuuvm_tuple_getSizeInSlots(dictionaryObject->storage);
        size_t newSize = tuuvm_tuple_size_decode(dictionaryObject->size) + 1;
        dictionaryObject->size = tuuvm_tuple_size_encode(context, newSize);
        size_t capacityThreshold = capacity * 4 / 5;

        // Make sure the maximum occupancy rate is not greater than 80%.
        if(newSize >= capacityThreshold)
            tuuvm_identityDictionary_increaseCapacity(context, dictionary);
    }
}

TUUVM_API void tuuvm_identityDictionary_atPut(tuuvm_context_t *context, tuuvm_tuple_t dictionary, tuuvm_tuple_t key, tuuvm_tuple_t value)
{
    if(!tuuvm_tuple_isNonNullPointer(dictionary))
        return;

    intptr_t elementIndex = tuuvm_identityDictionary_scanFor(dictionary, key);
    if(elementIndex < 0)
    {
        tuuvm_identityDictionary_increaseCapacity(context, dictionary);
        elementIndex = tuuvm_identityDictionary_scanFor(dictionary, key);
        if(elementIndex < 0)
           tuuvm_error("Dictionary out of memory.");
    }

    tuuvm_dictionary_t *dictionaryObject = (tuuvm_dictionary_t*)dictionary;
    tuuvm_array_t *storage = (tuuvm_array_t*)dictionaryObject->storage;
    if(storage->elements[elementIndex])
    {
        tuuvm_association_setValue(storage->elements[elementIndex], value);
    }
    else
    {
        storage->elements[elementIndex] = tuuvm_association_create(context, key, value);
        size_t capacity = tuuvm_tuple_getSizeInSlots(dictionaryObject->storage);
        size_t newSize = tuuvm_tuple_size_decode(dictionaryObject->size) + 1;
        dictionaryObject->size = tuuvm_tuple_size_encode(context, newSize);
        size_t capacityThreshold = capacity * 4 / 5;

        // Make sure the maximum occupancy rate is not greater than 80%.
        if(newSize >= capacityThreshold)
            tuuvm_identityDictionary_increaseCapacity(context, dictionary);
    }
}

static intptr_t tuuvm_methodDictionary_scanFor(tuuvm_tuple_t dictionary, tuuvm_tuple_t element)
{
    if(!tuuvm_tuple_isNonNullPointer(dictionary))
        return -1;

    tuuvm_dictionary_t *dictionaryObject = (tuuvm_dictionary_t*)dictionary;
    size_t capacity = tuuvm_tuple_getSizeInSlots(dictionaryObject->storage) / 2;
    if(capacity == 0)
        return -1;

    tuuvm_array_t *storage = (tuuvm_array_t*)dictionaryObject->storage;
    size_t hashIndex = tuuvm_tuple_identityHash(element) % capacity;
    for(size_t i = hashIndex; i < capacity; ++i)
    {
        tuuvm_tuple_t dictionaryKey = storage->elements[i*2];
        if(dictionaryKey == TUUVM_HASHTABLE_EMPTY_ELEMENT_TUPLE || tuuvm_tuple_identityEquals(element, dictionaryKey))
            return (intptr_t)i;
    }

    for(size_t i = 0; i < hashIndex; ++i)
    {
        tuuvm_tuple_t dictionaryKey = storage->elements[i*2];
        if(dictionaryKey == TUUVM_HASHTABLE_EMPTY_ELEMENT_TUPLE || tuuvm_tuple_identityEquals(element, dictionaryKey))
            return (intptr_t)i;
    }

    return -1;
}

TUUVM_API bool tuuvm_methodDictionary_find(tuuvm_tuple_t dictionary, tuuvm_tuple_t element, tuuvm_tuple_t *outValue)
{
    *outValue = TUUVM_NULL_TUPLE;
    if(!tuuvm_tuple_isNonNullPointer(dictionary))
        return false;

    intptr_t elementIndex = tuuvm_methodDictionary_scanFor(dictionary, element);
    if(elementIndex < 0)
        return false;

    tuuvm_dictionary_t *dictionaryObject = (tuuvm_dictionary_t*)dictionary;
    tuuvm_array_t *storage = (tuuvm_array_t*)dictionaryObject->storage;
    if(storage->elements[elementIndex*2] == TUUVM_HASHTABLE_EMPTY_ELEMENT_TUPLE)
        return false;

    *outValue = storage->elements[elementIndex*2 + 1];
    return true;
}

static void tuuvm_methodDictionary_insertNoCheck(tuuvm_tuple_t dictionary, tuuvm_tuple_t key, tuuvm_tuple_t value)
{
    intptr_t elementIndex = tuuvm_methodDictionary_scanFor(dictionary, key);
    TUUVM_ASSERT(elementIndex >= 0);

    tuuvm_dictionary_t *dictionaryObject = (tuuvm_dictionary_t*)dictionary;
    tuuvm_array_t *storage = (tuuvm_array_t*)dictionaryObject->storage;
    storage->elements[elementIndex*2] = key;
    storage->elements[elementIndex*2 + 1] = value;
}

static void tuuvm_methodDictionary_increaseCapacity(tuuvm_context_t *context, tuuvm_tuple_t dictionary)
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
            tuuvm_methodDictionary_insertNoCheck(dictionary, key, value);
    }
}

TUUVM_API void tuuvm_methodDictionary_atPut(tuuvm_context_t *context, tuuvm_tuple_t dictionary, tuuvm_tuple_t key, tuuvm_tuple_t value)
{
    if(key == TUUVM_HASHTABLE_EMPTY_ELEMENT_TUPLE)
        return;

    if(!tuuvm_tuple_isNonNullPointer(dictionary))
        return;

    intptr_t elementIndex = tuuvm_methodDictionary_scanFor(dictionary, key);
    if(elementIndex < 0)
    {
        tuuvm_methodDictionary_increaseCapacity(context, dictionary);
        elementIndex = tuuvm_methodDictionary_scanFor(dictionary, key);
        if(elementIndex < 0)
           tuuvm_error("Dictionary out of memory.");
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
            tuuvm_methodDictionary_increaseCapacity(context, dictionary);
    }
}

static tuuvm_tuple_t tuuvm_identityDictionary_primitive_atOrNil(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t found = TUUVM_NULL_TUPLE;
    if(!tuuvm_identityDictionary_find(arguments[0], arguments[1], &found))
        found = TUUVM_NULL_TUPLE;

    return found;
}

static tuuvm_tuple_t tuuvm_identityDictionary_primitive_at(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t found = TUUVM_NULL_TUPLE;
    if(!tuuvm_identityDictionary_find(arguments[0], arguments[1], &found))
        tuuvm_error("Failed to find the expected key in the dictionary.");

    return found;
}

static tuuvm_tuple_t tuuvm_identityDictionary_primitive_atPut(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 3) tuuvm_error_argumentCountMismatch(3, argumentCount);

    tuuvm_identityDictionary_atPut(context, arguments[0], arguments[1], arguments[2]);
    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_methodDictionary_primitive_new(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    (void)arguments;
    if(argumentCount != 0) tuuvm_error_argumentCountMismatch(0, argumentCount);

    return tuuvm_methodDictionary_create(context);
}

static tuuvm_tuple_t tuuvm_methodDictionary_primitive_atOrNil(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t found = TUUVM_NULL_TUPLE;
    if(!tuuvm_methodDictionary_find(arguments[0], arguments[1], &found))
        found = TUUVM_NULL_TUPLE;

    return found;
}

static tuuvm_tuple_t tuuvm_methodDictionary_primitive_at(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t found = TUUVM_NULL_TUPLE;
    if(!tuuvm_methodDictionary_find(arguments[0], arguments[1], &found))
        tuuvm_error("Failed to find the expected key in the dictionary.");

    return found;
}

static tuuvm_tuple_t tuuvm_methodDictionary_primitive_atPut(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 3) tuuvm_error_argumentCountMismatch(3, argumentCount);

    tuuvm_methodDictionary_atPut(context, arguments[0], arguments[1], arguments[2]);
    return TUUVM_VOID_TUPLE;
}

void tuuvm_dictionary_registerPrimitives(void)
{
    tuuvm_primitiveTable_registerFunction(tuuvm_identityDictionary_primitive_atOrNil);
    tuuvm_primitiveTable_registerFunction(tuuvm_identityDictionary_primitive_atPut);
    tuuvm_primitiveTable_registerFunction(tuuvm_identityDictionary_primitive_at);

    tuuvm_primitiveTable_registerFunction(tuuvm_methodDictionary_primitive_new);
    tuuvm_primitiveTable_registerFunction(tuuvm_methodDictionary_primitive_atOrNil);
    tuuvm_primitiveTable_registerFunction(tuuvm_methodDictionary_primitive_at);
    tuuvm_primitiveTable_registerFunction(tuuvm_methodDictionary_primitive_atPut);
}

void tuuvm_dictionary_setupPrimitives(tuuvm_context_t *context)
{
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "IdentityDictionary::atOrNil:", context->roots.identityDictionaryType, "atOrNil:", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_identityDictionary_primitive_atOrNil);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "IdentityDictionary::at:put:", context->roots.identityDictionaryType, "at:put:", 3, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_identityDictionary_primitive_atPut);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "IdentityDictionary::at:", context->roots.identityDictionaryType, "at:", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_identityDictionary_primitive_at);

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "MethodDictionary::new", 0, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_methodDictionary_primitive_new);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "MethodDictionary::atOrNil:", context->roots.methodDictionaryType, "atOrNil:", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_methodDictionary_primitive_atOrNil);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "MethodDictionary::at:", context->roots.methodDictionaryType, "at:put:", 3, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_methodDictionary_primitive_at);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "MethodDictionary::at:put:", context->roots.methodDictionaryType, "at:put:", 3, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_methodDictionary_primitive_atPut);
}
