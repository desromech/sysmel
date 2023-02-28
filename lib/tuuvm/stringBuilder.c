#include "tuuvm/stringBuilder.h"
#include "tuuvm/errors.h"
#include "tuuvm/function.h"
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
    size_t oldCapacity = tuuvm_tuple_getSizeInBytes(stringBuilderObject->storage);
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
    size_t capacity = tuuvm_tuple_getSizeInBytes(stringBuilderObject->storage);
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
    size_t capacity = tuuvm_tuple_getSizeInBytes(stringBuilderObject->storage);
    size_t requiredCapacity = size + stringSize;
    if(requiredCapacity > capacity)
        tuuvm_arrayList_increaseCapacityToAtLeast(context, stringBuilder, requiredCapacity);

    tuuvm_object_tuple_t *storage = (tuuvm_object_tuple_t*)stringBuilderObject->storage;
    memcpy(storage->bytes + size, string, stringSize);
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

TUUVM_API tuuvm_tuple_t tuuvm_stringBuilder_asSymbol(tuuvm_context_t *context, tuuvm_tuple_t stringBuilder)
{
    return tuuvm_symbol_internFromTuple(context, tuuvm_stringBuilder_asString(context, stringBuilder));
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

static tuuvm_tuple_t tuuvm_stringBuilder_primitive_add(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_stringBuilder_add(context, arguments[0], arguments[1]);
    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_stringBuilder_primitive_addAll(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_stringBuilder_addString(context, arguments[0], arguments[1]);
    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_stringBuilder_primitive_asString(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_stringBuilder_asString(context, arguments[0]);
}

static tuuvm_tuple_t tuuvm_stringBuilder_primitive_asSymbol(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_stringBuilder_asSymbol(context, arguments[0]);
}

void tuuvm_stringBuilder_setupPrimitives(tuuvm_context_t *context)
{
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveMethod(context, "StringBuilder::add:", context->roots.stringBuilderType, "add:", 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_stringBuilder_primitive_add);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveMethod(context, "StringBuilder::addAll:", context->roots.stringBuilderType, "addAll:", 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_stringBuilder_primitive_addAll);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveMethod(context, "StringBuilder::asString", context->roots.stringBuilderType, "asString", 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_stringBuilder_primitive_asString);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveMethod(context, "StringBuilder::asSymbol", context->roots.stringBuilderType, "asSymbol", 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_stringBuilder_primitive_asSymbol);
}
