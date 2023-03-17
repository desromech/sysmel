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
    tuuvm_tuple_t result = tuuvm_string_createEmptyWithSize(context, stringSize + suffixLen);
    uint8_t *resultData = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(result)->bytes;
    memcpy(resultData, TUUVM_CAST_OOP_TO_OBJECT_TUPLE(string)->bytes, stringSize);
    memcpy(resultData + stringSize, suffix, suffixLen);
    return result;
}

TUUVM_API tuuvm_tuple_t tuuvm_string_createWithCString(tuuvm_context_t *context, const char *cstring)
{
    return tuuvm_string_createWithString(context, strlen(cstring), cstring);
}

TUUVM_API tuuvm_tuple_t tuuvm_string_concat(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right)
{
    size_t leftSize = tuuvm_tuple_getSizeInBytes(left);
    size_t rightSize = tuuvm_tuple_getSizeInBytes(right);

    if(leftSize == 0)
        return right;
    if(rightSize == 0)
        return left;

    size_t resultSize = leftSize + rightSize;
    tuuvm_tuple_t result = tuuvm_string_createEmptyWithSize(context, resultSize);
    uint8_t *resultData = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(result)->bytes;
    memcpy(resultData, TUUVM_CAST_OOP_TO_OBJECT_TUPLE(left)->bytes, leftSize);
    memcpy(resultData + leftSize, TUUVM_CAST_OOP_TO_OBJECT_TUPLE(right)->bytes, rightSize);
    return result;
}

TUUVM_API tuuvm_tuple_t tuuvm_symbol_internWithString(tuuvm_context_t *context, size_t stringSize, const char *string)
{
    {
        tuuvm_stringSlice_t stringSlice = {
            .elements = string,
            .size = stringSize
        };

        tuuvm_tuple_t existent;
        if(tuuvm_identitySet_findWithExplicitHash(context->roots.internedSymbolSet, &stringSlice, tuuvm_stringSlice_hashFunction, tuuvm_stringSlice_equalsFunction, &existent))
            return existent;
    }

    tuuvm_object_tuple_t *result = tuuvm_context_allocateByteTuple(context, context->roots.stringSymbolType, stringSize);
    if(!result) return 0;

    // In the case of symbols, make the identity hash match with the string hash.
    tuuvm_tuple_setIdentityHash(result, tuuvm_string_computeHashWithBytes(stringSize, (const uint8_t*)string));

    memcpy(result->bytes, string, stringSize);
    tuuvm_identitySet_insert(context, context->roots.internedSymbolSet, (tuuvm_tuple_t)result);
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
        result = tuuvm_hashConcatenate(result, bytes[i]);
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

    uint8_t *firstBytes = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(a)->bytes;
    uint8_t *secondBytes = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(b)->bytes;
    return memcmp(firstBytes, secondBytes, firstSize) == 0;
}

TUUVM_API bool tuuvm_string_equalsCString(tuuvm_tuple_t string, const char *cstring)
{
    if(!tuuvm_tuple_isNonNullPointer(string))
        return false;

    size_t firstSize = tuuvm_tuple_getSizeInBytes(string);
    size_t secondSize = strlen(cstring);
    if(firstSize != secondSize)
        return false;

    uint8_t *firstBytes = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(string)->bytes;
    return memcmp(firstBytes, cstring, firstSize) == 0;
}

TUUVM_API bool tuuvm_string_endsWithCString(tuuvm_tuple_t string, const char *cstring)
{
    if(!tuuvm_tuple_isNonNullPointer(string))
        return false;

    size_t firstSize = tuuvm_tuple_getSizeInBytes(string);
    size_t secondSize = strlen(cstring);
    if(firstSize < secondSize)
        return false;

    uint8_t *firstBytes = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(string)->bytes;
    return memcmp(firstBytes + firstSize - secondSize, cstring, secondSize) == 0;
}

TUUVM_API tuuvm_tuple_t tuuvm_string_primitive_hash(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    (void)argumentCount;
    return tuuvm_tuple_size_encode(context, tuuvm_string_hash(arguments[0]));
}

TUUVM_API tuuvm_tuple_t tuuvm_string_primitive_equals(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)argumentCount;
    return tuuvm_tuple_boolean_encode(tuuvm_string_equals(arguments[0], arguments[1]));
}

TUUVM_API tuuvm_tuple_t tuuvm_tuple_defaultAsString(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_printString(context, tuple);
}

TUUVM_API tuuvm_tuple_t tuuvm_tuple_defaultPrintString(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    tuuvm_tuple_t type = tuuvm_tuple_getType(context, tuple);
    if(type != TUUVM_NULL_TUPLE)
    {
        // Is the tuple a type?
        if(tuuvm_type_isDirectSubtypeOf(type, context->roots.typeType))
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

TUUVM_API tuuvm_tuple_t tuuvm_tuple_asString(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    tuuvm_tuple_t type = tuuvm_tuple_getType(context, tuple);
    tuuvm_tuple_t asStringFunction = tuuvm_type_getAsStringFunction(context, type);
    if(asStringFunction == TUUVM_NULL_TUPLE)
        return tuuvm_tuple_defaultAsString(context, tuple);
    return tuuvm_function_apply1(context, asStringFunction, tuple);
}

TUUVM_API tuuvm_tuple_t tuuvm_tuple_printString(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    tuuvm_tuple_t type = tuuvm_tuple_getType(context, tuple);
    tuuvm_tuple_t printStringFunction = tuuvm_type_getPrintStringFunction(context, type);
    if(printStringFunction == TUUVM_NULL_TUPLE)
        return tuuvm_tuple_defaultPrintString(context, tuple);
    return tuuvm_function_apply1(context, printStringFunction, tuple);
}

tuuvm_tuple_t tuuvm_string_primitive_asString(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);
    return arguments[0];
}

tuuvm_tuple_t tuuvm_symbol_primitive_asString(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    tuuvm_tuple_t symbol = arguments[0];
    return tuuvm_string_createWithString(context, tuuvm_tuple_getSizeInBytes(symbol), (const char*)TUUVM_CAST_OOP_TO_OBJECT_TUPLE(symbol)->bytes);
}

tuuvm_tuple_t tuuvm_tuple_primitive_printString(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    tuuvm_tuple_t tuple = arguments[0];
    return tuuvm_tuple_printString(context, tuple);
}

tuuvm_tuple_t tuuvm_tuple_primitive_asString(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    tuuvm_tuple_t tuple = arguments[0];
    return tuuvm_tuple_asString(context, tuple);
}

static tuuvm_tuple_t tuuvm_tuple_primitive_defaultPrintString(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    tuuvm_tuple_t tuple = arguments[0];
    return tuuvm_tuple_defaultPrintString(context, tuple);
}

static tuuvm_tuple_t tuuvm_tuple_primitive_defaultAsString(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    tuuvm_tuple_t tuple = arguments[0];
    return tuuvm_tuple_defaultAsString(context, tuple);
}

static tuuvm_tuple_t tuuvm_string_primitive_concat(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return tuuvm_string_concat(context, arguments[0], arguments[1]);
}

static tuuvm_tuple_t tuuvm_string_primitive_withoutSuffix(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t string = arguments[0];
    tuuvm_tuple_t suffix = arguments[1];

    size_t suffixLen = tuuvm_tuple_getSizeInBytes(suffix);
    if(suffixLen == 0)
        return string;

    size_t stringSize = tuuvm_tuple_getSizeInBytes(string);
    if(stringSize < suffixLen)
        return string;
    
    uint8_t *suffixData = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(suffix)->bytes;
    uint8_t *stringData = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(string)->bytes;
    if(memcmp(stringData + stringSize - suffixLen, suffixData, suffixLen))
        return string;

    return tuuvm_string_createWithString(context, stringSize - suffixLen, (const char*)(stringData));
}

static tuuvm_tuple_t tuuvm_symbol_primitive_intern(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_symbol_internFromTuple(context, arguments[0]);
}

void tuuvm_string_registerPrimitives(void)
{
    tuuvm_primitiveTable_registerFunction(tuuvm_string_primitive_asString, "String::asString");
    tuuvm_primitiveTable_registerFunction(tuuvm_symbol_primitive_asString, "Symbol::asString");

    tuuvm_primitiveTable_registerFunction(tuuvm_tuple_primitive_printString, "RawTuple::printString");
    tuuvm_primitiveTable_registerFunction(tuuvm_tuple_primitive_asString, "RawTuple::asString");
    tuuvm_primitiveTable_registerFunction(tuuvm_tuple_primitive_defaultPrintString, "RawTuple::defaultPrintString");
    tuuvm_primitiveTable_registerFunction(tuuvm_tuple_primitive_defaultAsString, "RawTuple::defaultAsString");
    tuuvm_primitiveTable_registerFunction(tuuvm_string_primitive_concat, "String::--");
    tuuvm_primitiveTable_registerFunction(tuuvm_string_primitive_withoutSuffix, "String::withoutSuffix:");
    tuuvm_primitiveTable_registerFunction(tuuvm_symbol_primitive_intern, "String::intern");
}

void tuuvm_string_setupPrimitives(tuuvm_context_t *context)
{
    // String primitives
    {
        tuuvm_type_setAsStringFunction(context, context->roots.stringType, tuuvm_function_createPrimitive(context, 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_string_primitive_asString));
    }

    // StringSymbol primitives.
    {
        tuuvm_type_setAsStringFunction(context, context->roots.stringSymbolType, tuuvm_function_createPrimitive(context, 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_symbol_primitive_asString));
    }

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "printString", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_tuple_primitive_printString);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "asString", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_tuple_primitive_asString);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::printString", context->roots.anyValueType, "printString", 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_tuple_primitive_defaultPrintString);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "RawTuple::asString", context->roots.anyValueType, "asString", 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_tuple_primitive_defaultAsString);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "String::concat:", context->roots.stringType, "--", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_string_primitive_concat);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "String::withoutSuffix:", context->roots.stringType, "withoutSuffix:", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_string_primitive_withoutSuffix);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "StringSymbol::intern", context->roots.stringType, "asSymbol", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_symbol_primitive_intern);
}
