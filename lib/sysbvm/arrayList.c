#include "sysbvm/arrayList.h"
#include "sysbvm/array.h"
#include "sysbvm/arraySlice.h"
#include "sysbvm/errors.h"
#include "sysbvm/function.h"
#include "sysbvm/type.h"
#include "internal/context.h"

SYSBVM_API sysbvm_tuple_t sysbvm_arrayList_create(sysbvm_context_t *context)
{
    sysbvm_arrayList_t *result = (sysbvm_arrayList_t*)sysbvm_context_allocatePointerTuple(context, context->roots.arrayListType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_arrayList_t));
    result->size = sysbvm_tuple_size_encode(context, 0);
    return (sysbvm_tuple_t)result;
}

static void sysbvm_arrayList_increaseCapacity(sysbvm_context_t *context, sysbvm_tuple_t arrayList)
{
    sysbvm_arrayList_t *arrayListObject = (sysbvm_arrayList_t*)arrayList;
    size_t size = sysbvm_tuple_size_decode(arrayListObject->size);
    size_t oldCapacity = sysbvm_tuple_getSizeInSlots(arrayListObject->storage);
    size_t newCapacity = oldCapacity * 2;
    if(newCapacity < 4)
        newCapacity = 4;

    bool isWeakArrayList = sysbvm_tuple_getType(context, arrayList) == context->roots.weakArrayListType;
    sysbvm_array_t *newStorage = isWeakArrayList
        ? (sysbvm_array_t*)sysbvm_weakArray_create(context, newCapacity)
        : (sysbvm_array_t*)sysbvm_array_create(context, newCapacity);
    sysbvm_array_t *oldStorage = (sysbvm_array_t*)arrayListObject->storage;
    for(size_t i = 0; i < size; ++i)
        newStorage->elements[i] = oldStorage->elements[i];
    arrayListObject->storage = (sysbvm_tuple_t)newStorage;
}

SYSBVM_API void sysbvm_arrayList_add(sysbvm_context_t *context, sysbvm_tuple_t arrayList, sysbvm_tuple_t element)
{
    if(!sysbvm_tuple_isNonNullPointer(arrayList))
        return;

    sysbvm_arrayList_t *arrayListObject = (sysbvm_arrayList_t*)arrayList;
    size_t size = sysbvm_tuple_size_decode(arrayListObject->size);
    size_t capacity = sysbvm_tuple_getSizeInSlots(arrayListObject->storage);
    if(size >= capacity)
        sysbvm_arrayList_increaseCapacity(context, arrayList);

    sysbvm_array_t *storage = (sysbvm_array_t*)arrayListObject->storage;
    storage->elements[size] = element;
    arrayListObject->size = sysbvm_tuple_size_encode(context, size + 1);
}

SYSBVM_API sysbvm_tuple_t sysbvm_arrayList_asArraySlice(sysbvm_context_t *context, sysbvm_tuple_t arrayList)
{
    if(!sysbvm_tuple_isNonNullPointer(arrayList))
        return SYSBVM_NULL_TUPLE;

    sysbvm_arrayList_t *arrayListObject = (sysbvm_arrayList_t*)arrayList;
    return sysbvm_arraySlice_create(context, arrayListObject->storage, sysbvm_tuple_size_encode(context, 0), arrayListObject->size);
}

SYSBVM_API sysbvm_tuple_t sysbvm_arrayList_asArray(sysbvm_context_t *context, sysbvm_tuple_t arrayList)
{
    if(!sysbvm_tuple_isNonNullPointer(arrayList))
        return SYSBVM_NULL_TUPLE;

    sysbvm_arrayList_t *arrayListObject = (sysbvm_arrayList_t*)arrayList;
    if(!arrayListObject->storage)
        return sysbvm_array_create(context, 0);

    return sysbvm_array_getFirstElements(context, arrayListObject->storage, sysbvm_tuple_size_decode(arrayListObject->size));
}

SYSBVM_API size_t sysbvm_arrayList_getSize(sysbvm_tuple_t arrayList)
{
    if(!sysbvm_tuple_isNonNullPointer(arrayList)) return 0;
    return sysbvm_tuple_size_decode(((sysbvm_arrayList_t*)arrayList)->size);
}

SYSBVM_API size_t sysbvm_arrayList_getCapacity(sysbvm_tuple_t arrayList)
{
    if(!sysbvm_tuple_isNonNullPointer(arrayList)) return 0;
    return sysbvm_tuple_getSizeInSlots(((sysbvm_arrayList_t*)arrayList)->storage);
}

SYSBVM_API sysbvm_tuple_t sysbvm_arrayList_at(sysbvm_tuple_t arrayList, size_t index)
{
    if(!sysbvm_tuple_isNonNullPointer(arrayList)) return SYSBVM_NULL_TUPLE;

    sysbvm_arrayList_t *arrayListObject = (sysbvm_arrayList_t*)arrayList;
    size_t size = sysbvm_tuple_size_decode(arrayListObject->size);
    if(index >= size)
    {
        sysbvm_error_indexOutOfBounds();
        return SYSBVM_NULL_TUPLE;
    }

    return ((sysbvm_array_t*)arrayListObject->storage)->elements[index];
}

static sysbvm_tuple_t sysbvm_arrayList_primitive_add(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_arrayList_add(context, arguments[0], arguments[1]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_arrayList_primitive_asArray(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_arrayList_asArray(context, arguments[0]);
}

void sysbvm_arrayList_registerPrimitives(void)
{
    sysbvm_primitiveTable_registerFunction(sysbvm_arrayList_primitive_add, "ArrayList::add:");
    sysbvm_primitiveTable_registerFunction(sysbvm_arrayList_primitive_asArray, "ArrayList::asArray");
}

void sysbvm_arrayList_setupPrimitives(sysbvm_context_t *context)
{
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "ArrayList::add:", context->roots.arrayListType, "add:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_arrayList_primitive_add);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "ArrayList::asArray", context->roots.arrayListType, "asArray", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_arrayList_primitive_asArray);
}
