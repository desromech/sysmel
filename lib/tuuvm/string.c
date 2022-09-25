#include "tuuvm/string.h"
#include "tuuvm/context.h"
#include "internal/context.h"
#include <string.h>

TUUVM_API tuuvm_tuple_t tuuvm_string_createWithString(tuuvm_context_t *context, size_t stringSize, const char *string)
{
    tuuvm_object_tuple_t *result = tuuvm_context_allocateByteTuple(context, stringSize);
    if(!result) return 0;

    memcpy(result->bytes, string, stringSize);
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_string_createWithCString(tuuvm_context_t *context, const char *cstring)
{
    return tuuvm_string_createWithString(context, strlen(cstring), cstring);
}

TUUVM_API tuuvm_tuple_t tuuvm_symbol_internWithString(tuuvm_context_t *context, size_t stringSize, const char *string)
{
    tuuvm_object_tuple_t *result = tuuvm_context_allocateByteTuple(context, stringSize);
    if(!result) return 0;

    memcpy(result->bytes, string, stringSize);
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_symbol_internFromTuple(tuuvm_context_t *context, tuuvm_tuple_t byteTuple)
{
    if(!tuuvm_tuple_isBytes(byteTuple)) return TUUVM_NULL_TUPLE;
    return tuuvm_symbol_internWithString(context, tuuvm_tuple_getSizeInBytes(byteTuple), (const char*)TUUVM_CAST_OOP_TO_OBJECT_TUPLE(byteTuple)->bytes);
}

TUUVM_API tuuvm_tuple_t tuuvm_symbol_internWithCString(tuuvm_context_t *context, const char *cstring)
{
    return tuuvm_symbol_internWithString(context, strlen(cstring), cstring);
}
