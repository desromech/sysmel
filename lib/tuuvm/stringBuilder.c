#include "tuuvm/stringBuilder.h"
#include "tuuvm/errors.h"
#include "tuuvm/string.h"
#include "internal/context.h"
#include <string.h>

TUUVM_API tuuvm_tuple_t tuuvm_stringBuilder_create(tuuvm_context_t *context)
{
    tuuvm_stringBuilder_t *result = (tuuvm_stringBuilder_t*)tuuvm_context_allocatePointerTuple(context, context->roots.stringBuilderType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_stringBuilder_t));
    result->size = tuuvm_tuple_size_encode(context, 0);
    return (tuuvm_tuple_t)result;
}

static void tuuvm_arrayList_increaseCapacityToAtLeast(tuuvm_context_t *context, tuuvm_tuple_t stringBuilder, size_t requiredCapacity)
{
    tuuvm_stringBuilder_t *stringBuilderObject = (tuuvm_stringBuilder_t*)stringBuilder;
    size_t size = tuuvm_tuple_size_decode(stringBuilderObject->size);
    size_t oldCapacity = tuuvm_tuple_getSizeInSlots(stringBuilderObject->storage);
    size_t newCapacity = oldCapacity * 2;
    if(newCapacity < 16)
        newCapacity = 16;
    while(newCapacity < requiredCapacity)
        newCapacity *= 2;

    tuuvm_object_tuple_t *newStorage = (tuuvm_object_tuple_t*)tuuvm_string_createEmptyWithSize(context, newCapacity);
    tuuvm_object_tuple_t *oldStorage = (tuuvm_object_tuple_t*)stringBuilderObject->storage;
    memcpy(newStorage->bytes, oldStorage->bytes, size);
    stringBuilderObject->storage = (tuuvm_tuple_t)newStorage;
}

TUUVM_API void tuuvm_stringBuilder_add(tuuvm_context_t *context, tuuvm_tuple_t stringBuilder, uint8_t character)
{
    if(!tuuvm_tuple_isNonNullPointer(stringBuilder))
        return;

    tuuvm_stringBuilder_t *stringBuilderObject = (tuuvm_stringBuilder_t*)stringBuilder;
    size_t size = tuuvm_tuple_size_decode(stringBuilderObject->size);
    size_t capacity = tuuvm_tuple_getSizeInSlots(stringBuilderObject->storage);
    size_t requiredCapacity = size + 1;
    if(requiredCapacity > capacity)
        tuuvm_arrayList_increaseCapacityToAtLeast(context, stringBuilder, requiredCapacity);

    tuuvm_object_tuple_t *storage = (tuuvm_object_tuple_t*)stringBuilderObject->storage;
    storage->bytes[size] = character;
    stringBuilderObject->size = tuuvm_tuple_size_encode(context, size + 1);

}

TUUVM_API void tuuvm_stringBuilder_addStringWithSize(tuuvm_context_t *context, tuuvm_tuple_t stringBuilder, size_t stringSize, const char *string)
{
    if(!tuuvm_tuple_isNonNullPointer(stringBuilder) || stringSize == 0)
        return;

    tuuvm_stringBuilder_t *stringBuilderObject = (tuuvm_stringBuilder_t*)stringBuilder;
    size_t size = tuuvm_tuple_size_decode(stringBuilderObject->size);
    size_t capacity = tuuvm_tuple_getSizeInSlots(stringBuilderObject->storage);
    size_t requiredCapacity = size + stringSize;
    if(requiredCapacity > capacity)
        tuuvm_arrayList_increaseCapacityToAtLeast(context, stringBuilder, requiredCapacity);

    tuuvm_object_tuple_t *storage = (tuuvm_object_tuple_t*)stringBuilderObject->storage;
    memcpy(storage->bytes, string, stringSize);
    stringBuilderObject->size = tuuvm_tuple_size_encode(context, size + stringSize);
}

TUUVM_API void tuuvm_stringBuilder_addString(tuuvm_context_t *context, tuuvm_tuple_t stringBuilder, tuuvm_tuple_t string)
{
    if(!tuuvm_tuple_isNonNullPointer(string))
        return;

    return tuuvm_stringBuilder_addStringWithSize(context, stringBuilder, tuuvm_tuple_getSizeInBytes(string), (const char*)TUUVM_CAST_OOP_TO_OBJECT_TUPLE(string)->bytes);
}

TUUVM_API void tuuvm_stringBuilder_addCString(tuuvm_context_t *context, tuuvm_tuple_t stringBuilder, const char *cstring)
{
    tuuvm_stringBuilder_addStringWithSize(context, stringBuilder, strlen(cstring), cstring);
}

TUUVM_API tuuvm_tuple_t tuuvm_stringBuilder_asString(tuuvm_context_t *context, tuuvm_tuple_t stringBuilder)
{
    if(!tuuvm_tuple_isNonNullPointer(stringBuilder))
        return TUUVM_NULL_TUPLE;

    tuuvm_stringBuilder_t *stringBuilderObject = (tuuvm_stringBuilder_t*)stringBuilder;
    size_t size = tuuvm_tuple_size_decode(stringBuilderObject->size);
    tuuvm_object_tuple_t *result = (tuuvm_object_tuple_t*)tuuvm_string_createEmptyWithSize(context, size);
    tuuvm_object_tuple_t *storage = (tuuvm_object_tuple_t*)stringBuilderObject->storage;
    memcpy(result->bytes, storage->bytes, size);
    return (tuuvm_tuple_t)result;
}

TUUVM_API size_t tuuvm_stringBuilder_getSize(tuuvm_tuple_t stringBuilder)
{
    if(!tuuvm_tuple_isNonNullPointer(stringBuilder)) return 0;
    return tuuvm_tuple_size_decode(((tuuvm_stringBuilder_t*)stringBuilder)->size);
}

TUUVM_API size_t tuuvm_stringBuilder_getCapacity(tuuvm_tuple_t stringBuilder)
{
    if(!tuuvm_tuple_isNonNullPointer(stringBuilder)) return 0;
    return tuuvm_tuple_getSizeInSlots(((tuuvm_stringBuilder_t*)stringBuilder)->storage);
}
