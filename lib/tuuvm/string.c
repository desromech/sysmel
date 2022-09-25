#include "tuuvm/string.h"
#include "tuuvm/context.h"
#include <string.h>

TUUVM_API tuuvm_tuple_t tuuvm_string_create(tuuvm_context_t *context, size_t stringSize, const char *string)
{
    tuuvm_object_tuple_t *result = tuuvm_context_allocateByteTuple(context, stringSize);
    if(!result) return 0;

    memcpy(result->bytes, string, stringSize);
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_string_createWithCString(tuuvm_context_t *context, const char *cstring)
{
    return tuuvm_string_create(context, strlen(cstring), cstring);
}
