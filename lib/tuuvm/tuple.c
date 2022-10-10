#include "tuuvm/tuple.h"
#include "internal/context.h"

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

TUUVM_API tuuvm_tuple_t tuuvm_tuple_primitive_identityHash(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    (void)argumentCount;
    return tuuvm_tuple_size_encode(context, tuuvm_tuple_identityHash(arguments[0]));
}

TUUVM_API tuuvm_tuple_t tuuvm_tuple_primitive_identityEquals(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)argumentCount;
    return tuuvm_tuple_boolean_encode(tuuvm_tuple_identityEquals(arguments[0], arguments[1]));
}
