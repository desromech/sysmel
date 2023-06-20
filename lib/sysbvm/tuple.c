#include "sysbvm/tuple.h"
#include "sysbvm/type.h"
#include "sysbvm/context.h"
#include "sysbvm/errors.h"
#include "sysbvm/function.h"
#include "sysbvm/string.h"
#include "internal/context.h"
#include "internal/heap.h"
#include <stdlib.h>
#include <string.h>

SYSBVM_API sysbvm_tuple_t sysbvm_tuple_getImmediateTypeWithTag(sysbvm_context_t *context, size_t immediateTypeTag)
{
    return immediateTypeTag < SYSBVM_TUPLE_TAG_COUNT ? context->roots.immediateTypeTable[immediateTypeTag] : 0;
}

SYSBVM_API sysbvm_tuple_t sysbvm_tuple_getImmediateTrivialTypeWithIndex(sysbvm_context_t *context, size_t immediateTrivialIndex)
{
    return immediateTrivialIndex < SYSBVM_TUPLE_IMMEDIATE_TRIVIAL_COUNT ? context->roots.immediateTrivialTypeTable[immediateTrivialIndex] : 0;
}

SYSBVM_API sysbvm_tuple_t sysbvm_tuple_char32_encodeBig(sysbvm_context_t *context, sysbvm_char32_t value)
{
    sysbvm_object_tuple_t *result = sysbvm_context_allocateByteTuple(context, context->roots.char32Type, sizeof(sysbvm_char32_t));
    *((sysbvm_char32_t*)result->bytes) = value;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_tuple_uint32_encodeBig(sysbvm_context_t *context, uint32_t value)
{
    sysbvm_object_tuple_t *result = sysbvm_context_allocateByteTuple(context, context->roots.uint32Type, sizeof(uint32_t));
    *((uint32_t*)result->bytes) = value;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_tuple_int32_encodeBig(sysbvm_context_t *context, int32_t value)
{
    sysbvm_object_tuple_t *result = sysbvm_context_allocateByteTuple(context, context->roots.int32Type, sizeof(int32_t));
    *((int32_t*)result->bytes) = value;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_tuple_uint64_encodeBig(sysbvm_context_t *context, uint64_t value)
{
    sysbvm_object_tuple_t *result = sysbvm_context_allocateByteTuple(context, context->roots.uint64Type, sizeof(uint64_t));
    *((uint64_t*)result->bytes) = value;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_tuple_int64_encodeBig(sysbvm_context_t *context, int64_t value)
{
    sysbvm_object_tuple_t *result = sysbvm_context_allocateByteTuple(context, context->roots.int64Type, sizeof(int64_t));
    *((int64_t*)result->bytes) = value;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API size_t sysbvm_tuple_hash(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    sysbvm_tuple_t type = sysbvm_tuple_getType(context, tuple);
    if(type)
    {
        sysbvm_tuple_t hashFunction = sysbvm_type_getHashFunction(context, type);
        if(hashFunction)
            return sysbvm_tuple_size_decode(sysbvm_function_apply1(context, hashFunction, tuple));
    }

    return sysbvm_tuple_identityHash(tuple);
}

SYSBVM_API bool sysbvm_tuple_equals(sysbvm_context_t *context, sysbvm_tuple_t a, sysbvm_tuple_t b)
{
    sysbvm_tuple_t type = sysbvm_tuple_getType(context, a);
    if(type)
    {
        sysbvm_tuple_t equalsFunction = sysbvm_type_getEqualsFunction(context, type);
        if(equalsFunction)
            return sysbvm_tuple_boolean_decode(sysbvm_function_apply2(context, equalsFunction, a, b));
    }

    return sysbvm_tuple_identityEquals(a, b);
}

SYSBVM_API sysbvm_tuple_t sysbvm_tuple_primitive_identityHash(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    (void)argumentCount;
    return sysbvm_tuple_size_encode(context, sysbvm_tuple_identityHash(arguments[0]));
}

SYSBVM_API sysbvm_tuple_t sysbvm_tuple_primitive_identityEquals(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)argumentCount;
    return sysbvm_tuple_boolean_encode(sysbvm_tuple_identityEquals(arguments[0], arguments[1]));
}

SYSBVM_API sysbvm_tuple_t sysbvm_tuple_primitive_identityNotEquals(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)argumentCount;
    return sysbvm_tuple_boolean_encode(sysbvm_tuple_identityNotEquals(arguments[0], arguments[1]));
}

SYSBVM_API char *sysbvm_tuple_bytesToCString(sysbvm_tuple_t tuple)
{
    size_t stringSize = sysbvm_tuple_getSizeInBytes(tuple);
    size_t sizeToAllocate = stringSize + 1;
    char *cstring = (char*)malloc(sizeToAllocate);
    memcpy(cstring, SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->bytes, sizeToAllocate);
    cstring[stringSize] = 0;
    return cstring;
}

SYSBVM_API void sysbvm_tuple_bytesToCStringFree(char *cstring)
{
    free(cstring);
}

static sysbvm_tuple_t sysbvm_tuple_primitive_getType(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_getType(context, arguments[0]);
}

static sysbvm_tuple_t sysbvm_tuple_primitive_setType(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);
    if(!sysbvm_tuple_isNonNullPointer(arguments[0])) sysbvm_error("Cannot set the type of an immediate value.");
    
    sysbvm_tuple_setType((sysbvm_object_tuple_t*)arguments[0], arguments[1]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_tuple_primitive_setIdentityHash(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);
    if(!sysbvm_tuple_isNonNullPointer(arguments[0])) sysbvm_error("Cannot set the identity hash of an immediate value.");

    sysbvm_tuple_setIdentityHash((sysbvm_object_tuple_t*)arguments[0], sysbvm_tuple_size_decode(arguments[1]));
    return SYSBVM_VOID_TUPLE;
}

SYSBVM_API sysbvm_tuple_t sysbvm_tuple_slotAt(sysbvm_context_t *context, sysbvm_tuple_t tuple, size_t slotIndex)
{
    (void)context;
    if(sysbvm_tuple_isDummyValue(tuple)) sysbvm_error_accessDummyValue();

    if(!sysbvm_tuple_isNonNullPointer(tuple))
    {
        if(slotIndex < sizeof(sysbvm_tuple_t))
        {
            sysbvm_tuple_t tag = tuple & SYSBVM_TUPLE_TAG_BIT_MASK;
            if(SYSBVM_TUPLE_TAG_SIGNED_START <= tag && tag <= SYSBVM_TUPLE_TAG_SIGNED_END)
            {
                sysbvm_tuple_t byteValue = ( ((sysbvm_stuple_t)tuple) >> (SYSBVM_TUPLE_TAG_BIT_COUNT + slotIndex*8)) & 0xFF;
                return sysbvm_tuple_uint8_encode((uint8_t)byteValue);
            }
            else
            {
                sysbvm_tuple_t byteValue = (tuple >> (SYSBVM_TUPLE_TAG_BIT_COUNT + slotIndex*8)) & 0xFF;
                return sysbvm_tuple_uint8_encode((uint8_t)byteValue);
            }
        }

        return SYSBVM_NULL_TUPLE;
    }

    if(sysbvm_tuple_isBytes(tuple))
    {
        if(slotIndex < sysbvm_tuple_getSizeInBytes(tuple))
            return sysbvm_tuple_uint8_encode(SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->bytes[slotIndex]);
    }
    else
    {
        if(slotIndex < sysbvm_tuple_getSizeInSlots(tuple))
            return SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->pointers[slotIndex];
    }

    sysbvm_error_outOfBoundsSlotAccess();
    return SYSBVM_NULL_TUPLE;
}

SYSBVM_API uint8_t sysbvm_tuple_byteSlotAt(sysbvm_context_t *context, sysbvm_tuple_t tuple, size_t slotIndex)
{
    (void)context;
    if(sysbvm_tuple_isDummyValue(tuple)) sysbvm_error_accessDummyValue();

    if(!sysbvm_tuple_isNonNullPointer(tuple))
    {
        if(slotIndex < sizeof(sysbvm_tuple_t))
        {
            sysbvm_tuple_t tag = tuple & SYSBVM_TUPLE_TAG_BIT_MASK;
            if(SYSBVM_TUPLE_TAG_SIGNED_START <= tag && tag <= SYSBVM_TUPLE_TAG_SIGNED_END)
            {
                sysbvm_tuple_t byteValue = ( ((sysbvm_stuple_t)tuple) >> (SYSBVM_TUPLE_TAG_BIT_COUNT + slotIndex*8)) & 0xFF;
                return (uint8_t)byteValue;
            }
            else
            {
                sysbvm_tuple_t byteValue = (tuple >> (SYSBVM_TUPLE_TAG_BIT_COUNT + slotIndex*8)) & 0xFF;
                return (uint8_t)byteValue;
            }
        }

        return 0;
    }

    if(!sysbvm_tuple_isBytes(tuple)) sysbvm_error("Expected a byte tuple.");

    if(slotIndex < sysbvm_tuple_getSizeInBytes(tuple))
        return SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->bytes[slotIndex];

    sysbvm_error_outOfBoundsSlotAccess();
    return 0;
}

static sysbvm_tuple_t sysbvm_tuple_primitive_slotAt(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    return sysbvm_tuple_slotAt(context, arguments[0], sysbvm_tuple_anySize_decode(arguments[1]));
}

static sysbvm_tuple_t sysbvm_tuple_primitive_byteSlotAt(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    return sysbvm_tuple_uint8_encode(sysbvm_tuple_byteSlotAt(context, arguments[0], sysbvm_tuple_anySize_decode(arguments[1])));
}

static sysbvm_tuple_t sysbvm_tuple_primitive_refSlotAt(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);
    if(!sysbvm_tuple_isNonNullPointer(arguments[0])) sysbvm_error_nullArgument();

    return sysbvm_tuple_slotAt(context, sysbvm_pointerLikeType_load(context, arguments[0]), sysbvm_tuple_anySize_decode(arguments[1]));
}

static sysbvm_tuple_t sysbvm_tuple_primitive_typeSlotAt(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);
    if(!sysbvm_tuple_isNonNullPointer(arguments[1])) sysbvm_error_nullArgument();

    return sysbvm_tuple_slotAt(context, arguments[0], sysbvm_typeSlot_getIndex(arguments[1]));
}

static sysbvm_tuple_t sysbvm_tuple_primitive_refTypeSlotAt(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);
    if(!sysbvm_tuple_isNonNullPointer(arguments[1])) sysbvm_error_nullArgument();

    return sysbvm_tuple_slotAt(context, sysbvm_pointerLikeType_load(context, arguments[0]), sysbvm_typeSlot_getIndex(arguments[1]));
}

SYSBVM_API void sysbvm_tuple_slotAtPut(sysbvm_context_t *context, sysbvm_tuple_t tuple, size_t slotIndex, sysbvm_tuple_t value)
{
    (void)context;
    if(!sysbvm_tuple_isNonNullPointer(tuple)) sysbvm_error_modifyImmediateValue();
    if(sysbvm_tuple_isDummyValue(tuple)) sysbvm_error_accessDummyValue();
    if(sysbvm_tuple_isImmediate(tuple)) sysbvm_error_modifyImmutableTuple();

    if(sysbvm_tuple_isBytes(tuple))
    {
        if(slotIndex < sysbvm_tuple_getSizeInBytes(tuple))
            SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->bytes[slotIndex] = sysbvm_tuple_anySize_decode(value) & 0xFF;
        else
            sysbvm_error_outOfBoundsSlotAccess();
    }
    else
    {
        if(slotIndex < sysbvm_tuple_getSizeInSlots(tuple))
            SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->pointers[slotIndex] = value;
        else
            sysbvm_error_outOfBoundsSlotAccess();
    }
}

SYSBVM_API void sysbvm_tuple_byteSlotAtPut(sysbvm_context_t *context, sysbvm_tuple_t tuple, size_t slotIndex, uint8_t value)
{
    (void)context;
    if(!sysbvm_tuple_isNonNullPointer(tuple)) sysbvm_error_modifyImmediateValue();
    if(sysbvm_tuple_isDummyValue(tuple)) sysbvm_error_accessDummyValue();
    if(sysbvm_tuple_isImmediate(tuple)) sysbvm_error_modifyImmutableTuple();

    if(!sysbvm_tuple_isBytes(tuple))
        sysbvm_error("Expected a byte tuple.");

    if(slotIndex < sysbvm_tuple_getSizeInBytes(tuple))
        SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->bytes[slotIndex] = value;
    else
        sysbvm_error_outOfBoundsSlotAccess();
}

bool sysbvm_tuple_isKindOf(sysbvm_context_t *context, sysbvm_tuple_t tuple, sysbvm_tuple_t type)
{
    sysbvm_tuple_t tupleType = sysbvm_tuple_getType(context, tuple);
    return tupleType == type
        || (!tupleType && type == context->roots.untypedType)
        || sysbvm_type_isDirectSubtypeOf(tupleType, type);
}

SYSBVM_API bool sysbvm_tuple_isTypeSatisfiedWithValue(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t value)
{
    if(!value && sysbvm_type_isNullable(type))
        return true;
    return sysbvm_tuple_isKindOf(context, value, type);
}

SYSBVM_API void sysbvm_tuple_typecheckValue(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t value)
{
    if(!sysbvm_tuple_isTypeSatisfiedWithValue(context, type, value))
        sysbvm_error_unexpectedType(type, value);
}

static sysbvm_tuple_t sysbvm_tuple_primitive_slotAtPut(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_tuple_slotAtPut(context, arguments[0], sysbvm_tuple_anySize_decode(arguments[1]), arguments[2]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_tuple_primitive_byteSlotAtPut(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_tuple_byteSlotAtPut(context, arguments[0], sysbvm_tuple_anySize_decode(arguments[1]), sysbvm_tuple_anySize_decode(arguments[2]));
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_tuple_primitive_refSlotAtPut(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);
    if(!sysbvm_tuple_isNonNullPointer(arguments[0])) sysbvm_error_nullArgument();

    sysbvm_tuple_slotAtPut(context, sysbvm_pointerLikeType_load(context, arguments[0]), sysbvm_tuple_anySize_decode(arguments[1]), arguments[2]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_tuple_primitive_typeSlotAtPut(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);
    if(!sysbvm_tuple_isNonNullPointer(arguments[1])) sysbvm_error_nullArgument();

    sysbvm_tuple_slotAtPut(context, arguments[0], sysbvm_typeSlot_getIndex(arguments[1]), arguments[2]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_tuple_primitive_refTypeSlotAtPut(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);
    if(!sysbvm_tuple_isNonNullPointer(arguments[1])) sysbvm_error_nullArgument();

    sysbvm_tuple_slotAtPut(context, sysbvm_pointerLikeType_load(context, arguments[0]), sysbvm_typeSlot_getIndex(arguments[1]), arguments[2]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_tuple_primitive_new(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return (sysbvm_tuple_t)sysbvm_context_allocatePointerTuple(context, SYSBVM_NULL_TUPLE, sysbvm_tuple_anySize_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_tuple_primitive_byteNew(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return (sysbvm_tuple_t)sysbvm_context_allocateByteTuple(context, SYSBVM_NULL_TUPLE, sysbvm_tuple_anySize_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_tuple_primitive_basicAllocateWithTypeInstanceSizeAlignmentSlotCountIsBytesIsWeak(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 7) sysbvm_error_argumentCountMismatch(7, argumentCount);

    sysbvm_tuple_t *type = arguments + 0;
    size_t instanceSize = sysbvm_tuple_anySize_decode(arguments[1]);
    //sysbvm_tuple_t *instanceAlignment = arguments + 2;
    size_t slotCount = sysbvm_tuple_anySize_decode(arguments[3]);
    size_t variableSize = sysbvm_tuple_anySize_decode(arguments[4]);
    bool isBytes = sysbvm_tuple_boolean_decode(arguments[5]);
    bool isWeak = sysbvm_tuple_boolean_decode(arguments[6]);

    sysbvm_tuple_t result = isBytes
        ? (sysbvm_tuple_t)sysbvm_context_allocateByteTuple(context, *type, instanceSize + variableSize)
        : (sysbvm_tuple_t)sysbvm_context_allocatePointerTuple(context, *type, slotCount + variableSize);

    if(isWeak)
        sysbvm_tuple_markWeakObject(result);
    
    return result;
}

static sysbvm_tuple_t sysbvm_tuple_primitive_size(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_size_encode(context, sysbvm_tuple_getSizeInSlots(arguments[0]));
}

static sysbvm_tuple_t sysbvm_tuple_primitive_byteSize(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_size_encode(context, sysbvm_tuple_getSizeInBytes(arguments[0]));
}

static sysbvm_tuple_t sysbvm_tuple_primitive_isBytes(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_boolean_encode(sysbvm_tuple_isBytes(arguments[0]));
}

static sysbvm_tuple_t sysbvm_tuple_primitive_isWeak(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_boolean_encode(sysbvm_tuple_isWeakObject(arguments[0]));
}

static sysbvm_tuple_t sysbvm_tuple_primitive_isDummyValue(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_boolean_encode(sysbvm_tuple_isDummyValue(arguments[0]));
}

static sysbvm_tuple_t sysbvm_tuple_primitive_shallowCopy(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_context_shallowCopy(context, arguments[0]);
}

static sysbvm_tuple_t sysbvm_tuple_primitive_markDummyValue(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_tuple_markDummyValue(arguments[0]);
    return arguments[0];
}

static sysbvm_tuple_t sysbvm_tuple_primitive_markWeak(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_tuple_markWeakObject(arguments[0]);
    return arguments[0];
}

static sysbvm_tuple_t sysbvm_tuple_primitive_firstInstanceWithType(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_heapIterator_t iterator = {0};
    sysbvm_heapIterator_begin(&context->heap, &iterator);
    if(!sysbvm_heapIterator_advanceUntilInstanceWithType(&iterator, arguments[0]))
        return SYSBVM_NULL_TUPLE;

    return (sysbvm_tuple_t)sysbvm_heapIterator_get(&iterator);
}

static sysbvm_tuple_t sysbvm_tuple_primitive_nextInstanceWithSameType(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_tuple_t *object = &arguments[0];
    sysbvm_tuple_t objectType = sysbvm_tuple_getType(context, *object);

    sysbvm_heapIterator_t iterator = {0};
    sysbvm_heapIterator_beginWithPointer(&context->heap, *object, &iterator);
    sysbvm_heapIterator_advance(&iterator);
    if(!sysbvm_heapIterator_advanceUntilInstanceWithType(&iterator, objectType))
        return SYSBVM_NULL_TUPLE;

    return (sysbvm_tuple_t)sysbvm_heapIterator_get(&iterator);
}

static sysbvm_tuple_t sysbvm_tuple_primitive_recordBindingWithOwnerAndName(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)arguments;
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_tuple_primitive_objectModel_isLogical(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)arguments;
    (void)context;
    (void)closure;
    if(argumentCount != 0) sysbvm_error_argumentCountMismatch(0, argumentCount);

    return SYSBVM_TRUE_TUPLE;
}

static sysbvm_tuple_t sysbvm_tuple_primitive_objectModel_isNative(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)arguments;
    (void)context;
    (void)closure;
    if(argumentCount != 0) sysbvm_error_argumentCountMismatch(0, argumentCount);

    return SYSBVM_FALSE_TUPLE;
}

void sysbvm_tuple_registerPrimitives(void)
{
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_getType, "RawTuple::type");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_setType, "RawTuple::type:");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_setIdentityHash, "RawTuple::identityHash:");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_slotAt, "RawTuple::slotAt:");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_slotAtPut, "RawTuple::slotAt:put:");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_byteSlotAt, "RawTuple::byteSlotAt:");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_byteSlotAtPut, "RawTuple::byteSlotAt:put:");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_refSlotAt, "RawTuple::refSlotAt:");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_refSlotAtPut, "RawTuple::refSlotAt:put:");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_typeSlotAt, "RawTuple::typeSlotAt:");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_typeSlotAtPut, "RawTuple::typeSlotAt:put:");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_refTypeSlotAt, "RawTuple::refTypeSlotAt:");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_refTypeSlotAtPut, "RawTuple::refTypeSlotAt:put:");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_new, "RawTuple::new");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_byteNew, "RawTuple::byteNew");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_basicAllocateWithTypeInstanceSizeAlignmentSlotCountIsBytesIsWeak, "RawTuple::basicAllocateWithType:instanceSize:alignment:slotCount:variableSize:isBytes:isWeak:");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_isBytes, "RawTuple::isBytes");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_isWeak, "RawTuple::isWeak");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_isDummyValue, "RawTuple::isDummyValue");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_size, "RawTuple::size");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_byteSize, "RawTuple::byteSize");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_shallowCopy, "RawTuple::shallowCopy");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_markWeak, "RawTuple::markWeak");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_markDummyValue, "RawTuple::markDumyValue");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_firstInstanceWithType, "RawTuple::firstInstanceWithType");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_nextInstanceWithSameType, "RawTuple::nextInstanceWithSameType");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_recordBindingWithOwnerAndName, "RawTuple::recordBindingWithOwnerAndName");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_objectModel_isLogical, "ObjectModel::isLogical");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_objectModel_isNative, "ObjectModel::isNative");
}

void sysbvm_tuple_setupPrimitives(sysbvm_context_t *context)
{
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::type", context->roots.anyValueType, "__type__", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_tuple_primitive_getType);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::type:", context->roots.anyValueType, "__type__:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_tuple_primitive_setType);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::identityHash:", context->roots.anyValueType, "__identityHash__:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_tuple_primitive_setIdentityHash);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::slotAt:", context->roots.anyValueType, "__slotAt__:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_tuple_primitive_slotAt);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::slotAt:put:", context->roots.anyValueType, "__slotAt__:put:", 3, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_tuple_primitive_slotAtPut);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::byteSlotAt:", context->roots.anyValueType, "__byteSlotAt__:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_tuple_primitive_byteSlotAt);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::byteSlotAt:put:", context->roots.anyValueType, "__byteSlotAt__:put:", 3, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_tuple_primitive_byteSlotAtPut);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::refSlotAt:", context->roots.anyValueType, "__refSlotAt__:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_tuple_primitive_refSlotAt);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::refSlotAt:put:", context->roots.anyValueType, "__refSlotAt__:put:", 3, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_tuple_primitive_refSlotAtPut);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::typeSlotAt:", context->roots.anyValueType, "__typeSlotAt__:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_TARGET_DEFINED_PRIMITIVE, NULL, sysbvm_tuple_primitive_typeSlotAt);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::typeSlotAt:put:", context->roots.anyValueType, "__typeSlotAt__:put:", 3, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_TARGET_DEFINED_PRIMITIVE, NULL, sysbvm_tuple_primitive_typeSlotAtPut);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::refTypeSlotAt:", context->roots.anyValueType, "__refTypeSlotAt__:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_TARGET_DEFINED_PRIMITIVE, NULL, sysbvm_tuple_primitive_refTypeSlotAt);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::refTypeSlotAt:put:", context->roots.anyValueType, "__refTypeSlotAt__:put:", 3, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_TARGET_DEFINED_PRIMITIVE, NULL, sysbvm_tuple_primitive_refTypeSlotAtPut);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "RawTuple::new", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_tuple_primitive_new);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "RawTuple::byteNew", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_tuple_primitive_byteNew);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "RawTuple::basicAllocateWithType:instanceSize:alignment:slotCount:variableSize:isBytes:isWeak:", 7, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_tuple_primitive_basicAllocateWithTypeInstanceSizeAlignmentSlotCountIsBytesIsWeak);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::isBytes", context->roots.anyValueType, "__isBytes__", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_tuple_primitive_isBytes);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::isDummyValue", context->roots.anyValueType, "__isDummyValue__", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_tuple_primitive_isDummyValue);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::isWeak", context->roots.anyValueType, "__isWeak__", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_tuple_primitive_isWeak);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::size", context->roots.anyValueType, "__size__", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_tuple_primitive_size);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::byteSize", context->roots.anyValueType, "__byteSize__", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_tuple_primitive_byteSize);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::shallowCopy", context->roots.anyValueType, "__shallowCopy__", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_tuple_primitive_shallowCopy);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::markWeak", context->roots.anyValueType, "__markWeak__", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_tuple_primitive_markWeak);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::markDummyValue", context->roots.anyValueType, "__markDummyValue__", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_tuple_primitive_markDummyValue);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "RawTuple::firstInstanceWithType", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_tuple_primitive_firstInstanceWithType);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "RawTuple::nextInstanceWithSameType", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_tuple_primitive_nextInstanceWithSameType);

    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::recordBindingWithOwnerAndName", context->roots.anyValueType, "recordBindingWithOwner:andName:", 3, SYSBVM_FUNCTION_FLAGS_VIRTUAL, NULL, sysbvm_tuple_primitive_recordBindingWithOwnerAndName);

    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "ObjectModel::isLogical", 0, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_tuple_primitive_objectModel_isLogical);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "ObjectModel::isNative", 0, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_tuple_primitive_objectModel_isNative);
}
