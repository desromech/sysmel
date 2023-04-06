#include "sysbvm/array.h"
#include "sysbvm/arraySlice.h"
#include "sysbvm/function.h"
#include "sysbvm/errors.h"
#include "internal/context.h"

SYSBVM_API sysbvm_tuple_t sysbvm_array_create(sysbvm_context_t *context, sysbvm_tuple_t slotCount)
{
    if(slotCount == 0)
    {
        if(!context->roots.emptyArrayConstant)
            context->roots.emptyArrayConstant = (sysbvm_tuple_t)sysbvm_context_allocatePointerTuple(context, context->roots.arrayType, 0);
        return context->roots.emptyArrayConstant;
    }

    return (sysbvm_tuple_t)sysbvm_context_allocatePointerTuple(context, context->roots.arrayType, slotCount);
}

SYSBVM_API sysbvm_tuple_t sysbvm_weakArray_create(sysbvm_context_t *context, sysbvm_tuple_t slotCount)
{
    if(slotCount == 0)
    {
        if(!context->roots.emptyWeakArrayConstant)
        {
            context->roots.emptyWeakArrayConstant = (sysbvm_tuple_t)sysbvm_context_allocatePointerTuple(context, context->roots.weakArrayType, 0);
            sysbvm_tuple_markWeakObject(context->roots.emptyWeakArrayConstant);
        }
        return context->roots.emptyWeakArrayConstant;
    }

    sysbvm_tuple_t result = (sysbvm_tuple_t)sysbvm_context_allocatePointerTuple(context, context->roots.weakArrayType, slotCount);
    sysbvm_tuple_markWeakObject(result);
    return result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_byteArray_create(sysbvm_context_t *context, sysbvm_tuple_t size)
{
    if(size == 0)
    {
        if(!context->roots.emptyByteArrayConstant)
            context->roots.emptyByteArrayConstant = (sysbvm_tuple_t)sysbvm_context_allocateByteTuple(context, context->roots.byteArrayType, 0);
        return context->roots.emptyByteArrayConstant;
    }

    return (sysbvm_tuple_t)sysbvm_context_allocateByteTuple(context, context->roots.byteArrayType, size);
}

SYSBVM_API sysbvm_tuple_t sysbvm_array_asArraySlice(sysbvm_context_t *context, sysbvm_tuple_t array)
{
    if(!sysbvm_tuple_isNonNullPointer(array)) return SYSBVM_NULL_TUPLE;

    return sysbvm_arraySlice_createWithOffsetAndSize(context, array, 0, sysbvm_tuple_getSizeInSlots(array));
}

SYSBVM_API sysbvm_tuple_t sysbvm_array_getFirstElements(sysbvm_context_t *context, sysbvm_tuple_t array, size_t size)
{
    if(!sysbvm_tuple_isNonNullPointer(array)) return SYSBVM_NULL_TUPLE;

    size_t resultSize = sysbvm_tuple_getSizeInSlots(array);
    if(size < resultSize)
        resultSize = size;

    sysbvm_array_t *source = (sysbvm_array_t*)array;
    sysbvm_array_t *result = (sysbvm_array_t*)sysbvm_array_create(context, resultSize);
    for(size_t i = 0; i < resultSize; ++i)
        result->elements[i] = source->elements[i];

    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_arrayOrByteArray_at(sysbvm_tuple_t array, size_t index)
{
    if(!sysbvm_tuple_isNonNullPointer(array)) return SYSBVM_NULL_TUPLE;

    if(sysbvm_tuple_isBytes(array))
    {
        size_t size = sysbvm_tuple_getSizeInBytes(array);
        if(index >= size) sysbvm_error_indexOutOfBounds();
        return sysbvm_tuple_uint8_encode(SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(array)->bytes[index]);
    }
    else
    {
        size_t size = sysbvm_tuple_getSizeInSlots(array);
        if(index >= size) sysbvm_error_indexOutOfBounds();
        return SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(array)->pointers[index];
    }
}

SYSBVM_API void sysbvm_arrayOrByteArray_atPut(sysbvm_tuple_t array, size_t index, sysbvm_tuple_t value)
{
    if(!sysbvm_tuple_isNonNullPointer(array))
        return;

    if(sysbvm_tuple_isBytes(array))
    {
        size_t size = sysbvm_tuple_getSizeInBytes(array);
        if(index >= size)
            sysbvm_error_indexOutOfBounds();
        SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(array)->bytes[index] = sysbvm_tuple_uint8_decode(value);
    }
    else
    {
        size_t size = sysbvm_tuple_getSizeInSlots(array);
        if(index >= size)
            sysbvm_error_indexOutOfBounds();
        SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(array)->pointers[index] = value;
    }
}

SYSBVM_API sysbvm_tuple_t sysbvm_array_fromOffset(sysbvm_context_t *context, sysbvm_tuple_t array, size_t fromOffset)
{
    if(!sysbvm_tuple_isNonNullPointer(array)) return SYSBVM_NULL_TUPLE;
    
    size_t size = sysbvm_tuple_getSizeInSlots(array);
    if(fromOffset >= size)
        return sysbvm_array_create(context, 0);

    size_t resultSize = size - fromOffset;
    sysbvm_tuple_t result = sysbvm_array_create(context, resultSize);
    sysbvm_tuple_t *sourceElements = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(array)->pointers;
    sysbvm_tuple_t *resultElements = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(result)->pointers;
    for(size_t i = 0; i < resultSize; ++i)
        resultElements[i] = sourceElements[fromOffset + i];

    return result;
}

static sysbvm_tuple_t sysbvm_array_primitive_equals(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);
    
    sysbvm_array_t **left = (sysbvm_array_t **)&arguments[0];
    sysbvm_array_t **right = (sysbvm_array_t **)&arguments[1];
    if(*left == *right)
        return SYSBVM_TRUE_TUPLE;
    if(sysbvm_tuple_getType(context, (sysbvm_tuple_t)*left) != sysbvm_tuple_getType(context, (sysbvm_tuple_t)*right))
        return SYSBVM_FALSE_TUPLE;

    size_t leftSize = sysbvm_array_getSize((sysbvm_tuple_t)*left);
    size_t rightSize = sysbvm_array_getSize((sysbvm_tuple_t)*right);
    if(leftSize != rightSize) return SYSBVM_FALSE_TUPLE;

    for(size_t i = 0; i < leftSize; ++i)
    {
        if(!sysbvm_tuple_equals(context, (*left)->elements[i], (*right)->elements[i]))
            return SYSBVM_FALSE_TUPLE;
    }

    return SYSBVM_TRUE_TUPLE;
}

static sysbvm_tuple_t sysbvm_array_primitive_hash(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_array_t **array = (sysbvm_array_t **)&arguments[0];
    size_t size = sysbvm_array_getSize((sysbvm_tuple_t)*array);

    size_t result = sysbvm_tuple_identityHash(sysbvm_tuple_getType(context, (sysbvm_tuple_t)*array));
    for(size_t i = 0; i < size; ++i)
        result = sysbvm_hashConcatenate(result, sysbvm_tuple_hash(context, (*array)->elements[i]));

    return sysbvm_tuple_size_encode(context, result);
}

void sysbvm_array_registerPrimitives(void)
{
    sysbvm_primitiveTable_registerFunction(sysbvm_array_primitive_equals, "Array::=");
    sysbvm_primitiveTable_registerFunction(sysbvm_array_primitive_hash, "Array::hash");
}

void sysbvm_array_setupPrimitives(sysbvm_context_t *context)
{
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Array::=", context->roots.arrayType, "=", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_array_primitive_equals);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Array::hash", context->roots.arrayType, "hash", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_array_primitive_hash);

    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "WeakArray::=", context->roots.weakArrayType, "=", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_array_primitive_equals);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "WeakArray::hash", context->roots.weakArrayType, "hash", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_array_primitive_hash);
}
