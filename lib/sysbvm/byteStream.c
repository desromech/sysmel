#include "sysbvm/byteStream.h"
#include "sysbvm/array.h"
#include "sysbvm/errors.h"
#include "sysbvm/function.h"
#include "sysbvm/string.h"
#include "internal/context.h"
#include <string.h>

SYSBVM_API sysbvm_tuple_t sysbvm_byteStream_create(sysbvm_context_t *context)
{
    sysbvm_byteStream_t *result = (sysbvm_byteStream_t*)sysbvm_context_allocatePointerTuple(context, context->roots.byteStreamType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_byteStream_t));
    result->size = sysbvm_tuple_size_encode(context, 0);
    return (sysbvm_tuple_t)result;
}

static void sysbvm_orderedCollection_increaseCapacityToAtLeast(sysbvm_context_t *context, sysbvm_tuple_t byteStream, size_t requiredCapacity)
{
    sysbvm_byteStream_t *byteStreamObject = (sysbvm_byteStream_t*)byteStream;
    size_t size = sysbvm_tuple_size_decode(byteStreamObject->size);
    size_t oldCapacity = sysbvm_tuple_getSizeInBytes(byteStreamObject->storage);
    size_t newCapacity = oldCapacity * 2;
    if(newCapacity < 16)
        newCapacity = 16;
    while(newCapacity < requiredCapacity)
        newCapacity *= 2;

    sysbvm_object_tuple_t *newStorage = (sysbvm_object_tuple_t*)sysbvm_byteArray_create(context, newCapacity);
    sysbvm_object_tuple_t *oldStorage = (sysbvm_object_tuple_t*)byteStreamObject->storage;
    memcpy(newStorage->bytes, oldStorage->bytes, size);
    byteStreamObject->storage = (sysbvm_tuple_t)newStorage;
}

SYSBVM_API void sysbvm_byteStream_nextPut(sysbvm_context_t *context, sysbvm_tuple_t byteStream, uint8_t value)
{
    if(!sysbvm_tuple_isNonNullPointer(byteStream))
        return;

    sysbvm_byteStream_t *byteStreamObject = (sysbvm_byteStream_t*)byteStream;
    size_t size = sysbvm_tuple_size_decode(byteStreamObject->size);
    size_t capacity = sysbvm_tuple_getSizeInBytes(byteStreamObject->storage);
    size_t requiredCapacity = size + 1;
    if(requiredCapacity > capacity)
        sysbvm_orderedCollection_increaseCapacityToAtLeast(context, byteStream, requiredCapacity);

    sysbvm_object_tuple_t *storage = (sysbvm_object_tuple_t*)byteStreamObject->storage;
    storage->bytes[size] = value;
    byteStreamObject->size = sysbvm_tuple_size_encode(context, size + 1);

}

SYSBVM_API void sysbvm_byteStream_nextPutStringWithSize(sysbvm_context_t *context, sysbvm_tuple_t byteStream, size_t stringSize, const char *string)
{
    if(!sysbvm_tuple_isNonNullPointer(byteStream) || stringSize == 0)
        return;

    sysbvm_byteStream_t *byteStreamObject = (sysbvm_byteStream_t*)byteStream;
    size_t size = sysbvm_tuple_size_decode(byteStreamObject->size);
    size_t capacity = sysbvm_tuple_getSizeInBytes(byteStreamObject->storage);
    size_t requiredCapacity = size + stringSize;
    if(requiredCapacity > capacity)
        sysbvm_orderedCollection_increaseCapacityToAtLeast(context, byteStream, requiredCapacity);

    sysbvm_object_tuple_t *storage = (sysbvm_object_tuple_t*)byteStreamObject->storage;
    memcpy(storage->bytes + size, string, stringSize);
    byteStreamObject->size = sysbvm_tuple_size_encode(context, size + stringSize);
}

SYSBVM_API void sysbvm_byteStream_nextPutByteArray(sysbvm_context_t *context, sysbvm_tuple_t byteStream, sysbvm_tuple_t string)
{
    if(!sysbvm_tuple_isNonNullPointer(string))
        return;

    sysbvm_byteStream_nextPutStringWithSize(context, byteStream, sysbvm_tuple_getSizeInBytes(string), (const char*)SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(string)->bytes);
}

SYSBVM_API void sysbvm_byteStream_nextPutString(sysbvm_context_t *context, sysbvm_tuple_t byteStream, sysbvm_tuple_t string)
{
    if(!sysbvm_tuple_isNonNullPointer(string))
        return;

    sysbvm_byteStream_nextPutStringWithSize(context, byteStream, sysbvm_tuple_getSizeInBytes(string), (const char*)SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(string)->bytes);
}

SYSBVM_API void sysbvm_byteStream_nextPutCString(sysbvm_context_t *context, sysbvm_tuple_t byteStream, const char *cstring)
{
    sysbvm_byteStream_nextPutStringWithSize(context, byteStream, strlen(cstring), cstring);
}

SYSBVM_API sysbvm_tuple_t sysbvm_byteStream_asByteArray(sysbvm_context_t *context, sysbvm_tuple_t byteStream)
{
    if(!sysbvm_tuple_isNonNullPointer(byteStream))
        return SYSBVM_NULL_TUPLE;

    sysbvm_byteStream_t *byteStreamObject = (sysbvm_byteStream_t*)byteStream;
    size_t size = sysbvm_tuple_size_decode(byteStreamObject->size);
    sysbvm_object_tuple_t *result = (sysbvm_object_tuple_t*)sysbvm_byteArray_create(context, size);
    sysbvm_object_tuple_t *storage = (sysbvm_object_tuple_t*)byteStreamObject->storage;
    memcpy(result->bytes, storage->bytes, size);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_byteStream_asString(sysbvm_context_t *context, sysbvm_tuple_t byteStream)
{
    if(!sysbvm_tuple_isNonNullPointer(byteStream))
        return SYSBVM_NULL_TUPLE;

    sysbvm_byteStream_t *byteStreamObject = (sysbvm_byteStream_t*)byteStream;
    size_t size = sysbvm_tuple_size_decode(byteStreamObject->size);
    sysbvm_object_tuple_t *result = (sysbvm_object_tuple_t*)sysbvm_string_createEmptyWithSize(context, size);
    sysbvm_object_tuple_t *storage = (sysbvm_object_tuple_t*)byteStreamObject->storage;
    memcpy(result->bytes, storage->bytes, size);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_byteStream_asSymbol(sysbvm_context_t *context, sysbvm_tuple_t byteStream)
{
    return sysbvm_symbol_internFromTuple(context, sysbvm_byteStream_asString(context, byteStream));
}

SYSBVM_API size_t sysbvm_byteStream_getSize(sysbvm_tuple_t byteStream)
{
    if(!sysbvm_tuple_isNonNullPointer(byteStream)) return 0;
    return sysbvm_tuple_size_decode(((sysbvm_byteStream_t*)byteStream)->size);
}

SYSBVM_API size_t sysbvm_byteStream_getCapacity(sysbvm_tuple_t byteStream)
{
    if(!sysbvm_tuple_isNonNullPointer(byteStream)) return 0;
    return sysbvm_tuple_getSizeInSlots(((sysbvm_byteStream_t*)byteStream)->storage);
}

static sysbvm_tuple_t sysbvm_byteStream_primitive_nextPut(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_byteStream_nextPut(context, arguments[0], sysbvm_tuple_char8_decode(arguments[1]));
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_byteStream_primitive_nextPutAll(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_byteStream_nextPutByteArray(context, arguments[0], arguments[1]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_byteStream_primitive_nextPutString(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_byteStream_nextPutString(context, arguments[0], arguments[1]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_byteStream_primitive_asByteArray(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_byteStream_asByteArray(context, arguments[0]);
}

static sysbvm_tuple_t sysbvm_byteStream_primitive_asString(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_byteStream_asString(context, arguments[0]);
}

static sysbvm_tuple_t sysbvm_byteStream_primitive_asSymbol(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_byteStream_asSymbol(context, arguments[0]);
}

void sysbvm_byteStream_registerPrimitives(void)
{
    sysbvm_primitiveTable_registerFunction(sysbvm_byteStream_primitive_nextPut, "ByteStream::nextPut:");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteStream_primitive_nextPutAll, "ByteStream::nextPutAll:");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteStream_primitive_nextPutString, "ByteStream::nextPutString:");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteStream_primitive_asByteArray, "ByteStream::asByteArray");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteStream_primitive_asString, "ByteStream::asString");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteStream_primitive_asSymbol, "ByteStream::asSymbol");
}

void sysbvm_byteStream_setupPrimitives(sysbvm_context_t *context)
{
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.byteStreamType, "nextPut:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_byteStream_primitive_nextPut);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.byteStreamType, "nextPutAll:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_byteStream_primitive_nextPutAll);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.byteStreamType, "nextPutString:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_byteStream_primitive_nextPutString);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.byteStreamType, "asByteArray", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteStream_primitive_asByteArray);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.byteStreamType, "asString", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteStream_primitive_asString);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.byteStreamType, "asSymbol", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteStream_primitive_asSymbol);
}
