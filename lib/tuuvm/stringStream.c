#include "tuuvm/stringStream.h"
#include "tuuvm/errors.h"
#include "tuuvm/function.h"
#include "tuuvm/string.h"
#include "internal/context.h"
#include <string.h>

TUUVM_API tuuvm_tuple_t tuuvm_stringStream_create(tuuvm_context_t *context)
{
    tuuvm_stringStream_t *result = (tuuvm_stringStream_t*)tuuvm_context_allocatePointerTuple(context, context->roots.stringStreamType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_stringStream_t));
    result->size = tuuvm_tuple_size_encode(context, 0);
    return (tuuvm_tuple_t)result;
}

static void tuuvm_arrayList_increaseCapacityToAtLeast(tuuvm_context_t *context, tuuvm_tuple_t stringStream, size_t requiredCapacity)
{
    tuuvm_stringStream_t *stringStreamObject = (tuuvm_stringStream_t*)stringStream;
    size_t size = tuuvm_tuple_size_decode(stringStreamObject->size);
    size_t oldCapacity = tuuvm_tuple_getSizeInBytes(stringStreamObject->storage);
    size_t newCapacity = oldCapacity * 2;
    if(newCapacity < 16)
        newCapacity = 16;
    while(newCapacity < requiredCapacity)
        newCapacity *= 2;

    tuuvm_object_tuple_t *newStorage = (tuuvm_object_tuple_t*)tuuvm_string_createEmptyWithSize(context, newCapacity);
    tuuvm_object_tuple_t *oldStorage = (tuuvm_object_tuple_t*)stringStreamObject->storage;
    memcpy(newStorage->bytes, oldStorage->bytes, size);
    stringStreamObject->storage = (tuuvm_tuple_t)newStorage;
}

TUUVM_API void tuuvm_stringStream_nextPut(tuuvm_context_t *context, tuuvm_tuple_t stringStream, uint8_t character)
{
    if(!tuuvm_tuple_isNonNullPointer(stringStream))
        return;

    tuuvm_stringStream_t *stringStreamObject = (tuuvm_stringStream_t*)stringStream;
    size_t size = tuuvm_tuple_size_decode(stringStreamObject->size);
    size_t capacity = tuuvm_tuple_getSizeInBytes(stringStreamObject->storage);
    size_t requiredCapacity = size + 1;
    if(requiredCapacity > capacity)
        tuuvm_arrayList_increaseCapacityToAtLeast(context, stringStream, requiredCapacity);

    tuuvm_object_tuple_t *storage = (tuuvm_object_tuple_t*)stringStreamObject->storage;
    storage->bytes[size] = character;
    stringStreamObject->size = tuuvm_tuple_size_encode(context, size + 1);

}

TUUVM_API void tuuvm_stringStream_nextPutStringWithSize(tuuvm_context_t *context, tuuvm_tuple_t stringStream, size_t stringSize, const char *string)
{
    if(!tuuvm_tuple_isNonNullPointer(stringStream) || stringSize == 0)
        return;

    tuuvm_stringStream_t *stringStreamObject = (tuuvm_stringStream_t*)stringStream;
    size_t size = tuuvm_tuple_size_decode(stringStreamObject->size);
    size_t capacity = tuuvm_tuple_getSizeInBytes(stringStreamObject->storage);
    size_t requiredCapacity = size + stringSize;
    if(requiredCapacity > capacity)
        tuuvm_arrayList_increaseCapacityToAtLeast(context, stringStream, requiredCapacity);

    tuuvm_object_tuple_t *storage = (tuuvm_object_tuple_t*)stringStreamObject->storage;
    memcpy(storage->bytes + size, string, stringSize);
    stringStreamObject->size = tuuvm_tuple_size_encode(context, size + stringSize);
}

TUUVM_API void tuuvm_stringStream_nextPutString(tuuvm_context_t *context, tuuvm_tuple_t stringStream, tuuvm_tuple_t string)
{
    if(!tuuvm_tuple_isNonNullPointer(string))
        return;

    tuuvm_stringStream_nextPutStringWithSize(context, stringStream, tuuvm_tuple_getSizeInBytes(string), (const char*)TUUVM_CAST_OOP_TO_OBJECT_TUPLE(string)->bytes);
}

TUUVM_API void tuuvm_stringStream_nextPutCString(tuuvm_context_t *context, tuuvm_tuple_t stringStream, const char *cstring)
{
    tuuvm_stringStream_nextPutStringWithSize(context, stringStream, strlen(cstring), cstring);
}

TUUVM_API tuuvm_tuple_t tuuvm_stringStream_asString(tuuvm_context_t *context, tuuvm_tuple_t stringStream)
{
    if(!tuuvm_tuple_isNonNullPointer(stringStream))
        return TUUVM_NULL_TUPLE;

    tuuvm_stringStream_t *stringStreamObject = (tuuvm_stringStream_t*)stringStream;
    size_t size = tuuvm_tuple_size_decode(stringStreamObject->size);
    tuuvm_object_tuple_t *result = (tuuvm_object_tuple_t*)tuuvm_string_createEmptyWithSize(context, size);
    tuuvm_object_tuple_t *storage = (tuuvm_object_tuple_t*)stringStreamObject->storage;
    memcpy(result->bytes, storage->bytes, size);
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_stringStream_asSymbol(tuuvm_context_t *context, tuuvm_tuple_t stringStream)
{
    return tuuvm_symbol_internFromTuple(context, tuuvm_stringStream_asString(context, stringStream));
}

TUUVM_API size_t tuuvm_stringStream_getSize(tuuvm_tuple_t stringStream)
{
    if(!tuuvm_tuple_isNonNullPointer(stringStream)) return 0;
    return tuuvm_tuple_size_decode(((tuuvm_stringStream_t*)stringStream)->size);
}

TUUVM_API size_t tuuvm_stringStream_getCapacity(tuuvm_tuple_t stringStream)
{
    if(!tuuvm_tuple_isNonNullPointer(stringStream)) return 0;
    return tuuvm_tuple_getSizeInSlots(((tuuvm_stringStream_t*)stringStream)->storage);
}

static tuuvm_tuple_t tuuvm_stringStream_primitive_nextPut(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_stringStream_nextPut(context, arguments[0], tuuvm_tuple_char8_decode(arguments[1]));
    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_stringStream_primitive_nextPutAll(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_stringStream_nextPutString(context, arguments[0], arguments[1]);
    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_stringStream_primitive_asString(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_stringStream_asString(context, arguments[0]);
}

static tuuvm_tuple_t tuuvm_stringStream_primitive_asSymbol(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_stringStream_asSymbol(context, arguments[0]);
}

void tuuvm_stringStream_registerPrimitives(void)
{
    tuuvm_primitiveTable_registerFunction(tuuvm_stringStream_primitive_nextPut, "StringStream::nextPut:");
    tuuvm_primitiveTable_registerFunction(tuuvm_stringStream_primitive_nextPutAll, "StringStream::nextPutAll:");
    tuuvm_primitiveTable_registerFunction(tuuvm_stringStream_primitive_asString, "StringStream::asString");
    tuuvm_primitiveTable_registerFunction(tuuvm_stringStream_primitive_asSymbol, "StringStream::asSymbol");
}

void tuuvm_stringStream_setupPrimitives(tuuvm_context_t *context)
{
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "StringStream::nextPut:", context->roots.stringStreamType, "nextPut:", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_stringStream_primitive_nextPut);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "StringStream::nextPutAll:", context->roots.stringStreamType, "nextPutAll:", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_stringStream_primitive_nextPutAll);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "StringStream::asString", context->roots.stringStreamType, "asString", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_stringStream_primitive_asString);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "StringStream::asSymbol", context->roots.stringStreamType, "asSymbol", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_stringStream_primitive_asSymbol);
}
