#include "tuuvm/tuple.h"
#include "tuuvm/type.h"
#include "tuuvm/context.h"
#include "tuuvm/errors.h"
#include "tuuvm/function.h"
#include "tuuvm/string.h"
#include "internal/context.h"
#include <stdlib.h>
#include <string.h>

TUUVM_API tuuvm_tuple_t tuuvm_tuple_getImmediateTypeWithTag(tuuvm_context_t *context, size_t immediateTypeTag)
{
    return immediateTypeTag < TUUVM_TUPLE_TAG_COUNT ? context->roots.immediateTypeTable[immediateTypeTag] : 0;
}

TUUVM_API tuuvm_tuple_t tuuvm_tuple_getImmediateTrivialTypeWithIndex(tuuvm_context_t *context, size_t immediateTrivialIndex)
{
    return immediateTrivialIndex < TUUVM_TUPLE_IMMEDIATE_TRIVIAL_COUNT ? context->roots.immediateTrivialTypeTable[immediateTrivialIndex] : 0;
}

TUUVM_API tuuvm_tuple_t tuuvm_tuple_char32_encodeBig(tuuvm_context_t *context, tuuvm_char32_t value)
{
    tuuvm_object_tuple_t *result = tuuvm_context_allocateByteTuple(context, context->roots.char32Type, sizeof(tuuvm_char32_t));
    *((tuuvm_char32_t*)result->bytes) = value;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_tuple_uint32_encodeBig(tuuvm_context_t *context, uint32_t value)
{
    tuuvm_object_tuple_t *result = tuuvm_context_allocateByteTuple(context, context->roots.uint32Type, sizeof(uint32_t));
    *((uint32_t*)result->bytes) = value;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_tuple_int32_encodeBig(tuuvm_context_t *context, int32_t value)
{
    tuuvm_object_tuple_t *result = tuuvm_context_allocateByteTuple(context, context->roots.int32Type, sizeof(int32_t));
    *((int32_t*)result->bytes) = value;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_tuple_uint64_encodeBig(tuuvm_context_t *context, uint64_t value)
{
    tuuvm_object_tuple_t *result = tuuvm_context_allocateByteTuple(context, context->roots.uint64Type, sizeof(uint64_t));
    *((uint64_t*)result->bytes) = value;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_tuple_int64_encodeBig(tuuvm_context_t *context, int64_t value)
{
    tuuvm_object_tuple_t *result = tuuvm_context_allocateByteTuple(context, context->roots.int64Type, sizeof(int64_t));
    *((int64_t*)result->bytes) = value;
    return (tuuvm_tuple_t)result;
}

TUUVM_API size_t tuuvm_tuple_hash(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    tuuvm_tuple_t type = tuuvm_tuple_getType(context, tuple);
    if(type)
    {
        tuuvm_tuple_t hashFunction = tuuvm_type_getHashFunction(context, type);
        if(hashFunction)
            return tuuvm_tuple_size_decode(tuuvm_function_apply1(context, hashFunction, tuple));
    }

    return tuuvm_tuple_identityHash(tuple);
}

TUUVM_API bool tuuvm_tuple_equals(tuuvm_context_t *context, tuuvm_tuple_t a, tuuvm_tuple_t b)
{
    tuuvm_tuple_t type = tuuvm_tuple_getType(context, a);
    if(type)
    {
        tuuvm_tuple_t equalsFunction = tuuvm_type_getEqualsFunction(context, type);
        if(equalsFunction)
            return tuuvm_tuple_boolean_decode(tuuvm_function_apply2(context, equalsFunction, a, b));
    }

    return tuuvm_tuple_identityEquals(a, b);
}

TUUVM_API tuuvm_tuple_t tuuvm_tuple_primitive_identityHash(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    (void)argumentCount;
    return tuuvm_tuple_size_encode(context, tuuvm_tuple_identityHash(arguments[0]));
}

TUUVM_API tuuvm_tuple_t tuuvm_tuple_primitive_identityEquals(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)argumentCount;
    return tuuvm_tuple_boolean_encode(tuuvm_tuple_identityEquals(arguments[0], arguments[1]));
}

TUUVM_API tuuvm_tuple_t tuuvm_tuple_primitive_identityNotEquals(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)argumentCount;
    return tuuvm_tuple_boolean_encode(tuuvm_tuple_identityNotEquals(arguments[0], arguments[1]));
}

TUUVM_API char *tuuvm_tuple_bytesToCString(tuuvm_tuple_t tuple)
{
    size_t stringSize = tuuvm_tuple_getSizeInBytes(tuple);
    size_t sizeToAllocate = stringSize + 1;
    char *cstring = (char*)malloc(sizeToAllocate);
    memcpy(cstring, TUUVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->bytes, sizeToAllocate);
    cstring[stringSize] = 0;
    return cstring;
}

TUUVM_API void tuuvm_tuple_bytesToCStringFree(char *cstring)
{
    free(cstring);
}

static tuuvm_tuple_t tuuvm_tuple_primitive_getType(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_getType(context, arguments[0]);
}

static tuuvm_tuple_t tuuvm_tuple_primitive_setType(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    if(tuuvm_tuple_isNonNullPointer(arguments[0]))
        tuuvm_tuple_setType((tuuvm_object_tuple_t*)arguments[0], arguments[1]);
    return TUUVM_VOID_TUPLE;
}

TUUVM_API tuuvm_tuple_t tuuvm_tuple_slotAt(tuuvm_context_t *context, tuuvm_tuple_t tuple, size_t slotIndex)
{
    (void)context;
    if(tuuvm_tuple_isDummyValue(tuple)) tuuvm_error_accessDummyValue();

    if(!tuuvm_tuple_isNonNullPointer(tuple))
    {
        if(slotIndex < sizeof(tuuvm_tuple_t))
        {
            tuuvm_tuple_t tag = tuple & TUUVM_TUPLE_TAG_BIT_MASK;
            if(TUUVM_TUPLE_TAG_SIGNED_START <= tag && tag <= TUUVM_TUPLE_TAG_SIGNED_END)
            {
                tuuvm_tuple_t byteValue = ( ((tuuvm_stuple_t)tuple) >> (TUUVM_TUPLE_TAG_BIT_COUNT + slotIndex*8)) & 0xFF;
                return tuuvm_tuple_uint8_encode((uint8_t)byteValue);
            }
            else
            {
                tuuvm_tuple_t byteValue = (tuple >> (TUUVM_TUPLE_TAG_BIT_COUNT + slotIndex*8)) & 0xFF;
                return tuuvm_tuple_uint8_encode((uint8_t)byteValue);
            }
        }

        return TUUVM_NULL_TUPLE;
    }

    if(tuuvm_tuple_isBytes(tuple))
    {
        if(slotIndex < tuuvm_tuple_getSizeInBytes(tuple))
            return tuuvm_tuple_uint8_encode(TUUVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->bytes[slotIndex]);
    }
    else
    {
        if(slotIndex < tuuvm_tuple_getSizeInSlots(tuple))
            return TUUVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->pointers[slotIndex];
    }

    return TUUVM_NULL_TUPLE;
}

static tuuvm_tuple_t tuuvm_tuple_primitive_slotAt(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return tuuvm_tuple_slotAt(context, arguments[0], tuuvm_tuple_anySize_decode(arguments[1]));
}

TUUVM_API void tuuvm_tuple_slotAtPut(tuuvm_context_t *context, tuuvm_tuple_t tuple, size_t slotIndex, tuuvm_tuple_t value)
{
    (void)context;
    if(!tuuvm_tuple_isNonNullPointer(tuple)) tuuvm_error_modifyImmediateValue();
    if(tuuvm_tuple_isDummyValue(tuple)) tuuvm_error_accessDummyValue();
    if(tuuvm_tuple_isImmediate(tuple)) tuuvm_error_modifyImmutableTuple();

    if(tuuvm_tuple_isBytes(tuple))
    {
        if(slotIndex < tuuvm_tuple_getSizeInBytes(tuple))
            TUUVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->bytes[slotIndex] = tuuvm_tuple_anySize_decode(value) & 0xFF;
    }
    else
    {
        if(slotIndex < tuuvm_tuple_getSizeInSlots(tuple))
            TUUVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->pointers[slotIndex] = value;
    }
}

bool tuuvm_tuple_isKindOf(tuuvm_context_t *context, tuuvm_tuple_t tuple, tuuvm_tuple_t type)
{
    tuuvm_tuple_t tupleType = tuuvm_tuple_getType(context, tuple);
    return tupleType == type || tuuvm_type_isSubtypeOf(tuuvm_tuple_getType(context, tuple), type);
}

static tuuvm_tuple_t tuuvm_tuple_primitive_slotAtPut(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) tuuvm_error_argumentCountMismatch(3, argumentCount);

    tuuvm_tuple_slotAtPut(context, arguments[0], tuuvm_tuple_anySize_decode(arguments[1]), arguments[2]);
    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_tuple_primitive_new(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return (tuuvm_tuple_t)tuuvm_context_allocatePointerTuple(context, TUUVM_NULL_TUPLE, tuuvm_tuple_anySize_decode(arguments[0]));
}

static tuuvm_tuple_t tuuvm_tuple_primitive_byteNew(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return (tuuvm_tuple_t)tuuvm_context_allocateByteTuple(context, TUUVM_NULL_TUPLE, tuuvm_tuple_anySize_decode(arguments[0]));
}

static tuuvm_tuple_t tuuvm_tuple_primitive_size(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_size_encode(context, tuuvm_tuple_getSizeInSlots(arguments[0]));
}

static tuuvm_tuple_t tuuvm_tuple_primitive_byteSize(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_size_encode(context, tuuvm_tuple_getSizeInBytes(arguments[0]));
}

static tuuvm_tuple_t tuuvm_tuple_primitive_shallowCopy(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_context_shallowCopy(context, arguments[0]);
}

void tuuvm_tuple_registerPrimitives(void)
{
    tuuvm_primitiveTable_registerFunction(tuuvm_tuple_primitive_getType, "RawTuple::type");
    tuuvm_primitiveTable_registerFunction(tuuvm_tuple_primitive_setType, "RawTuple::type:");
    tuuvm_primitiveTable_registerFunction(tuuvm_tuple_primitive_slotAt, "RawTuple::slotAt:");
    tuuvm_primitiveTable_registerFunction(tuuvm_tuple_primitive_slotAtPut, "RawTuple::slotAt:put:");
    tuuvm_primitiveTable_registerFunction(tuuvm_tuple_primitive_new, "RawTuple::new");
    tuuvm_primitiveTable_registerFunction(tuuvm_tuple_primitive_byteNew, "RawTuple::byteNew");
    tuuvm_primitiveTable_registerFunction(tuuvm_tuple_primitive_size, "RawTuple::size");
    tuuvm_primitiveTable_registerFunction(tuuvm_tuple_primitive_byteSize, "RawTuple::byteSize");
    tuuvm_primitiveTable_registerFunction(tuuvm_tuple_primitive_shallowCopy, "RawTuple::shallowCopy");
}

void tuuvm_tuple_setupPrimitives(tuuvm_context_t *context)
{
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::type", context->roots.anyValueType, "__type__", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_tuple_primitive_getType);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::type:", context->roots.anyValueType, "__type__:", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_tuple_primitive_setType);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::slotAt:", context->roots.anyValueType, "__slotAt__:", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_tuple_primitive_slotAt);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::slotAt:put:", context->roots.anyValueType, "__slotAt__:put:", 3, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_tuple_primitive_slotAtPut);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "RawTuple::new", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_tuple_primitive_new);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "RawTuple::byteNew", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_tuple_primitive_byteNew);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::size", context->roots.anyValueType, "__size__", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_tuple_primitive_size);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::byteSize", context->roots.anyValueType, "__byteSize__", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_tuple_primitive_byteSize);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::shallowCopy", context->roots.anyValueType, "__shallowCopy__", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_tuple_primitive_shallowCopy);
}
