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