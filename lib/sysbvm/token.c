#include "sysbvm/token.h"
#include "internal/context.h"

SYSBVM_API sysbvm_tuple_t sysbvm_token_create(sysbvm_context_t *context, sysbvm_tuple_t kind, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t value)
{
    sysbvm_token_t *result = (sysbvm_token_t*)sysbvm_context_allocatePointerTuple(context, context->roots.sourceCodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_token_t));
    result->kind = kind;
    result->sourcePosition = sourcePosition;
    result->value = value;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_token_createWithKind(sysbvm_context_t *context, sysbvm_tokenKind_t kind, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t value)
{
    return sysbvm_token_create(context, sysbvm_tuple_uint8_encode(kind), sourcePosition, value);
}

SYSBVM_API sysbvm_tokenKind_t sysbvm_token_getKind(sysbvm_tuple_t token)
{
    return sysbvm_tuple_uint8_decode(((sysbvm_token_t*)token)->kind);
}

SYSBVM_API sysbvm_tuple_t sysbvm_token_getSourcePosition(sysbvm_tuple_t token)
{
    return ((sysbvm_token_t*)token)->sourcePosition;
}

SYSBVM_API sysbvm_tuple_t sysbvm_token_getValue(sysbvm_tuple_t token)
{
    return ((sysbvm_token_t*)token)->value;
}
