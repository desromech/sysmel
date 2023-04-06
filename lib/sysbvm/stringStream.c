#include "sysbvm/stringStream.h"
#include "sysbvm/errors.h"
#include "sysbvm/function.h"
#include "sysbvm/string.h"
#include "internal/context.h"
#include <string.h>

SYSBVM_API sysbvm_tuple_t sysbvm_stringStream_create(sysbvm_context_t *context)
{
    sysbvm_stringStream_t *result = (sysbvm_stringStream_t*)sysbvm_context_allocatePointerTuple(context, context->roots.stringStreamType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_stringStream_t));
    result->size = sysbvm_tuple_size_encode(context, 0);
    return (sysbvm_tuple_t)result;
}

static void sysbvm_arrayList_increaseCapacityToAtLeast(sysbvm_context_t *context, sysbvm_tuple_t stringStream, size_t requiredCapacity)
{
    sysbvm_stringStream_t *stringStreamObject = (sysbvm_stringStream_t*)stringStream;
    size_t size = sysbvm_tuple_size_decode(stringStreamObject->size);
    size_t oldCapacity = sysbvm_tuple_getSizeInBytes(stringStreamObject->storage);
    size_t newCapacity = oldCapacity * 2;
    if(newCapacity < 16)
        newCapacity = 16;
    while(newCapacity < requiredCapacity)
        newCapacity *= 2;

    sysbvm_object_tuple_t *newStorage = (sysbvm_object_tuple_t*)sysbvm_string_createEmptyWithSize(context, newCapacity);
    sysbvm_object_tuple_t *oldStorage = (sysbvm_object_tuple_t*)stringStreamObject->storage;
    memcpy(newStorage->bytes, oldStorage->bytes, size);
    stringStreamObject->storage = (sysbvm_tuple_t)newStorage;
}

SYSBVM_API void sysbvm_stringStream_nextPut(sysbvm_context_t *context, sysbvm_tuple_t stringStream, uint8_t character)
{
    if(!sysbvm_tuple_isNonNullPointer(stringStream))
        return;

    sysbvm_stringStream_t *stringStreamObject = (sysbvm_stringStream_t*)stringStream;
    size_t size = sysbvm_tuple_size_decode(stringStreamObject->size);
    size_t capacity = sysbvm_tuple_getSizeInBytes(stringStreamObject->storage);
    size_t requiredCapacity = size + 1;
    if(requiredCapacity > capacity)
        sysbvm_arrayList_increaseCapacityToAtLeast(context, stringStream, requiredCapacity);

    sysbvm_object_tuple_t *storage = (sysbvm_object_tuple_t*)stringStreamObject->storage;
    storage->bytes[size] = character;
    stringStreamObject->size = sysbvm_tuple_size_encode(context, size + 1);

}

SYSBVM_API void sysbvm_stringStream_nextPutStringWithSize(sysbvm_context_t *context, sysbvm_tuple_t stringStream, size_t stringSize, const char *string)
{
    if(!sysbvm_tuple_isNonNullPointer(stringStream) || stringSize == 0)
        return;

    sysbvm_stringStream_t *stringStreamObject = (sysbvm_stringStream_t*)stringStream;
    size_t size = sysbvm_tuple_size_decode(stringStreamObject->size);
    size_t capacity = sysbvm_tuple_getSizeInBytes(stringStreamObject->storage);
    size_t requiredCapacity = size + stringSize;
    if(requiredCapacity > capacity)
        sysbvm_arrayList_increaseCapacityToAtLeast(context, stringStream, requiredCapacity);

    sysbvm_object_tuple_t *storage = (sysbvm_object_tuple_t*)stringStreamObject->storage;
    memcpy(storage->bytes + size, string, stringSize);
    stringStreamObject->size = sysbvm_tuple_size_encode(context, size + stringSize);
}

SYSBVM_API void sysbvm_stringStream_nextPutString(sysbvm_context_t *context, sysbvm_tuple_t stringStream, sysbvm_tuple_t string)
{
    if(!sysbvm_tuple_isNonNullPointer(string))
        return;

    sysbvm_stringStream_nextPutStringWithSize(context, stringStream, sysbvm_tuple_getSizeInBytes(string), (const char*)SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(string)->bytes);
}

SYSBVM_API void sysbvm_stringStream_nextPutCString(sysbvm_context_t *context, sysbvm_tuple_t stringStream, const char *cstring)
{
    sysbvm_stringStream_nextPutStringWithSize(context, stringStream, strlen(cstring), cstring);
}

SYSBVM_API sysbvm_tuple_t sysbvm_stringStream_asString(sysbvm_context_t *context, sysbvm_tuple_t stringStream)
{
    if(!sysbvm_tuple_isNonNullPointer(stringStream))
        return SYSBVM_NULL_TUPLE;

    sysbvm_stringStream_t *stringStreamObject = (sysbvm_stringStream_t*)stringStream;
    size_t size = sysbvm_tuple_size_decode(stringStreamObject->size);
    sysbvm_object_tuple_t *result = (sysbvm_object_tuple_t*)sysbvm_string_createEmptyWithSize(context, size);
    sysbvm_object_tuple_t *storage = (sysbvm_object_tuple_t*)stringStreamObject->storage;
    memcpy(result->bytes, storage->bytes, size);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_stringStream_asSymbol(sysbvm_context_t *context, sysbvm_tuple_t stringStream)
{
    return sysbvm_symbol_internFromTuple(context, sysbvm_stringStream_asString(context, stringStream));
}

SYSBVM_API size_t sysbvm_stringStream_getSize(sysbvm_tuple_t stringStream)
{
    if(!sysbvm_tuple_isNonNullPointer(stringStream)) return 0;
    return sysbvm_tuple_size_decode(((sysbvm_stringStream_t*)stringStream)->size);
}

SYSBVM_API size_t sysbvm_stringStream_getCapacity(sysbvm_tuple_t stringStream)
{
    if(!sysbvm_tuple_isNonNullPointer(stringStream)) return 0;
    return sysbvm_tuple_getSizeInSlots(((sysbvm_stringStream_t*)stringStream)->storage);
}

static sysbvm_tuple_t sysbvm_stringStream_primitive_nextPut(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_stringStream_nextPut(context, arguments[0], sysbvm_tuple_char8_decode(arguments[1]));
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_stringStream_primitive_nextPutAll(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_stringStream_nextPutString(context, arguments[0], arguments[1]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_stringStream_primitive_asString(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_stringStream_asString(context, arguments[0]);
}

static sysbvm_tuple_t sysbvm_stringStream_primitive_asSymbol(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_stringStream_asSymbol(context, arguments[0]);
}

void sysbvm_stringStream_registerPrimitives(void)
{
    sysbvm_primitiveTable_registerFunction(sysbvm_stringStream_primitive_nextPut, "StringStream::nextPut:");
    sysbvm_primitiveTable_registerFunction(sysbvm_stringStream_primitive_nextPutAll, "StringStream::nextPutAll:");
    sysbvm_primitiveTable_registerFunction(sysbvm_stringStream_primitive_asString, "StringStream::asString");
    sysbvm_primitiveTable_registerFunction(sysbvm_stringStream_primitive_asSymbol, "StringStream::asSymbol");
}

void sysbvm_stringStream_setupPrimitives(sysbvm_context_t *context)
{
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "StringStream::nextPut:", context->roots.stringStreamType, "nextPut:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_stringStream_primitive_nextPut);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "StringStream::nextPutAll:", context->roots.stringStreamType, "nextPutAll:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_stringStream_primitive_nextPutAll);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "StringStream::asString", context->roots.stringStreamType, "asString", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_stringStream_primitive_asString);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "StringStream::asSymbol", context->roots.stringStreamType, "asSymbol", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_stringStream_primitive_asSymbol);
}
