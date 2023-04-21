#include "sysbvm/orderedCollection.h"
#include "sysbvm/array.h"
#include "sysbvm/arraySlice.h"
#include "sysbvm/errors.h"
#include "sysbvm/function.h"
#include "sysbvm/type.h"
#include "internal/context.h"

SYSBVM_API sysbvm_tuple_t sysbvm_orderedCollection_create(sysbvm_context_t *context)
{
    sysbvm_orderedCollection_t *result = (sysbvm_orderedCollection_t*)sysbvm_context_allocatePointerTuple(context, context->roots.orderedCollectionType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_orderedCollection_t));
    result->size = sysbvm_tuple_size_encode(context, 0);
    return (sysbvm_tuple_t)result;
}

static void sysbvm_orderedCollection_increaseCapacity(sysbvm_context_t *context, sysbvm_tuple_t orderedCollection)
{
    sysbvm_orderedCollection_t *orderedCollectionObject = (sysbvm_orderedCollection_t*)orderedCollection;
    size_t size = sysbvm_tuple_size_decode(orderedCollectionObject->size);
    size_t oldCapacity = sysbvm_tuple_getSizeInSlots(orderedCollectionObject->storage);
    size_t newCapacity = oldCapacity * 2;
    if(newCapacity < 4)
        newCapacity = 4;

    bool isWeakOrderedCollection = sysbvm_tuple_getType(context, orderedCollection) == context->roots.weakOrderedCollectionType;
    sysbvm_array_t *newStorage = isWeakOrderedCollection
        ? (sysbvm_array_t*)sysbvm_weakArray_create(context, newCapacity)
        : (sysbvm_array_t*)sysbvm_array_create(context, newCapacity);
    sysbvm_array_t *oldStorage = (sysbvm_array_t*)orderedCollectionObject->storage;
    for(size_t i = 0; i < size; ++i)
        newStorage->elements[i] = oldStorage->elements[i];
    orderedCollectionObject->storage = (sysbvm_tuple_t)newStorage;
}

SYSBVM_API void sysbvm_orderedCollection_add(sysbvm_context_t *context, sysbvm_tuple_t orderedCollection, sysbvm_tuple_t element)
{
    if(!sysbvm_tuple_isNonNullPointer(orderedCollection))
        return;

    sysbvm_orderedCollection_t *orderedCollectionObject = (sysbvm_orderedCollection_t*)orderedCollection;
    size_t size = sysbvm_tuple_size_decode(orderedCollectionObject->size);
    size_t capacity = sysbvm_tuple_getSizeInSlots(orderedCollectionObject->storage);
    if(size >= capacity)
        sysbvm_orderedCollection_increaseCapacity(context, orderedCollection);

    sysbvm_array_t *storage = (sysbvm_array_t*)orderedCollectionObject->storage;
    storage->elements[size] = element;
    orderedCollectionObject->size = sysbvm_tuple_size_encode(context, size + 1);
}

SYSBVM_API sysbvm_tuple_t sysbvm_orderedCollection_asArraySlice(sysbvm_context_t *context, sysbvm_tuple_t orderedCollection)
{
    if(!sysbvm_tuple_isNonNullPointer(orderedCollection))
        return SYSBVM_NULL_TUPLE;

    sysbvm_orderedCollection_t *orderedCollectionObject = (sysbvm_orderedCollection_t*)orderedCollection;
    return sysbvm_arraySlice_create(context, orderedCollectionObject->storage, sysbvm_tuple_size_encode(context, 0), orderedCollectionObject->size);
}

SYSBVM_API sysbvm_tuple_t sysbvm_orderedCollection_asArray(sysbvm_context_t *context, sysbvm_tuple_t orderedCollection)
{
    if(!sysbvm_tuple_isNonNullPointer(orderedCollection))
        return SYSBVM_NULL_TUPLE;

    sysbvm_orderedCollection_t *orderedCollectionObject = (sysbvm_orderedCollection_t*)orderedCollection;
    if(!orderedCollectionObject->storage)
        return sysbvm_array_create(context, 0);

    return sysbvm_array_getFirstElements(context, orderedCollectionObject->storage, sysbvm_tuple_size_decode(orderedCollectionObject->size));
}

SYSBVM_API size_t sysbvm_orderedCollection_getSize(sysbvm_tuple_t orderedCollection)
{
    if(!sysbvm_tuple_isNonNullPointer(orderedCollection)) return 0;
    return sysbvm_tuple_size_decode(((sysbvm_orderedCollection_t*)orderedCollection)->size);
}

SYSBVM_API size_t sysbvm_orderedCollection_getCapacity(sysbvm_tuple_t orderedCollection)
{
    if(!sysbvm_tuple_isNonNullPointer(orderedCollection)) return 0;
    return sysbvm_tuple_getSizeInSlots(((sysbvm_orderedCollection_t*)orderedCollection)->storage);
}

SYSBVM_API sysbvm_tuple_t sysbvm_orderedCollection_at(sysbvm_tuple_t orderedCollection, size_t index)
{
    if(!sysbvm_tuple_isNonNullPointer(orderedCollection)) return SYSBVM_NULL_TUPLE;

    sysbvm_orderedCollection_t *orderedCollectionObject = (sysbvm_orderedCollection_t*)orderedCollection;
    size_t size = sysbvm_tuple_size_decode(orderedCollectionObject->size);
    if(index >= size)
    {
        sysbvm_error_indexOutOfBounds();
        return SYSBVM_NULL_TUPLE;
    }

    return ((sysbvm_array_t*)orderedCollectionObject->storage)->elements[index];
}

static sysbvm_tuple_t sysbvm_orderedCollection_primitive_add(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_orderedCollection_add(context, arguments[0], arguments[1]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_orderedCollection_primitive_asArray(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_orderedCollection_asArray(context, arguments[0]);
}

void sysbvm_orderedCollection_registerPrimitives(void)
{
    sysbvm_primitiveTable_registerFunction(sysbvm_orderedCollection_primitive_add, "OrderedCollection::add:");
    sysbvm_primitiveTable_registerFunction(sysbvm_orderedCollection_primitive_asArray, "OrderedCollection::asArray");
}

void sysbvm_orderedCollection_setupPrimitives(sysbvm_context_t *context)
{
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "OrderedCollection::add:", context->roots.orderedCollectionType, "add:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_orderedCollection_primitive_add);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "OrderedCollection::asArray", context->roots.orderedCollectionType, "asArray", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_orderedCollection_primitive_asArray);
}
