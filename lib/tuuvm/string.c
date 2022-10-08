#include "tuuvm/string.h"
#include "tuuvm/context.h"
#include "tuuvm/set.h"
#include "internal/context.h"
#include <string.h>


typedef struct tuuvm_stringSlice_s
{
    const char *elements;
    size_t size;
} tuuvm_stringSlice_t;

static size_t tuuvm_stringSlice_hashFunction(void *element);

bool tuuvm_stringSlice_equalsFunction(void *element, tuuvm_tuple_t setElement)
{
    tuuvm_stringSlice_t *stringSlice = (tuuvm_stringSlice_t*)element;
    if(tuuvm_tuple_isBytes(!setElement))
        return false;

    size_t setElementSize = tuuvm_tuple_getSizeInBytes(setElement);
    if(setElementSize != stringSlice->size)
        return false;

    return memcmp(stringSlice->elements, TUUVM_CAST_OOP_TO_OBJECT_TUPLE(setElement)->bytes, setElementSize) == 0;
}


TUUVM_API tuuvm_tuple_t tuuvm_string_createWithString(tuuvm_context_t *context, size_t stringSize, const char *string)
{
    tuuvm_object_tuple_t *result = tuuvm_context_allocateByteTuple(context, context->roots.stringType, stringSize);
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
    {
        tuuvm_stringSlice_t stringSlice = {
            .elements = string,
            .size = stringSize
        };

        tuuvm_tuple_t existent = tuuvm_set_findOrNilWithExplicitHash(context->roots.internedSymbolSet, &stringSlice, tuuvm_stringSlice_hashFunction, tuuvm_stringSlice_equalsFunction);
        if(existent != TUUVM_NULL_TUPLE)
            return existent;
    }

    tuuvm_object_tuple_t *result = tuuvm_context_allocateByteTuple(context, context->roots.symbolType, stringSize);
    if(!result) return 0;

    memcpy(result->bytes, string, stringSize);
    tuuvm_set_insert(context, context->roots.internedSymbolSet, (tuuvm_tuple_t)result);
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

TUUVM_API size_t tuuvm_string_computeHashWithBytes(size_t size, const uint8_t *bytes)
{
    // TODO: Use a better hash function.
    size_t result = 0;
    for(size_t i = 0; i < size; ++i)
        result = result * 33 + bytes[i];
    return result;
}

static size_t tuuvm_stringSlice_hashFunction(void *element)
{
    tuuvm_stringSlice_t *stringSlice = (tuuvm_stringSlice_t*)element;
    return tuuvm_string_computeHashWithBytes(stringSlice->size, (const uint8_t *)stringSlice->elements);
}

TUUVM_API size_t tuuvm_string_hash(tuuvm_tuple_t string)
{
    if(!tuuvm_tuple_isNonNullPointer(string))
        return 0;

    return tuuvm_string_computeHashWithBytes(tuuvm_tuple_getSizeInBytes(string), TUUVM_CAST_OOP_TO_OBJECT_TUPLE(string)->bytes);
}

TUUVM_API bool tuuvm_string_equals(tuuvm_tuple_t a, tuuvm_tuple_t b)
{
    if(a == b)
        return true;

    if(!tuuvm_tuple_isNonNullPointer(a) || !tuuvm_tuple_isNonNullPointer(b))
        return false;

    size_t firstSize = tuuvm_tuple_getSizeInBytes(a);
    size_t secondSize = tuuvm_tuple_getSizeInBytes(b);
    if(firstSize != secondSize)
        return false;

    return memcmp(TUUVM_CAST_OOP_TO_OBJECT_TUPLE(a)->bytes, TUUVM_CAST_OOP_TO_OBJECT_TUPLE(b)->bytes, firstSize) == 0;
}

TUUVM_API tuuvm_tuple_t tuuvm_string_primitive_hash(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    (void)argumentCount;
    return tuuvm_tuple_size_encode(context, tuuvm_string_hash(arguments[0]));
}

TUUVM_API tuuvm_tuple_t tuuvm_string_primitive_equals(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)argumentCount;
    return tuuvm_tuple_boolean_encode(tuuvm_string_equals(arguments[0], arguments[1]));
}
