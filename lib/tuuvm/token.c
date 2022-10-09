#include "tuuvm/token.h"
#include "internal/context.h"

TUUVM_API tuuvm_tuple_t tuuvm_token_create(tuuvm_context_t *context, tuuvm_tuple_t kind, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t value)
{
    tuuvm_token_t *result = (tuuvm_token_t*)tuuvm_context_allocatePointerTuple(context, context->roots.sourceCodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_token_t));
    result->kind = kind;
    result->sourcePosition = sourcePosition;
    result->value = value;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_token_createWithKind(tuuvm_context_t *context, tuuvm_tokenKind_t kind, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t value)
{
    return tuuvm_token_create(context, tuuvm_tuple_uint8_encode(kind), sourcePosition, value);
}

TUUVM_API tuuvm_tokenKind_t tuuvm_token_getKind(tuuvm_tuple_t token)
{
    return tuuvm_tuple_uint8_decode(((tuuvm_token_t*)token)->kind);
}

TUUVM_API tuuvm_tuple_t tuuvm_token_getSourcePosition(tuuvm_tuple_t token)
{
    return ((tuuvm_token_t*)token)->sourcePosition;
}

TUUVM_API tuuvm_tuple_t tuuvm_token_getValue(tuuvm_tuple_t token)
{
    return ((tuuvm_token_t*)token)->value;
}
