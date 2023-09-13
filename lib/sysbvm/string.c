#include "sysbvm/string.h"
#include "sysbvm/context.h"
#include "sysbvm/errors.h"
#include "sysbvm/function.h"
#include "sysbvm/set.h"
#include "sysbvm/type.h"
#include "internal/context.h"
#include <string.h>

typedef struct sysbvm_stringSlice_s
{
    const char *elements;
    size_t size;
} sysbvm_stringSlice_t;

static size_t sysbvm_stringSliceSymbol_hashFunction(sysbvm_context_t *context, void *element);

bool sysbvm_stringSlice_equalsFunction(void *element, sysbvm_tuple_t setElement)
{
    sysbvm_stringSlice_t *stringSlice = (sysbvm_stringSlice_t*)element;
    if(sysbvm_tuple_isBytes(!setElement))
        return false;

    size_t setElementSize = sysbvm_tuple_getSizeInBytes(setElement);
    if(setElementSize != stringSlice->size)
        return false;

    return memcmp(stringSlice->elements, SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(setElement)->bytes, setElementSize) == 0;
}


SYSBVM_API sysbvm_tuple_t sysbvm_string_createWithString(sysbvm_context_t *context, size_t stringSize, const char *string)
{
    sysbvm_object_tuple_t *result = sysbvm_context_allocateByteTuple(context, context->roots.stringType, stringSize);
    if(!result) return 0;

    memcpy(result->bytes, string, stringSize);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_string_createWithReversedString(sysbvm_context_t *context, size_t stringSize, const char *string)
{
    sysbvm_object_tuple_t *result = sysbvm_context_allocateByteTuple(context, context->roots.stringType, stringSize);
    if(!result) return 0;

    for(size_t i = 0; i < stringSize; ++i)
        result->bytes[i] = string[stringSize - i - 1];
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_string_createEmptyWithSize(sysbvm_context_t *context, size_t stringSize)
{
    return (sysbvm_tuple_t)sysbvm_context_allocateByteTuple(context, context->roots.stringType, stringSize);
}

SYSBVM_API sysbvm_tuple_t sysbvm_string_createWithPrefix(sysbvm_context_t *context, const char *prefix, sysbvm_tuple_t string)
{
    size_t prefixLen = strlen(prefix);
    if(prefixLen == 0)
        return string;

    size_t stringSize = sysbvm_tuple_getSizeInBytes(string);
    sysbvm_tuple_t result = sysbvm_string_createEmptyWithSize(context, prefixLen + stringSize);
    uint8_t *resultData = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(result)->bytes;
    memcpy(resultData, prefix, prefixLen);
    memcpy(resultData + prefixLen, SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(string)->bytes, stringSize);
    return result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_string_createWithSuffix(sysbvm_context_t *context, sysbvm_tuple_t string, const char *suffix)
{
    size_t suffixLen = strlen(suffix);
    if(suffixLen == 0)
        return string;

    size_t stringSize = sysbvm_tuple_getSizeInBytes(string);
    sysbvm_tuple_t result = sysbvm_string_createEmptyWithSize(context, stringSize + suffixLen);
    uint8_t *resultData = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(result)->bytes;
    memcpy(resultData, SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(string)->bytes, stringSize);
    memcpy(resultData + stringSize, suffix, suffixLen);
    return result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_string_createWithoutSuffix(sysbvm_context_t *context, sysbvm_tuple_t string, const char *suffix)
{
    size_t suffixLen = strlen(suffix);
    if(suffixLen == 0)
        return string;

    size_t stringSize = sysbvm_tuple_getSizeInBytes(string);
    if(stringSize < suffixLen)
        return string;

    uint8_t *stringData = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(string)->bytes;
    if(memcmp(stringData + stringSize - suffixLen, suffix, suffixLen))
        return string;

    sysbvm_tuple_t result = sysbvm_string_createEmptyWithSize(context, stringSize - suffixLen);
    uint8_t *resultData = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(result)->bytes;
    memcpy(resultData, stringData, stringSize - suffixLen);
    return result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_string_createWithCString(sysbvm_context_t *context, const char *cstring)
{
    return sysbvm_string_createWithString(context, strlen(cstring), cstring);
}

SYSBVM_API sysbvm_tuple_t sysbvm_string_concat(sysbvm_context_t *context, sysbvm_tuple_t left, sysbvm_tuple_t right)
{
    size_t leftSize = sysbvm_tuple_getSizeInBytes(left);
    size_t rightSize = sysbvm_tuple_getSizeInBytes(right);

    if(leftSize == 0)
        return right;
    if(rightSize == 0)
        return left;

    size_t resultSize = leftSize + rightSize;
    sysbvm_tuple_t result = sysbvm_string_createEmptyWithSize(context, resultSize);
    uint8_t *resultData = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(result)->bytes;
    memcpy(resultData, SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(left)->bytes, leftSize);
    memcpy(resultData + leftSize, SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(right)->bytes, rightSize);
    return result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_symbol_internWithString(sysbvm_context_t *context, size_t stringSize, const char *string)
{
    {
        sysbvm_stringSlice_t stringSlice = {
            .elements = string,
            .size = stringSize
        };

        sysbvm_tuple_t existent;
        if(sysbvm_identitySet_findWithExplicitHash(context, context->roots.internedSymbolSet, &stringSlice, sysbvm_stringSliceSymbol_hashFunction, sysbvm_stringSlice_equalsFunction, &existent))
            return existent;
    }

    sysbvm_object_tuple_t *result = sysbvm_context_allocateByteTuple(context, context->roots.stringSymbolType, stringSize);
    if(!result) return 0;

    // In the case of symbols, make the identity hash match with the string hash.
    sysbvm_tuple_setIdentityHash(result, sysbvm_string_computeHashWithBytes(stringSize, (const uint8_t*)string));

    memcpy(result->bytes, string, stringSize);
    sysbvm_identitySet_insert(context, context->roots.internedSymbolSet, (sysbvm_tuple_t)result);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_symbol_internFromTuple(sysbvm_context_t *context, sysbvm_tuple_t byteTuple)
{
    if(!sysbvm_tuple_isBytes(byteTuple)) return SYSBVM_NULL_TUPLE;
    return sysbvm_symbol_internWithString(context, sysbvm_tuple_getSizeInBytes(byteTuple), (const char*)SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(byteTuple)->bytes);
}

SYSBVM_API sysbvm_tuple_t sysbvm_symbol_internWithCString(sysbvm_context_t *context, const char *cstring)
{
    return sysbvm_symbol_internWithString(context, strlen(cstring), cstring);
}

SYSBVM_API size_t sysbvm_string_computeHashWithBytes(size_t size, const uint8_t *bytes)
{
    size_t result = sysbvm_hashMultiply(1);
    for(size_t i = 0; i < size; ++i)
        result = sysbvm_hashConcatenate(result, bytes[i]);
    return result;
}

static size_t sysbvm_stringSliceSymbol_hashFunction(sysbvm_context_t *context, void *element)
{
    sysbvm_stringSlice_t *stringSlice = (sysbvm_stringSlice_t*)element;
    size_t symbolHash = (SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(context->roots.stringSymbolType)->header.identityHashAndFlags >> SYSBVM_TUPLE_TAG_BIT_COUNT) & SYSBVM_STORED_IDENTITY_HASH_BIT_MASK;
    size_t hash = sysbvm_string_computeHashWithBytes(stringSlice->size, (const uint8_t *)stringSlice->elements);
    return (hash | (symbolHash << SYSBVM_STORED_IDENTITY_HASH_BIT_COUNT)) & SYSBVM_HASH_BIT_MASK;
}

SYSBVM_API size_t sysbvm_string_hash(sysbvm_tuple_t string)
{
    if(!sysbvm_tuple_isNonNullPointer(string))
        return 0;

    return sysbvm_string_computeHashWithBytes(sysbvm_tuple_getSizeInBytes(string), SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(string)->bytes);
}

SYSBVM_API bool sysbvm_string_equals(sysbvm_tuple_t a, sysbvm_tuple_t b)
{
    if(a == b)
        return true;

    if(!sysbvm_tuple_isNonNullPointer(a) || !sysbvm_tuple_isNonNullPointer(b))
        return false;

    size_t firstSize = sysbvm_tuple_getSizeInBytes(a);
    size_t secondSize = sysbvm_tuple_getSizeInBytes(b);
    if(firstSize != secondSize)
        return false;

    uint8_t *firstBytes = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(a)->bytes;
    uint8_t *secondBytes = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(b)->bytes;
    return memcmp(firstBytes, secondBytes, firstSize) == 0;
}

SYSBVM_API bool sysbvm_string_equalsCString(sysbvm_tuple_t string, const char *cstring)
{
    if(!sysbvm_tuple_isNonNullPointer(string))
        return false;

    size_t firstSize = sysbvm_tuple_getSizeInBytes(string);
    size_t secondSize = strlen(cstring);
    if(firstSize != secondSize)
        return false;

    uint8_t *firstBytes = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(string)->bytes;
    return memcmp(firstBytes, cstring, firstSize) == 0;
}

SYSBVM_API bool sysbvm_string_endsWithCString(sysbvm_tuple_t string, const char *cstring)
{
    if(!sysbvm_tuple_isNonNullPointer(string))
        return false;

    size_t firstSize = sysbvm_tuple_getSizeInBytes(string);
    size_t secondSize = strlen(cstring);
    if(firstSize < secondSize)
        return false;

    uint8_t *firstBytes = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(string)->bytes;
    return memcmp(firstBytes + firstSize - secondSize, cstring, secondSize) == 0;
}

SYSBVM_API sysbvm_tuple_t sysbvm_string_primitive_hash(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    (void)argumentCount;
    return sysbvm_tuple_size_encode(context, sysbvm_string_hash(arguments[0]));
}

SYSBVM_API sysbvm_tuple_t sysbvm_string_primitive_equals(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)argumentCount;
    return sysbvm_tuple_boolean_encode(sysbvm_string_equals(arguments[0], arguments[1]));
}

SYSBVM_API sysbvm_tuple_t sysbvm_tuple_defaultAsString(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_printString(context, tuple);
}

SYSBVM_API sysbvm_tuple_t sysbvm_tuple_defaultPrintString(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    sysbvm_tuple_t type = sysbvm_tuple_getType(context, tuple);
    if(type != SYSBVM_NULL_TUPLE)
    {
        // Is the tuple a type?
        if(sysbvm_type_isDirectSubtypeOf(type, context->roots.typeType))
        {
            sysbvm_tuple_t thisTypeName = sysbvm_type_getName(tuple);
            if(sysbvm_tuple_isNonNullPointer(thisTypeName))   
                return thisTypeName;
        }

        sysbvm_tuple_t typeName = sysbvm_type_getName(type);
        if(sysbvm_tuple_isNonNullPointer(typeName))
            return sysbvm_string_createWithPrefix(context, "a ", typeName);
    }

    return sysbvm_string_createWithCString(context, "TODO: defaultPrintString");
}

SYSBVM_API sysbvm_tuple_t sysbvm_tuple_asString(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    sysbvm_tuple_t type = sysbvm_tuple_getType(context, tuple);
    if(type == context->roots.stringType)
        return tuple;
    sysbvm_tuple_t asStringFunction = sysbvm_type_getAsStringFunction(context, type);
    if(asStringFunction == SYSBVM_NULL_TUPLE)
        return sysbvm_tuple_defaultAsString(context, tuple);
    return sysbvm_function_apply1(context, asStringFunction, tuple);
}

SYSBVM_API sysbvm_tuple_t sysbvm_tuple_printString(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    sysbvm_tuple_t type = sysbvm_tuple_getType(context, tuple);
    sysbvm_tuple_t printStringFunction = sysbvm_type_getPrintStringFunction(context, type);
    if(printStringFunction == SYSBVM_NULL_TUPLE)
        return sysbvm_tuple_defaultPrintString(context, tuple);
    return sysbvm_function_apply1(context, printStringFunction, tuple);
}

sysbvm_tuple_t sysbvm_string_primitive_asString(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);
    return arguments[0];
}

sysbvm_tuple_t sysbvm_symbol_primitive_asString(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_tuple_t symbol = arguments[0];
    return sysbvm_string_createWithString(context, sysbvm_tuple_getSizeInBytes(symbol), (const char*)SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(symbol)->bytes);
}

static sysbvm_tuple_t sysbvm_tuple_primitive_defaultPrintString(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_tuple_t tuple = arguments[0];
    return sysbvm_tuple_defaultPrintString(context, tuple);
}

static sysbvm_tuple_t sysbvm_tuple_primitive_defaultAsString(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_tuple_t tuple = arguments[0];
    return sysbvm_tuple_defaultAsString(context, tuple);
}

static sysbvm_tuple_t sysbvm_string_primitive_concat(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    return sysbvm_string_concat(context, arguments[0], arguments[1]);
}

static sysbvm_tuple_t sysbvm_string_primitive_withoutSuffix(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t string = arguments[0];
    sysbvm_tuple_t suffix = arguments[1];

    size_t suffixLen = sysbvm_tuple_getSizeInBytes(suffix);
    if(suffixLen == 0)
        return string;

    size_t stringSize = sysbvm_tuple_getSizeInBytes(string);
    if(stringSize < suffixLen)
        return string;
    
    uint8_t *suffixData = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(suffix)->bytes;
    uint8_t *stringData = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(string)->bytes;
    if(memcmp(stringData + stringSize - suffixLen, suffixData, suffixLen))
        return string;

    return sysbvm_string_createWithString(context, stringSize - suffixLen, (const char*)(stringData));
}

static sysbvm_tuple_t sysbvm_symbol_primitive_intern(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_symbol_internFromTuple(context, arguments[0]);
}

void sysbvm_string_registerPrimitives(void)
{
    sysbvm_primitiveTable_registerFunction(sysbvm_string_primitive_asString, "String::asString");
    sysbvm_primitiveTable_registerFunction(sysbvm_symbol_primitive_asString, "Symbol::asString");

    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_defaultPrintString, "RawTuple::defaultPrintString");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_defaultAsString, "RawTuple::defaultAsString");
    sysbvm_primitiveTable_registerFunction(sysbvm_string_primitive_concat, "String::--");
    sysbvm_primitiveTable_registerFunction(sysbvm_string_primitive_withoutSuffix, "String::withoutSuffix:");
    sysbvm_primitiveTable_registerFunction(sysbvm_symbol_primitive_intern, "String::intern");
}

void sysbvm_string_setupPrimitives(sysbvm_context_t *context)
{
    // String primitives
    {
        sysbvm_type_setAsStringFunction(context, context->roots.stringType, sysbvm_function_createPrimitive(context, 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_string_primitive_asString));
    }

    // StringSymbol primitives.
    {
        sysbvm_type_setAsStringFunction(context, context->roots.stringSymbolType, sysbvm_function_createPrimitive(context, 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_symbol_primitive_asString));
        sysbvm_type_setPrintStringFunction(context, context->roots.stringSymbolType, sysbvm_function_createPrimitive(context, 1, SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_symbol_primitive_asString));
    }

    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.anyValueType, "printString", 1, SYSBVM_FUNCTION_FLAGS_VIRTUAL, NULL, sysbvm_tuple_primitive_defaultPrintString);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.anyValueType, "asString", 1, SYSBVM_FUNCTION_FLAGS_VIRTUAL, NULL, sysbvm_tuple_primitive_defaultAsString);
    sysbvm_type_setMethodWithSelector(context, context->roots.stringType, sysbvm_symbol_internWithCString(context, "="), context->roots.stringEqualsFunction);
    sysbvm_type_setMethodWithSelector(context, context->roots.stringType, sysbvm_symbol_internWithCString(context, "hash"), context->roots.stringHashFunction);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.stringType, "--", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_string_primitive_concat);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.stringType, "withoutSuffix:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_string_primitive_withoutSuffix);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.stringType, "asSymbol", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_symbol_primitive_intern);
}
