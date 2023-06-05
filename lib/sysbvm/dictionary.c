#include "sysbvm/array.h"
#include "sysbvm/assert.h"
#include "sysbvm/association.h"
#include "sysbvm/dictionary.h"
#include "sysbvm/errors.h"
#include "sysbvm/function.h"
#include "sysbvm/stackFrame.h"
#include "internal/context.h"
#include <stdlib.h>

SYSBVM_API sysbvm_tuple_t sysbvm_dictionary_create(sysbvm_context_t *context)
{
    sysbvm_dictionary_t *result = (sysbvm_dictionary_t*)sysbvm_context_allocatePointerTuple(context, context->roots.dictionaryType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_dictionary_t));
    result->size = sysbvm_tuple_size_encode(context, 0);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_dictionary_createWithCapacity(sysbvm_context_t *context, size_t expectedCapacity)
{
    size_t requiredStorageCapacity = expectedCapacity * 130 / 100;

    sysbvm_dictionary_t *result = (sysbvm_dictionary_t*)sysbvm_context_allocatePointerTuple(context, context->roots.dictionaryType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_dictionary_t));
    result->size = sysbvm_tuple_size_encode(context, 0);
    if(requiredStorageCapacity > 0)
        result->storage = sysbvm_array_create(context, requiredStorageCapacity);
    return (sysbvm_tuple_t)result;
}

static intptr_t sysbvm_dictionary_scanFor(sysbvm_context_t *context, sysbvm_dictionary_t **dictionary, sysbvm_tuple_t *element)
{
    struct {
        sysbvm_array_t *storage;
        sysbvm_association_t *association;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    size_t capacity = sysbvm_tuple_getSizeInSlots((*dictionary)->storage);
    if(capacity == 0)
    {
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return -1;
    }

    gcFrame.storage = (sysbvm_array_t*)(*dictionary)->storage;
    size_t hashIndex = sysbvm_tuple_hash(context, *element) % capacity;
    for(size_t i = hashIndex; i < capacity; ++i)
    {
        gcFrame.association = (sysbvm_association_t*)gcFrame.storage->elements[i];
        if(!gcFrame.association ||
            sysbvm_tuple_equals(context, *element, gcFrame.association->key))
        {
            SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
            return (intptr_t)i;
        }
    }

    for(size_t i = 0; i < hashIndex; ++i)
    {
        gcFrame.association = (sysbvm_association_t*)gcFrame.storage->elements[i];
        if(!gcFrame.association ||
            sysbvm_tuple_equals(context, *element, gcFrame.association->key))
        {
            SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
            return (intptr_t)i;
        }
    }

    return -1;
}

static void sysbvm_dictionary_insertNoCheck(sysbvm_context_t *context, sysbvm_dictionary_t **dictionary, sysbvm_weakValueAssociation_t **association, sysbvm_tuple_t *key)
{
    intptr_t elementIndex = sysbvm_dictionary_scanFor(context, dictionary, key);
    SYSBVM_ASSERT(elementIndex >= 0);

    sysbvm_array_t *storage = (sysbvm_array_t*)(*dictionary)->storage;
    storage->elements[elementIndex] = (sysbvm_tuple_t)*association;
}

SYSBVM_API bool sysbvm_dictionary_find(sysbvm_context_t *context, sysbvm_tuple_t dictionary, sysbvm_tuple_t key, sysbvm_tuple_t *outValue)
{
    if(!sysbvm_tuple_isNonNullPointer(dictionary))
        return false;

    struct {
        sysbvm_weakValueDictionary_t *dictionary;
        sysbvm_tuple_t key;
        sysbvm_weakValueAssociation_t *association;
    } gcFrame = {
        .dictionary = (sysbvm_weakValueDictionary_t*)dictionary,
        .key = key
    };
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    intptr_t elementIndex = sysbvm_dictionary_scanFor(context, &gcFrame.dictionary, &gcFrame.key);
    if(elementIndex < 0)
    {
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return false;
    }

    sysbvm_dictionary_t *dictionaryObject = (sysbvm_dictionary_t*)dictionary;
    sysbvm_array_t *storage = (sysbvm_array_t*)dictionaryObject->storage;
    gcFrame.association = (sysbvm_weakValueAssociation_t*)storage->elements[elementIndex];
    if(!gcFrame.association)
    {
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return false;
    }

    *outValue = gcFrame.association->value;
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return true;
}

static void sysbvm_dictionary_increaseCapacity(sysbvm_context_t *context, sysbvm_dictionary_t **dictionary)
{
    struct {
        sysbvm_array_t *oldStorage;
        sysbvm_array_t *newStorage;
        sysbvm_association_t *association;
        sysbvm_tuple_t key;
    } gcFrame = {
        .oldStorage = (sysbvm_array_t*)(*dictionary)->storage,
    };
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    size_t oldCapacity = sysbvm_tuple_getSizeInSlots((sysbvm_tuple_t)gcFrame.oldStorage);
    size_t newCapacity = oldCapacity * 2;
    size_t newSize = 0;
    if(newCapacity < 8)
        newCapacity = 8;

    // Make the new storage.
    gcFrame.newStorage = (sysbvm_array_t*)sysbvm_array_create(context, newCapacity);
    (*dictionary)->storage = (sysbvm_tuple_t)gcFrame.newStorage;

    // Reinsert the old elements.
    for(size_t i = 0; i < oldCapacity; ++i)
    {
        gcFrame.association = (sysbvm_association_t *)gcFrame.oldStorage->elements[i];
        if(gcFrame.association)
        {
            gcFrame.key = gcFrame.association->key;
            sysbvm_dictionary_insertNoCheck(context, dictionary, &gcFrame.association, &gcFrame.key);
            ++newSize;
        }
    }

    // We need to recompute the size due to deleted weak objects.
    (*dictionary)->size = sysbvm_tuple_size_encode(context, newSize);
}

SYSBVM_API void sysbvm_dictionary_add(sysbvm_context_t *context, sysbvm_tuple_t dictionary, sysbvm_tuple_t association)
{
    if(!sysbvm_tuple_isNonNullPointer(dictionary)) return;

    struct {
        sysbvm_dictionary_t *dictionary;
        sysbvm_tuple_t key;
        sysbvm_tuple_t associationToInsert;
        sysbvm_tuple_t association;
    } gcFrame = {
        .dictionary = (sysbvm_dictionary_t*)dictionary,
        .associationToInsert = association,
    };
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.key = sysbvm_association_getKey(gcFrame.associationToInsert);

    intptr_t elementIndex = sysbvm_dictionary_scanFor(context, &gcFrame.dictionary, &gcFrame.key);
    if(elementIndex < 0)
    {
        sysbvm_dictionary_increaseCapacity(context, &gcFrame.dictionary);
        elementIndex = sysbvm_dictionary_scanFor(context, &gcFrame.dictionary, &gcFrame.key);
        if(elementIndex < 0)
           sysbvm_error("Dictionary out of memory.");
    }

    sysbvm_array_t *storage = (sysbvm_array_t*)gcFrame.dictionary->storage;
    gcFrame.association = storage->elements[elementIndex];
    if(gcFrame.association)
    {
        gcFrame.association = gcFrame.associationToInsert;
    }
    else
    {
        storage->elements[elementIndex] = gcFrame.associationToInsert;
        size_t capacity = sysbvm_tuple_getSizeInSlots(gcFrame.dictionary->storage);
        size_t newSize = sysbvm_tuple_size_decode(gcFrame.dictionary->size) + 1;
        gcFrame.dictionary->size = sysbvm_tuple_size_encode(context, newSize);
        size_t capacityThreshold = capacity * 4 / 5;

        // Make sure the maximum occupancy rate is not greater than 80%.
        if(newSize >= capacityThreshold)
            sysbvm_dictionary_increaseCapacity(context, &gcFrame.dictionary);
    }

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
}

SYSBVM_API void sysbvm_dictionary_atPut(sysbvm_context_t *context, sysbvm_tuple_t dictionary, sysbvm_tuple_t key, sysbvm_tuple_t value)
{
    if(!sysbvm_tuple_isNonNullPointer(dictionary)) return;

    struct {
        sysbvm_dictionary_t *dictionary;
        sysbvm_tuple_t key;
        sysbvm_tuple_t value;
        sysbvm_association_t *association;
    } gcFrame = {
        .dictionary = (sysbvm_dictionary_t*)dictionary,
        .key = key,
        .value = value,
    };
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    intptr_t elementIndex = sysbvm_dictionary_scanFor(context, &gcFrame.dictionary, &gcFrame.key);
    if(elementIndex < 0)
    {
        sysbvm_dictionary_increaseCapacity(context, &gcFrame.dictionary);
        elementIndex = sysbvm_dictionary_scanFor(context, &gcFrame.dictionary, &gcFrame.key);
        if(elementIndex < 0)
           sysbvm_error("Dictionary out of memory.");
    }

    sysbvm_array_t *storage = (sysbvm_array_t*)gcFrame.dictionary->storage;
    gcFrame.association = (sysbvm_association_t*)storage->elements[elementIndex];
    if(gcFrame.association)
    {
        gcFrame.association->value = gcFrame.value;
    }
    else
    {
        storage->elements[elementIndex] = sysbvm_association_create(context, gcFrame.key, gcFrame.value);
        size_t capacity = sysbvm_tuple_getSizeInSlots(gcFrame.dictionary->storage);
        size_t newSize = sysbvm_tuple_size_decode(gcFrame.dictionary->size) + 1;
        gcFrame.dictionary->size = sysbvm_tuple_size_encode(context, newSize);
        size_t capacityThreshold = capacity * 4 / 5;

        // Make sure the maximum occupancy rate is not greater than 80%.
        if(newSize >= capacityThreshold)
            sysbvm_dictionary_increaseCapacity(context, &gcFrame.dictionary);
    }

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
}

SYSBVM_API sysbvm_tuple_t sysbvm_weakValueDictionary_create(sysbvm_context_t *context)
{
    sysbvm_weakValueDictionary_t *result = (sysbvm_weakValueDictionary_t*)sysbvm_context_allocatePointerTuple(context, context->roots.weakValueDictionaryType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_weakValueDictionary_t));
    result->size = sysbvm_tuple_size_encode(context, 0);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API bool sysbvm_weakValueDictionary_find(sysbvm_context_t *context, sysbvm_tuple_t dictionary, sysbvm_tuple_t key, sysbvm_tuple_t *outValue)
{
    if(!sysbvm_tuple_isNonNullPointer(dictionary))
        return false;

    struct {
        sysbvm_weakValueDictionary_t *dictionary;
        sysbvm_tuple_t key;
        sysbvm_weakValueAssociation_t *association;
    } gcFrame = {
        .dictionary = (sysbvm_weakValueDictionary_t*)dictionary,
        .key = key
    };
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    intptr_t elementIndex = sysbvm_dictionary_scanFor(context, &gcFrame.dictionary, &gcFrame.key);
    if(elementIndex < 0)
    {
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return false;
    }

    sysbvm_dictionary_t *dictionaryObject = (sysbvm_dictionary_t*)dictionary;
    sysbvm_array_t *storage = (sysbvm_array_t*)dictionaryObject->storage;
    gcFrame.association = (sysbvm_weakValueAssociation_t*)storage->elements[elementIndex];
    if(!gcFrame.association || gcFrame.association->value == SYSBVM_TOMBSTONE_TUPLE)
    {
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return false;
    }

    *outValue = gcFrame.association->value;
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return true;
}

SYSBVM_API bool sysbvm_weakValueDictionary_findAssociation(sysbvm_context_t *context, sysbvm_tuple_t dictionary, sysbvm_tuple_t key, sysbvm_tuple_t *outAssociation)
{
    if(!sysbvm_tuple_isNonNullPointer(dictionary))
        return false;

    struct {
        sysbvm_weakValueDictionary_t *dictionary;
        sysbvm_tuple_t key;
        sysbvm_weakValueAssociation_t *association;
    } gcFrame = {
        .dictionary = (sysbvm_weakValueDictionary_t*)dictionary,
        .key = key
    };
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    intptr_t elementIndex = sysbvm_dictionary_scanFor(context, &gcFrame.dictionary, &gcFrame.key);
    if(elementIndex < 0)
    {
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return false;
    }

    sysbvm_dictionary_t *dictionaryObject = (sysbvm_dictionary_t*)dictionary;
    sysbvm_array_t *storage = (sysbvm_array_t*)dictionaryObject->storage;
    gcFrame.association = (sysbvm_weakValueAssociation_t*)storage->elements[elementIndex];
    if(!gcFrame.association || gcFrame.association->value == SYSBVM_TOMBSTONE_TUPLE)
    {
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return false;
    }

    *outAssociation = (sysbvm_tuple_t)gcFrame.association;
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return true;
}

static void sysbvm_weakValueDictionary_increaseCapacity(sysbvm_context_t *context, sysbvm_weakValueDictionary_t **dictionary)
{
    struct {
        sysbvm_array_t *oldStorage;
        sysbvm_array_t *newStorage;
        sysbvm_weakValueAssociation_t *association;
        sysbvm_tuple_t key;
    } gcFrame = {
        .oldStorage = (sysbvm_array_t*)(*dictionary)->storage,
    };
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    size_t oldCapacity = sysbvm_tuple_getSizeInSlots((sysbvm_tuple_t)gcFrame.oldStorage);
    size_t newCapacity = oldCapacity * 2;
    size_t newSize = 0;
    if(newCapacity < 8)
        newCapacity = 8;

    // Make the new storage.
    gcFrame.newStorage = (sysbvm_array_t*)sysbvm_array_create(context, newCapacity);
    (*dictionary)->storage = (sysbvm_tuple_t)gcFrame.newStorage;

    // Reinsert the old elements.
    for(size_t i = 0; i < oldCapacity; ++i)
    {
        gcFrame.association = (sysbvm_weakValueAssociation_t *)gcFrame.oldStorage->elements[i];
        if(gcFrame.association && gcFrame.association->value != SYSBVM_TOMBSTONE_TUPLE)
        {
            gcFrame.key = gcFrame.association->key;
            sysbvm_dictionary_insertNoCheck(context, dictionary, &gcFrame.association, &gcFrame.key);
            ++newSize;
        }
    }

    // We need to recompute the size due to deleted weak objects.
    (*dictionary)->size = sysbvm_tuple_size_encode(context, newSize);
}

SYSBVM_API void sysbvm_weakValueDictionary_atPut(sysbvm_context_t *context, sysbvm_tuple_t dictionary, sysbvm_tuple_t key, sysbvm_tuple_t value)
{
    if(!sysbvm_tuple_isNonNullPointer(dictionary)) return;

    struct {
        sysbvm_weakValueDictionary_t *dictionary;
        sysbvm_tuple_t key;
        sysbvm_tuple_t value;
        sysbvm_weakValueAssociation_t *association;
    } gcFrame = {
        .dictionary = (sysbvm_weakValueDictionary_t*)dictionary,
        .key = key,
        .value = value,
    };
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    intptr_t elementIndex = sysbvm_dictionary_scanFor(context, &gcFrame.dictionary, &gcFrame.key);
    if(elementIndex < 0)
    {
        sysbvm_weakValueDictionary_increaseCapacity(context, &gcFrame.dictionary);
        elementIndex = sysbvm_dictionary_scanFor(context, &gcFrame.dictionary, &gcFrame.key);
        if(elementIndex < 0)
           sysbvm_error("Dictionary out of memory.");
    }

    sysbvm_array_t *storage = (sysbvm_array_t*)gcFrame.dictionary->storage;
    gcFrame.association = (sysbvm_weakValueAssociation_t*)storage->elements[elementIndex];
    if(gcFrame.association)
    {
        gcFrame.association->value = gcFrame.value;
    }
    else
    {
        storage->elements[elementIndex] = sysbvm_weakValueAssociation_create(context, gcFrame.key, gcFrame.value);
        size_t capacity = sysbvm_tuple_getSizeInSlots(gcFrame.dictionary->storage);
        size_t newSize = sysbvm_tuple_size_decode(gcFrame.dictionary->size) + 1;
        gcFrame.dictionary->size = sysbvm_tuple_size_encode(context, newSize);
        size_t capacityThreshold = capacity * 4 / 5;

        // Make sure the maximum occupancy rate is not greater than 80%.
        if(newSize >= capacityThreshold)
            sysbvm_weakValueDictionary_increaseCapacity(context, &gcFrame.dictionary);
    }

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
}

SYSBVM_API sysbvm_tuple_t sysbvm_identityDictionary_create(sysbvm_context_t *context)
{
    sysbvm_identityDictionary_t *result = (sysbvm_identityDictionary_t*)sysbvm_context_allocatePointerTuple(context, context->roots.identityDictionaryType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_identityDictionary_t));
    result->size = sysbvm_tuple_size_encode(context, 0);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_methodDictionary_create(sysbvm_context_t *context)
{
    sysbvm_methodDictionary_t *result = (sysbvm_methodDictionary_t*)sysbvm_context_allocatePointerTuple(context, context->roots.methodDictionaryType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_methodDictionary_t));
    result->size = sysbvm_tuple_size_encode(context, 0);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_methodDictionary_createWithCapacity(sysbvm_context_t *context, size_t initialCapacity)
{
    size_t requiredStorageCapacity = initialCapacity * 130 / 100;
    sysbvm_methodDictionary_t *result = (sysbvm_methodDictionary_t*)sysbvm_context_allocatePointerTuple(context, context->roots.methodDictionaryType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_methodDictionary_t));
    result->size = sysbvm_tuple_size_encode(context, 0);
    if(requiredStorageCapacity > 0)
    {
        size_t entryCount = requiredStorageCapacity * 2;
        result->storage = sysbvm_array_create(context, entryCount);
        for(size_t i = 0; i < entryCount; ++i)
            sysbvm_array_atPut(result->storage, i, SYSBVM_HASHTABLE_EMPTY_ELEMENT_TUPLE);
    }

    return (sysbvm_tuple_t)result;
}

static intptr_t sysbvm_identityDictionary_scanFor(sysbvm_tuple_t dictionary, sysbvm_tuple_t element)
{
    if(!sysbvm_tuple_isNonNullPointer(dictionary))
        return -1;

    sysbvm_dictionary_t *dictionaryObject = (sysbvm_dictionary_t*)dictionary;
    size_t capacity = sysbvm_tuple_getSizeInSlots(dictionaryObject->storage);
    if(capacity == 0)
        return -1;

    sysbvm_array_t *storage = (sysbvm_array_t*)dictionaryObject->storage;
    size_t hashIndex = sysbvm_tuple_identityHash(element) % capacity;
    for(size_t i = hashIndex; i < capacity; ++i)
    {
        sysbvm_tuple_t association = storage->elements[i];
        if(!association || sysbvm_tuple_identityEquals(element, sysbvm_association_getKey(association)))
            return (intptr_t)i;
    }

    for(size_t i = 0; i < hashIndex; ++i)
    {
        sysbvm_tuple_t association = storage->elements[i];
        if(!association || sysbvm_tuple_identityEquals(element, sysbvm_association_getKey(association)))
            return (intptr_t)i;
    }

    return -1;
}

SYSBVM_API bool sysbvm_identityDictionary_findAssociation(sysbvm_tuple_t dictionary, sysbvm_tuple_t key, sysbvm_tuple_t *outAssociation)
{
    *outAssociation = SYSBVM_NULL_TUPLE;
    if(!sysbvm_tuple_isNonNullPointer(dictionary))
        return false;

    intptr_t elementIndex = sysbvm_identityDictionary_scanFor(dictionary, key);
    if(elementIndex < 0)
        return false;

    sysbvm_dictionary_t *dictionaryObject = (sysbvm_dictionary_t*)dictionary;
    sysbvm_array_t *storage = (sysbvm_array_t*)dictionaryObject->storage;
    if(!storage->elements[elementIndex])
        return false;

    *outAssociation = storage->elements[elementIndex];
    return true;
}

SYSBVM_API bool sysbvm_identityDictionary_find(sysbvm_tuple_t dictionary, sysbvm_tuple_t element, sysbvm_tuple_t *outValue)
{
    *outValue = SYSBVM_NULL_TUPLE;
    if(!sysbvm_tuple_isNonNullPointer(dictionary))
        return false;

    intptr_t elementIndex = sysbvm_identityDictionary_scanFor(dictionary, element);
    if(elementIndex < 0)
        return false;

    sysbvm_dictionary_t *dictionaryObject = (sysbvm_dictionary_t*)dictionary;
    sysbvm_array_t *storage = (sysbvm_array_t*)dictionaryObject->storage;
    if(!storage->elements[elementIndex])
        return false;

    *outValue = sysbvm_association_getValue(storage->elements[elementIndex]);
    return true;
}

static void sysbvm_identityDictionary_insertNoCheck(sysbvm_tuple_t dictionary, sysbvm_tuple_t association)
{
    intptr_t elementIndex = sysbvm_identityDictionary_scanFor(dictionary, sysbvm_association_getKey(association));
    SYSBVM_ASSERT(elementIndex >= 0);

    sysbvm_dictionary_t *dictionaryObject = (sysbvm_dictionary_t*)dictionary;
    sysbvm_array_t *storage = (sysbvm_array_t*)dictionaryObject->storage;
    storage->elements[elementIndex] = association;
}

static void sysbvm_identityDictionary_increaseCapacity(sysbvm_context_t *context, sysbvm_tuple_t dictionary)
{
    sysbvm_dictionary_t *dictionaryObject = (sysbvm_dictionary_t*)dictionary;
    sysbvm_array_t *oldStorage = (sysbvm_array_t*)dictionaryObject->storage;

    size_t oldCapacity = sysbvm_tuple_getSizeInSlots(dictionaryObject->storage);
    size_t newCapacity = oldCapacity * 2;
    if(newCapacity < 8)
        newCapacity = 8;

    // Make the new storage.
    sysbvm_array_t *newStorage = (sysbvm_array_t*)sysbvm_array_create(context, newCapacity);
    dictionaryObject->storage = (sysbvm_tuple_t)newStorage;

    // Reinsert the old elements.
    for(size_t i = 0; i < oldCapacity; ++i)
    {
        sysbvm_tuple_t association = oldStorage->elements[i];
        if(association)
            sysbvm_identityDictionary_insertNoCheck(dictionary, association);
    }
}

SYSBVM_API void sysbvm_identityDictionary_add(sysbvm_context_t *context, sysbvm_tuple_t dictionary, sysbvm_tuple_t association)
{
    if(!sysbvm_tuple_isNonNullPointer(dictionary))
        return;

    sysbvm_tuple_t key = sysbvm_association_getKey(association);
    intptr_t elementIndex = sysbvm_identityDictionary_scanFor(dictionary, key);
    if(elementIndex < 0)
    {
        sysbvm_identityDictionary_increaseCapacity(context, dictionary);
        elementIndex = sysbvm_identityDictionary_scanFor(dictionary, key);
        if(elementIndex < 0)
           sysbvm_error("Dictionary out of memory.");
    }

    sysbvm_dictionary_t *dictionaryObject = (sysbvm_dictionary_t*)dictionary;
    sysbvm_array_t *storage = (sysbvm_array_t*)dictionaryObject->storage;
    if(storage->elements[elementIndex])
    {
        storage->elements[elementIndex] = association;
    }
    else
    {
        storage->elements[elementIndex] = association;
        size_t capacity = sysbvm_tuple_getSizeInSlots(dictionaryObject->storage);
        size_t newSize = sysbvm_tuple_size_decode(dictionaryObject->size) + 1;
        dictionaryObject->size = sysbvm_tuple_size_encode(context, newSize);
        size_t capacityThreshold = capacity * 4 / 5;

        // Make sure the maximum occupancy rate is not greater than 80%.
        if(newSize >= capacityThreshold)
            sysbvm_identityDictionary_increaseCapacity(context, dictionary);
    }
}

SYSBVM_API void sysbvm_identityDictionary_atPut(sysbvm_context_t *context, sysbvm_tuple_t dictionary, sysbvm_tuple_t key, sysbvm_tuple_t value)
{
    if(!sysbvm_tuple_isNonNullPointer(dictionary))
        return;

    intptr_t elementIndex = sysbvm_identityDictionary_scanFor(dictionary, key);
    if(elementIndex < 0)
    {
        sysbvm_identityDictionary_increaseCapacity(context, dictionary);
        elementIndex = sysbvm_identityDictionary_scanFor(dictionary, key);
        if(elementIndex < 0)
           sysbvm_error("Dictionary out of memory.");
    }

    sysbvm_dictionary_t *dictionaryObject = (sysbvm_dictionary_t*)dictionary;
    sysbvm_array_t *storage = (sysbvm_array_t*)dictionaryObject->storage;
    if(storage->elements[elementIndex])
    {
        sysbvm_association_setValue(storage->elements[elementIndex], value);
    }
    else
    {
        storage->elements[elementIndex] = sysbvm_association_create(context, key, value);
        size_t capacity = sysbvm_tuple_getSizeInSlots(dictionaryObject->storage);
        size_t newSize = sysbvm_tuple_size_decode(dictionaryObject->size) + 1;
        dictionaryObject->size = sysbvm_tuple_size_encode(context, newSize);
        size_t capacityThreshold = capacity * 4 / 5;

        // Make sure the maximum occupancy rate is not greater than 80%.
        if(newSize >= capacityThreshold)
            sysbvm_identityDictionary_increaseCapacity(context, dictionary);
    }
}

static intptr_t sysbvm_methodDictionary_scanFor(sysbvm_tuple_t dictionary, sysbvm_tuple_t element)
{
    if(!sysbvm_tuple_isNonNullPointer(dictionary))
        return -1;

    sysbvm_dictionary_t *dictionaryObject = (sysbvm_dictionary_t*)dictionary;
    size_t capacity = sysbvm_tuple_getSizeInSlots(dictionaryObject->storage) / 2;
    if(capacity == 0)
        return -1;

    sysbvm_array_t *storage = (sysbvm_array_t*)dictionaryObject->storage;
    size_t hashIndex = sysbvm_tuple_identityHash(element) % capacity;
    for(size_t i = hashIndex; i < capacity; ++i)
    {
        sysbvm_tuple_t dictionaryKey = storage->elements[i*2];
        if(dictionaryKey == SYSBVM_HASHTABLE_EMPTY_ELEMENT_TUPLE || sysbvm_tuple_identityEquals(element, dictionaryKey))
            return (intptr_t)i;
    }

    for(size_t i = 0; i < hashIndex; ++i)
    {
        sysbvm_tuple_t dictionaryKey = storage->elements[i*2];
        if(dictionaryKey == SYSBVM_HASHTABLE_EMPTY_ELEMENT_TUPLE || sysbvm_tuple_identityEquals(element, dictionaryKey))
            return (intptr_t)i;
    }

    return -1;
}

SYSBVM_API bool sysbvm_methodDictionary_find(sysbvm_tuple_t dictionary, sysbvm_tuple_t element, sysbvm_tuple_t *outValue)
{
    *outValue = SYSBVM_NULL_TUPLE;
    if(!sysbvm_tuple_isNonNullPointer(dictionary))
        return false;

    intptr_t elementIndex = sysbvm_methodDictionary_scanFor(dictionary, element);
    if(elementIndex < 0)
        return false;

    sysbvm_dictionary_t *dictionaryObject = (sysbvm_dictionary_t*)dictionary;
    sysbvm_array_t *storage = (sysbvm_array_t*)dictionaryObject->storage;
    if(storage->elements[elementIndex*2] == SYSBVM_HASHTABLE_EMPTY_ELEMENT_TUPLE)
        return false;

    *outValue = storage->elements[elementIndex*2 + 1];
    return true;
}

static void sysbvm_methodDictionary_insertNoCheck(sysbvm_tuple_t dictionary, sysbvm_tuple_t key, sysbvm_tuple_t value)
{
    intptr_t elementIndex = sysbvm_methodDictionary_scanFor(dictionary, key);
    SYSBVM_ASSERT(elementIndex >= 0);

    sysbvm_dictionary_t *dictionaryObject = (sysbvm_dictionary_t*)dictionary;
    sysbvm_array_t *storage = (sysbvm_array_t*)dictionaryObject->storage;
    storage->elements[elementIndex*2] = key;
    storage->elements[elementIndex*2 + 1] = value;
}

static void sysbvm_methodDictionary_increaseCapacity(sysbvm_context_t *context, sysbvm_tuple_t dictionary)
{
    sysbvm_dictionary_t *dictionaryObject = (sysbvm_dictionary_t*)dictionary;
    sysbvm_array_t *oldStorage = (sysbvm_array_t*)dictionaryObject->storage;

    size_t oldCapacity = sysbvm_tuple_getSizeInSlots(dictionaryObject->storage) / 2;
    size_t newCapacity = oldCapacity * 2;
    if(newCapacity < 8)
        newCapacity = 8;

    // Make the new storage.
    sysbvm_array_t *newStorage = (sysbvm_array_t*)sysbvm_array_create(context, newCapacity*2);
    dictionaryObject->storage = (sysbvm_tuple_t)newStorage;
    for(size_t i = 0; i < newCapacity; ++i)
        newStorage->elements[i*2] = SYSBVM_HASHTABLE_EMPTY_ELEMENT_TUPLE;

    // Reinsert the old elements.
    for(size_t i = 0; i < oldCapacity; ++i)
    {
        sysbvm_tuple_t key = oldStorage->elements[i*2];
        sysbvm_tuple_t value = oldStorage->elements[i*2 + 1];
        if(key != SYSBVM_HASHTABLE_EMPTY_ELEMENT_TUPLE)
            sysbvm_methodDictionary_insertNoCheck(dictionary, key, value);
    }
}

SYSBVM_API void sysbvm_methodDictionary_atPut(sysbvm_context_t *context, sysbvm_tuple_t dictionary, sysbvm_tuple_t key, sysbvm_tuple_t value)
{
    if(key == SYSBVM_HASHTABLE_EMPTY_ELEMENT_TUPLE)
        return;

    if(!sysbvm_tuple_isNonNullPointer(dictionary))
        return;

    intptr_t elementIndex = sysbvm_methodDictionary_scanFor(dictionary, key);
    if(elementIndex < 0)
    {
        sysbvm_methodDictionary_increaseCapacity(context, dictionary);
        elementIndex = sysbvm_methodDictionary_scanFor(dictionary, key);
        if(elementIndex < 0)
           sysbvm_error("Dictionary out of memory.");
    }

    sysbvm_dictionary_t *dictionaryObject = (sysbvm_dictionary_t*)dictionary;
    sysbvm_array_t *storage = (sysbvm_array_t*)dictionaryObject->storage;
    bool isNewElement = storage->elements[elementIndex*2] == SYSBVM_HASHTABLE_EMPTY_ELEMENT_TUPLE;
    storage->elements[elementIndex*2] = key;
    storage->elements[elementIndex*2 + 1] = value;

    // Count the newly inserted element.
    if(isNewElement)
    {
        size_t capacity = sysbvm_tuple_getSizeInSlots(dictionaryObject->storage) / 2;
        size_t newSize = sysbvm_tuple_size_decode(dictionaryObject->size) + 1;
        dictionaryObject->size = sysbvm_tuple_size_encode(context, newSize);
        size_t capacityThreshold = capacity * 3 / 4;

        // Make sure the maximum occupancy rate is not greater than 80%.
        if(newSize >= capacityThreshold)
            sysbvm_methodDictionary_increaseCapacity(context, dictionary);
    }
}

static sysbvm_tuple_t sysbvm_dictionary_primitive_add(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_dictionary_add(context, arguments[0], arguments[1]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_dictionary_primitive_atOrNil(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t found = SYSBVM_NULL_TUPLE;
    if(!sysbvm_dictionary_find(context, arguments[0], arguments[1], &found))
        found = SYSBVM_NULL_TUPLE;

    return found;
}

static sysbvm_tuple_t sysbvm_dictionary_primitive_at(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t found = SYSBVM_NULL_TUPLE;
    if(!sysbvm_dictionary_find(context, arguments[0], arguments[1], &found))
        sysbvm_error("Failed to find the expected key in the dictionary.");

    return found;
}

static sysbvm_tuple_t sysbvm_dictionary_primitive_atPut(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_dictionary_atPut(context, arguments[0], arguments[1], arguments[2]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_dictionary_primitive_scanFor(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    return sysbvm_tuple_intptr_encode(context, sysbvm_dictionary_scanFor(context, (sysbvm_dictionary_t**)&arguments[0], &arguments[1]));
}

static sysbvm_tuple_t sysbvm_identityDictionary_primitive_add(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_identityDictionary_add(context, arguments[0], arguments[1]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_identityDictionary_primitive_atOrNil(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t found = SYSBVM_NULL_TUPLE;
    if(!sysbvm_identityDictionary_find(arguments[0], arguments[1], &found))
        found = SYSBVM_NULL_TUPLE;

    return found;
}

static sysbvm_tuple_t sysbvm_identityDictionary_primitive_at(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t found = SYSBVM_NULL_TUPLE;
    if(!sysbvm_identityDictionary_find(arguments[0], arguments[1], &found))
        sysbvm_error("Failed to find the expected key in the dictionary.");

    return found;
}

static sysbvm_tuple_t sysbvm_identityDictionary_primitive_atPut(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_identityDictionary_atPut(context, arguments[0], arguments[1], arguments[2]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_identityDictionary_primitive_scanFor(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    return sysbvm_tuple_intptr_encode(context, sysbvm_identityDictionary_scanFor(arguments[0], arguments[1]));
}

static sysbvm_tuple_t sysbvm_weakValueDictionary_primitive_atOrNil(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t found = SYSBVM_NULL_TUPLE;
    if(!sysbvm_weakValueDictionary_find(context, arguments[0], arguments[1], &found))
        found = SYSBVM_NULL_TUPLE;

    return found;
}

static sysbvm_tuple_t sysbvm_weakValueDictionary_primitive_at(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t found = SYSBVM_NULL_TUPLE;
    if(!sysbvm_weakValueDictionary_find(context, arguments[0], arguments[1], &found))
        sysbvm_error("Failed to find the expected key in the dictionary.");

    return found;
}

static sysbvm_tuple_t sysbvm_weakValueDictionary_primitive_atPut(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_weakValueDictionary_atPut(context, arguments[0], arguments[1], arguments[2]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_methodDictionary_primitive_new(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    (void)arguments;
    if(argumentCount != 0) sysbvm_error_argumentCountMismatch(0, argumentCount);

    return sysbvm_methodDictionary_create(context);
}

static sysbvm_tuple_t sysbvm_methodDictionary_primitive_atOrNil(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t found = SYSBVM_NULL_TUPLE;
    if(!sysbvm_methodDictionary_find(arguments[0], arguments[1], &found))
        found = SYSBVM_NULL_TUPLE;

    return found;
}

static sysbvm_tuple_t sysbvm_methodDictionary_primitive_at(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t found = SYSBVM_NULL_TUPLE;
    if(!sysbvm_methodDictionary_find(arguments[0], arguments[1], &found))
        sysbvm_error("Failed to find the expected key in the dictionary.");

    return found;
}

static sysbvm_tuple_t sysbvm_methodDictionary_primitive_atPut(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_methodDictionary_atPut(context, arguments[0], arguments[1], arguments[2]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_methodDictionary_primitive_scanFor(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    return sysbvm_tuple_intptr_encode(context, sysbvm_methodDictionary_scanFor(arguments[0], arguments[1]));
}

void sysbvm_dictionary_registerPrimitives(void)
{
    sysbvm_primitiveTable_registerFunction(sysbvm_dictionary_primitive_add, "Dictionary::add:");
    sysbvm_primitiveTable_registerFunction(sysbvm_dictionary_primitive_atOrNil, "Dictionary::atOrNil:");
    sysbvm_primitiveTable_registerFunction(sysbvm_dictionary_primitive_atPut, "Dictionary::at:put:");
    sysbvm_primitiveTable_registerFunction(sysbvm_dictionary_primitive_at, "Dictionary::at:");
    sysbvm_primitiveTable_registerFunction(sysbvm_dictionary_primitive_scanFor, "Dictionary::scanFor:");

    sysbvm_primitiveTable_registerFunction(sysbvm_identityDictionary_primitive_add, "IdentityDictionary::add:");
    sysbvm_primitiveTable_registerFunction(sysbvm_identityDictionary_primitive_atOrNil, "IdentityDictionary::atOrNil:");
    sysbvm_primitiveTable_registerFunction(sysbvm_identityDictionary_primitive_atPut, "IdentityDictionary::at:put:");
    sysbvm_primitiveTable_registerFunction(sysbvm_identityDictionary_primitive_at, "IdentityDictionary::at:");
    sysbvm_primitiveTable_registerFunction(sysbvm_identityDictionary_primitive_scanFor, "IdentityDictionary::scanFor:");

    sysbvm_primitiveTable_registerFunction(sysbvm_weakValueDictionary_primitive_atOrNil, "WeakValueDictionary::atOrNil:");
    sysbvm_primitiveTable_registerFunction(sysbvm_weakValueDictionary_primitive_atPut, "WeakValueDictionary::at:put:");
    sysbvm_primitiveTable_registerFunction(sysbvm_weakValueDictionary_primitive_at, "WeakValueDictionary::at:");

    sysbvm_primitiveTable_registerFunction(sysbvm_methodDictionary_primitive_new, "MethodDictionary::new");
    sysbvm_primitiveTable_registerFunction(sysbvm_methodDictionary_primitive_atOrNil, "MethodDictionary::atOrNil:");
    sysbvm_primitiveTable_registerFunction(sysbvm_methodDictionary_primitive_at, "MethodDictionary::at:");
    sysbvm_primitiveTable_registerFunction(sysbvm_methodDictionary_primitive_atPut, "MethodDictionary::at:put:");
    sysbvm_primitiveTable_registerFunction(sysbvm_methodDictionary_primitive_scanFor, "MethodDictionary::scanFor:");
}

void sysbvm_dictionary_setupPrimitives(sysbvm_context_t *context)
{
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Dictionary::add:", context->roots.dictionaryType, "add:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_dictionary_primitive_add);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Dictionary::atOrNil:", context->roots.dictionaryType, "atOrNil:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_dictionary_primitive_atOrNil);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Dictionary::at:put:", context->roots.dictionaryType, "at:put:", 3, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_dictionary_primitive_atPut);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Dictionary::at:", context->roots.dictionaryType, "at:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_dictionary_primitive_at);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Dictionary::scanFor:", context->roots.dictionaryType, "scanFor:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_dictionary_primitive_scanFor);

    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "IdentityDictionary::add:", context->roots.identityDictionaryType, "add:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_identityDictionary_primitive_add);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "IdentityDictionary::atOrNil:", context->roots.identityDictionaryType, "atOrNil:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_identityDictionary_primitive_atOrNil);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "IdentityDictionary::at:put:", context->roots.identityDictionaryType, "at:put:", 3, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_identityDictionary_primitive_atPut);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "IdentityDictionary::at:", context->roots.identityDictionaryType, "at:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_identityDictionary_primitive_at);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "IdentityDictionary::scanFor:", context->roots.identityDictionaryType, "scanFor:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_identityDictionary_primitive_scanFor);

    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "WeakValueDictionary::atOrNil:", context->roots.weakValueDictionaryType, "atOrNil:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_weakValueDictionary_primitive_atOrNil);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "WeakValueDictionary::at:put:", context->roots.weakValueDictionaryType, "at:put:", 3, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_weakValueDictionary_primitive_atPut);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "WeakValueDictionary::at:", context->roots.weakValueDictionaryType, "at:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_weakValueDictionary_primitive_at);

    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "MethodDictionary::new", 0, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_methodDictionary_primitive_new);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "MethodDictionary::atOrNil:", context->roots.methodDictionaryType, "atOrNil:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_TYPE_FLAGS_FINAL, NULL, sysbvm_methodDictionary_primitive_atOrNil);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "MethodDictionary::at:", context->roots.methodDictionaryType, "at:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_TYPE_FLAGS_FINAL, NULL, sysbvm_methodDictionary_primitive_at);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "MethodDictionary::at:put:", context->roots.methodDictionaryType, "at:put:", 3, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_TYPE_FLAGS_FINAL, NULL, sysbvm_methodDictionary_primitive_atPut);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "MethodDictionary::scanFor:", context->roots.methodDictionaryType, "scanFor:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_TYPE_FLAGS_FINAL, NULL, sysbvm_methodDictionary_primitive_scanFor);
}
