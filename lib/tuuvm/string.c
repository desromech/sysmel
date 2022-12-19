#include "tuuvm/string.h"
#include "tuuvm/context.h"
#include "tuuvm/errors.h"
#include "tuuvm/function.h"
#include "tuuvm/set.h"
#include "tuuvm/type.h"
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

TUUVM_API tuuvm_tuple_t tuuvm_string_createWithReversedString(tuuvm_context_t *context, size_t stringSize, const char *string)
{
    tuuvm_object_tuple_t *result = tuuvm_context_allocateByteTuple(context, context->roots.stringType, stringSize);
    if(!result) return 0;

    for(size_t i = 0; i < stringSize; ++i)
        result->bytes[i] = string[stringSize - i - 1];
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_string_createEmptyWithSize(tuuvm_context_t *context, size_t stringSize)
{
    return (tuuvm_tuple_t)tuuvm_context_allocateByteTuple(context, context->roots.stringType, stringSize);
}

TUUVM_API tuuvm_tuple_t tuuvm_string_createWithPrefix(tuuvm_context_t *context, const char *prefix, tuuvm_tuple_t string)
{
    size_t prefixLen = strlen(prefix);
    if(prefixLen == 0)
        return string;

    size_t stringSize = tuuvm_tuple_getSizeInBytes(string);
    tuuvm_tuple_t result = tuuvm_string_createEmptyWithSize(context, prefixLen + stringSize);
    uint8_t *resultData = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(result)->bytes;
    memcpy(resultData, prefix, prefixLen);
    memcpy(resultData + prefixLen, TUUVM_CAST_OOP_TO_OBJECT_TUPLE(string)->bytes, stringSize);
    return result;
}

TUUVM_API tuuvm_tuple_t tuuvm_string_createWithSuffix(tuuvm_context_t *context, tuuvm_tuple_t string, const char *suffix)
{
    size_t suffixLen = strlen(suffix);
    if(suffixLen == 0)
        return string;

    size_t stringSize = tuuvm_tuple_getSizeInBytes(string);
    tuuvm_tuple_t result = tuuvm_string_createEmptyWithSize(context, stringSize + suffix);
    uint8_t *resultData = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(result)->bytes;
    memcpy(resultData, TUUVM_CAST_OOP_TO_OBJECT_TUPLE(string)->bytes, stringSize);
    memcpy(resultData + stringSize, suffix, suffixLen);
    return result;
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

        tuuvm_tuple_t existent;
        if(tuuvm_set_findWithExplicitHash(context->roots.internedSymbolSet, &stringSlice, tuuvm_stringSlice_hashFunction, tuuvm_stringSlice_equalsFunction, &existent))
            return existent;
    }

    tuuvm_object_tuple_t *result = tuuvm_context_allocateByteTuple(context, context->roots.symbolType, stringSize);
    if(!result) return 0;

    // In the case of symbols, make the identity hash match with the string hash.
    result->header.identityHash = tuuvm_string_computeHashWithBytes(stringSize, (const uint8_t*)string);

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
    size_t result = tuuvm_hashMultiply(1);
    for(size_t i = 0; i < size; ++i)
        result = tuuvm_hashMultiply(result) + bytes[i];
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

TUUVM_API tuuvm_tuple_t tuuvm_tuple_defaultToString(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_printString(context, tuple);
}

TUUVM_API tuuvm_tuple_t tuuvm_tuple_defaultPrintString(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    tuuvm_tuple_t type = tuuvm_tuple_getType(context, tuple);
    if(type != TUUVM_NULL_TUPLE)
    {
        // Is the tuple a type?
        if(type == context->roots.typeType)
        {
            tuuvm_tuple_t thisTypeName = tuuvm_type_getName(tuple);
            if(tuuvm_tuple_isNonNullPointer(thisTypeName))   
                return thisTypeName;
        }

        tuuvm_tuple_t typeName = tuuvm_type_getName(type);
        if(tuuvm_tuple_isNonNullPointer(typeName))
            return tuuvm_string_createWithPrefix(context, "a ", typeName);
    }

    return tuuvm_string_createWithCString(context, "TODO: defaultPrintString");
}

TUUVM_API tuuvm_tuple_t tuuvm_tuple_toString(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    tuuvm_tuple_t type = tuuvm_tuple_getType(context, tuple);
    tuuvm_tuple_t toStringFunction = tuuvm_type_getToStringFunction(type);
    if(toStringFunction == TUUVM_NULL_TUPLE)
        return tuuvm_tuple_defaultToString(context, tuple);
    return tuuvm_function_apply1(context, toStringFunction, tuple);
}

TUUVM_API tuuvm_tuple_t tuuvm_tuple_printString(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    tuuvm_tuple_t type = tuuvm_tuple_getType(context, tuple);
    tuuvm_tuple_t printStringFunction = tuuvm_type_getPrintStringFunction(type);
    if(printStringFunction == TUUVM_NULL_TUPLE)
        return tuuvm_tuple_defaultPrintString(context, tuple);
    return tuuvm_function_apply1(context, printStringFunction, tuple);
}

tuuvm_tuple_t tuuvm_string_primitive_toString(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);
    return arguments[0];
}

tuuvm_tuple_t tuuvm_symbol_primitive_toString(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    tuuvm_tuple_t symbol = arguments[0];
    return tuuvm_string_createWithString(context, tuuvm_tuple_getSizeInBytes(symbol), (const char*)TUUVM_CAST_OOP_TO_OBJECT_TUPLE(symbol)->bytes);
}

tuuvm_tuple_t tuuvm_tuple_primitive_printString(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    tuuvm_tuple_t tuple = arguments[0];
    return tuuvm_tuple_printString(context, tuple);
}

tuuvm_tuple_t tuuvm_tuple_primitive_toString(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    tuuvm_tuple_t tuple = arguments[0];
    return tuuvm_tuple_toString(context, tuple);
}

void tuuvm_string_setupPrimitives(tuuvm_context_t *context)
{
    // String primitives
    {
        tuuvm_type_setToStringFunction(context->roots.stringType, tuuvm_function_createPrimitive(context, 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_string_primitive_toString));
    }

    // Symbol primitives.
    {
        tuuvm_type_setToStringFunction(context->roots.symbolType, tuuvm_function_createPrimitive(context, 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_symbol_primitive_toString));
    }

    tuuvm_context_setIntrinsicSymbolBinding(context, tuuvm_symbol_internWithCString(context, "printString"), tuuvm_function_createPrimitive(context, 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_tuple_primitive_printString));
    tuuvm_context_setIntrinsicSymbolBinding(context, tuuvm_symbol_internWithCString(context, "toString"), tuuvm_function_createPrimitive(context, 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_tuple_primitive_toString));
}
