#include "tuuvm/integer.h"
#include "tuuvm/errors.h"
#include "tuuvm/function.h"
#include "tuuvm/string.h"
#include "tuuvm/type.h"
#include "internal/context.h"
#include <string.h>
#include <stdio.h>

#define CONCAT_SYMBOLS_(prefix, suffix) prefix ## _ ## suffix
#define CONCAT_SYMBOLS(prefix, suffix) CONCAT_SYMBOLS_(prefix, suffix)
#define PRIMITIVE_INTEGER_FUNCTION(name) CONCAT_SYMBOLS(FUNCTION_PREFIX, name)

static tuuvm_tuple_t tuuvm_primitiveInteger_signed_printString(tuuvm_context_t *context, int64_t integer)
{
    char buffer[64];
    size_t bufferSize = 0;

    // Work with positive integers.
    bool isNegative = false;
    int64_t value = integer;
    if(value < 0)
    {
        isNegative = true;
        value = -value;
    }

    // Extract each one of the digits.
    while (value != 0 || bufferSize == 0)
    {
        buffer[bufferSize++] = '0' + (value % 10);
        value /= 10;
    }

    // Add the sign.
    if(isNegative)
        buffer[bufferSize++] = '-';
    buffer[bufferSize] = 0;
    return tuuvm_string_createWithReversedString(context, bufferSize, buffer);
}

static tuuvm_tuple_t tuuvm_primitiveInteger_unsigned_printString(tuuvm_context_t *context, uint64_t integer)
{
    char buffer[64];
    size_t bufferSize = 0;

    // Extract each one of the digits.
    uint64_t value = integer;
    while (value != 0 || bufferSize == 0)
    {
        buffer[bufferSize++] = '0' + (value % 10);
        value /= 10;
    }

    buffer[bufferSize] = 0;
    return tuuvm_string_createWithReversedString(context, bufferSize, buffer);
}

#define integer_t char
#define primitiveInteger_decode tuuvm_tuple_char8_decode
#define primitiveInteger_encode tuuvm_tuple_char8_encode
#define IS_SIGNED false
#define FUNCTION_PREFIX tuuvm_char8
#define INTEGER_TYPE_NAME "Char8"
#define INTEGER_TYPE_ROOT_NAME char8Type
#define INTEGER_TYPE_SHORT_SUFFIX_NAME "c8"
#include "primitiveIntegers.inc"

#define integer_t uint8_t
#define primitiveInteger_decode tuuvm_tuple_uint8_decode
#define primitiveInteger_encode tuuvm_tuple_uint8_encode
#define IS_SIGNED false
#define FUNCTION_PREFIX tuuvm_uint8
#define INTEGER_TYPE_NAME "UInt8"
#define INTEGER_TYPE_ROOT_NAME uint8Type
#define INTEGER_TYPE_SHORT_SUFFIX_NAME "u8"
#include "primitiveIntegers.inc"

#define integer_t int8_t
#define primitiveInteger_decode tuuvm_tuple_int8_decode
#define primitiveInteger_encode tuuvm_tuple_int8_encode
#define IS_SIGNED true
#define FUNCTION_PREFIX tuuvm_int8
#define INTEGER_TYPE_NAME "Int8"
#define INTEGER_TYPE_ROOT_NAME int8Type
#define INTEGER_TYPE_SHORT_SUFFIX_NAME "i8"
#include "primitiveIntegers.inc"

#define integer_t uint16_t
#define primitiveInteger_decode tuuvm_tuple_char16_decode
#define primitiveInteger_encode tuuvm_tuple_char16_encode
#define IS_SIGNED false
#define FUNCTION_PREFIX tuuvm_char16
#define INTEGER_TYPE_NAME "Char16"
#define INTEGER_TYPE_ROOT_NAME char16Type
#define INTEGER_TYPE_SHORT_SUFFIX_NAME "c16"
#include "primitiveIntegers.inc"

#define integer_t uint16_t
#define primitiveInteger_decode tuuvm_tuple_uint16_decode
#define primitiveInteger_encode tuuvm_tuple_uint16_encode
#define IS_SIGNED false
#define FUNCTION_PREFIX tuuvm_uint16
#define INTEGER_TYPE_NAME "UInt16"
#define INTEGER_TYPE_ROOT_NAME uint16Type
#define INTEGER_TYPE_SHORT_SUFFIX_NAME "u16"
#include "primitiveIntegers.inc"

#define integer_t int16_t
#define primitiveInteger_decode tuuvm_tuple_int16_decode
#define primitiveInteger_encode tuuvm_tuple_int16_encode
#define IS_SIGNED true
#define FUNCTION_PREFIX tuuvm_int16
#define INTEGER_TYPE_NAME "Int16"
#define INTEGER_TYPE_ROOT_NAME int16Type
#define INTEGER_TYPE_SHORT_SUFFIX_NAME "i16"
#include "primitiveIntegers.inc"

#define integer_t uint32_t
#define primitiveInteger_decode tuuvm_tuple_char32_decode
#define primitiveInteger_encode(v) tuuvm_tuple_char32_encode(context, v)
#define IS_SIGNED false
#define FUNCTION_PREFIX tuuvm_char32
#define INTEGER_TYPE_NAME "Char32"
#define INTEGER_TYPE_ROOT_NAME char32Type
#define INTEGER_TYPE_SHORT_SUFFIX_NAME "c32"
#include "primitiveIntegers.inc"

#define integer_t uint32_t
#define primitiveInteger_decode tuuvm_tuple_uint32_decode
#define primitiveInteger_encode(v) tuuvm_tuple_uint32_encode(context, v)
#define IS_SIGNED false
#define FUNCTION_PREFIX tuuvm_uint32
#define INTEGER_TYPE_NAME "UInt32"
#define INTEGER_TYPE_ROOT_NAME uint32Type
#define INTEGER_TYPE_SHORT_SUFFIX_NAME "u32"
#include "primitiveIntegers.inc"

#define integer_t int32_t
#define primitiveInteger_decode tuuvm_tuple_int32_decode
#define primitiveInteger_encode(v) tuuvm_tuple_int32_encode(context, v)
#define IS_SIGNED true
#define FUNCTION_PREFIX tuuvm_int32
#define INTEGER_TYPE_NAME "Int32"
#define INTEGER_TYPE_ROOT_NAME int32Type
#define INTEGER_TYPE_SHORT_SUFFIX_NAME "i32"
#include "primitiveIntegers.inc"

#define integer_t uint64_t
#define primitiveInteger_decode tuuvm_tuple_uint64_decode
#define primitiveInteger_encode(v) tuuvm_tuple_uint64_encode(context, v)
#define IS_SIGNED false
#define FUNCTION_PREFIX tuuvm_uint64
#define INTEGER_TYPE_NAME "UInt64"
#define INTEGER_TYPE_ROOT_NAME uint64Type
#define INTEGER_TYPE_SHORT_SUFFIX_NAME "u64"
#include "primitiveIntegers.inc"

#define integer_t int64_t
#define primitiveInteger_decode tuuvm_tuple_int64_decode
#define primitiveInteger_encode(v) tuuvm_tuple_int64_encode(context, v)
#define IS_SIGNED true
#define FUNCTION_PREFIX tuuvm_int64
#define INTEGER_TYPE_NAME "Int64"
#define INTEGER_TYPE_ROOT_NAME int64Type
#define INTEGER_TYPE_SHORT_SUFFIX_NAME "i64"
#include "primitiveIntegers.inc"

void tuuvm_primitiveInteger_registerPrimitives(void)
{
    tuuvm_char8_registerPrimitives();
    tuuvm_uint8_registerPrimitives();
    tuuvm_int8_registerPrimitives();

    tuuvm_char16_registerPrimitives();
    tuuvm_uint16_registerPrimitives();
    tuuvm_int16_registerPrimitives();

    tuuvm_char32_registerPrimitives();
    tuuvm_uint32_registerPrimitives();
    tuuvm_int32_registerPrimitives();

    tuuvm_uint64_registerPrimitives();
    tuuvm_int64_registerPrimitives();
}

void tuuvm_primitiveInteger_setupPrimitives(tuuvm_context_t *context)
{
    tuuvm_char8_setupPrimitives(context);
    tuuvm_uint8_setupPrimitives(context);
    tuuvm_int8_setupPrimitives(context);

    tuuvm_char16_setupPrimitives(context);
    tuuvm_uint16_setupPrimitives(context);
    tuuvm_int16_setupPrimitives(context);

    tuuvm_char32_setupPrimitives(context);
    tuuvm_uint32_setupPrimitives(context);
    tuuvm_int32_setupPrimitives(context);

    tuuvm_uint64_setupPrimitives(context);
    tuuvm_int64_setupPrimitives(context);

    if(sizeof(tuuvm_bitflags_t) == 4)
    {
        tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Bitflags::fromInteger", context->roots.integerType, "bflgs", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_uint32_primitive_fromInteger);
    }
    else
    {
        tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Bitflags::fromInteger", context->roots.integerType, "bflgs", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_uint64_primitive_fromInteger);
    }

    if(context->roots.sizeType == context->roots.uint32Type)
    {
        tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "UIntPointer::fromInteger", context->roots.integerType, "uptr", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_uint32_primitive_fromInteger);
        tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "IntPointer::fromInteger", context->roots.integerType, "iptr", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_int32_primitive_fromInteger);
        tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Size::fromInteger", context->roots.integerType, "sz", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_uint32_primitive_fromInteger);
        tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "Size::+", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_uint32_primitive_add);
    }
    else
    {
        tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "UIntPointer::fromInteger", context->roots.integerType, "uptr", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_uint64_primitive_fromInteger);
        tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "IntPointer::fromInteger", context->roots.integerType, "iptr", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_int64_primitive_fromInteger);
        tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Size::fromInteger", context->roots.integerType, "sz", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_uint64_primitive_fromInteger);
        tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "Size::+", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_uint64_primitive_add);
    }
}
