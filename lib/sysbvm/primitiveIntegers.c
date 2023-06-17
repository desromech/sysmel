#include "sysbvm/integer.h"
#include "sysbvm/errors.h"
#include "sysbvm/function.h"
#include "sysbvm/string.h"
#include "sysbvm/type.h"
#include "internal/context.h"
#include <string.h>
#include <stdio.h>

#define CONCAT_SYMBOLS_(prefix, suffix) prefix ## _ ## suffix
#define CONCAT_SYMBOLS(prefix, suffix) CONCAT_SYMBOLS_(prefix, suffix)
#define PRIMITIVE_INTEGER_FUNCTION(name) CONCAT_SYMBOLS(FUNCTION_PREFIX, name)

static sysbvm_tuple_t sysbvm_primitiveInteger_signed_printString(sysbvm_context_t *context, int64_t integer)
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
    return sysbvm_string_createWithReversedString(context, bufferSize, buffer);
}

static sysbvm_tuple_t sysbvm_primitiveInteger_unsigned_printString(sysbvm_context_t *context, uint64_t integer)
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
    return sysbvm_string_createWithReversedString(context, bufferSize, buffer);
}

#define integer_t char
#define primitiveInteger_decode sysbvm_tuple_char8_decode
#define primitiveInteger_encode sysbvm_tuple_char8_encode
#define primitiveInteger_highBit sysbvm_uint32_highBit
#define primitiveInteger_lowBit sysbvm_uint32_lowBit
#define IS_CHARACTER true
#define IS_SIGNED false
#define FUNCTION_PREFIX sysbvm_char8
#define INTEGER_TYPE_NAME "Char8"
#define INTEGER_TYPE_ROOT_NAME char8Type
#define INTEGER_TYPE_SHORT_SUFFIX_NAME "c8"
#include "primitiveIntegers.inc"

#define integer_t uint8_t
#define primitiveInteger_decode sysbvm_tuple_uint8_decode
#define primitiveInteger_encode sysbvm_tuple_uint8_encode
#define primitiveInteger_highBit sysbvm_uint32_highBit
#define primitiveInteger_lowBit sysbvm_uint32_lowBit
#define IS_CHARACTER false
#define IS_SIGNED false
#define FUNCTION_PREFIX sysbvm_uint8
#define INTEGER_TYPE_NAME "UInt8"
#define INTEGER_TYPE_ROOT_NAME uint8Type
#define INTEGER_TYPE_SHORT_SUFFIX_NAME "u8"
#include "primitiveIntegers.inc"

#define integer_t int8_t
#define primitiveInteger_decode sysbvm_tuple_int8_decode
#define primitiveInteger_encode sysbvm_tuple_int8_encode
#define primitiveInteger_highBit sysbvm_uint32_highBit
#define primitiveInteger_lowBit sysbvm_uint32_lowBit
#define IS_CHARACTER false
#define IS_SIGNED true
#define FUNCTION_PREFIX sysbvm_int8
#define INTEGER_TYPE_NAME "Int8"
#define INTEGER_TYPE_ROOT_NAME int8Type
#define INTEGER_TYPE_SHORT_SUFFIX_NAME "i8"
#include "primitiveIntegers.inc"

#define integer_t uint16_t
#define primitiveInteger_decode sysbvm_tuple_char16_decode
#define primitiveInteger_encode sysbvm_tuple_char16_encode
#define primitiveInteger_highBit sysbvm_uint32_highBit
#define primitiveInteger_lowBit sysbvm_uint32_lowBit
#define IS_CHARACTER true
#define IS_SIGNED false
#define FUNCTION_PREFIX sysbvm_char16
#define INTEGER_TYPE_NAME "Char16"
#define INTEGER_TYPE_ROOT_NAME char16Type
#define INTEGER_TYPE_SHORT_SUFFIX_NAME "c16"
#include "primitiveIntegers.inc"

#define integer_t uint16_t
#define primitiveInteger_decode sysbvm_tuple_uint16_decode
#define primitiveInteger_encode sysbvm_tuple_uint16_encode
#define primitiveInteger_highBit sysbvm_uint32_highBit
#define primitiveInteger_lowBit sysbvm_uint32_lowBit
#define IS_CHARACTER false
#define IS_SIGNED false
#define FUNCTION_PREFIX sysbvm_uint16
#define INTEGER_TYPE_NAME "UInt16"
#define INTEGER_TYPE_ROOT_NAME uint16Type
#define INTEGER_TYPE_SHORT_SUFFIX_NAME "u16"
#include "primitiveIntegers.inc"

#define integer_t int16_t
#define primitiveInteger_decode sysbvm_tuple_int16_decode
#define primitiveInteger_encode sysbvm_tuple_int16_encode
#define primitiveInteger_highBit sysbvm_uint32_highBit
#define primitiveInteger_lowBit sysbvm_uint32_lowBit
#define IS_CHARACTER false
#define IS_SIGNED true
#define FUNCTION_PREFIX sysbvm_int16
#define INTEGER_TYPE_NAME "Int16"
#define INTEGER_TYPE_ROOT_NAME int16Type
#define INTEGER_TYPE_SHORT_SUFFIX_NAME "i16"
#include "primitiveIntegers.inc"

#define integer_t uint32_t
#define primitiveInteger_decode sysbvm_tuple_char32_decode
#define primitiveInteger_encode(v) sysbvm_tuple_char32_encode(context, v)
#define primitiveInteger_highBit sysbvm_uint32_highBit
#define primitiveInteger_lowBit sysbvm_uint32_lowBit
#define IS_CHARACTER true
#define IS_SIGNED false
#define FUNCTION_PREFIX sysbvm_char32
#define INTEGER_TYPE_NAME "Char32"
#define INTEGER_TYPE_ROOT_NAME char32Type
#define INTEGER_TYPE_SHORT_SUFFIX_NAME "c32"
#include "primitiveIntegers.inc"

#define integer_t uint32_t
#define primitiveInteger_decode sysbvm_tuple_uint32_decode
#define primitiveInteger_encode(v) sysbvm_tuple_uint32_encode(context, v)
#define primitiveInteger_highBit sysbvm_uint32_highBit
#define primitiveInteger_lowBit sysbvm_uint32_lowBit
#define IS_CHARACTER false
#define IS_SIGNED false
#define FUNCTION_PREFIX sysbvm_uint32
#define INTEGER_TYPE_NAME "UInt32"
#define INTEGER_TYPE_ROOT_NAME uint32Type
#define INTEGER_TYPE_SHORT_SUFFIX_NAME "u32"
#include "primitiveIntegers.inc"

#define integer_t int32_t
#define primitiveInteger_decode sysbvm_tuple_int32_decode
#define primitiveInteger_encode(v) sysbvm_tuple_int32_encode(context, v)
#define primitiveInteger_highBit sysbvm_uint32_highBit
#define primitiveInteger_lowBit sysbvm_uint32_lowBit
#define IS_CHARACTER false
#define IS_SIGNED true
#define FUNCTION_PREFIX sysbvm_int32
#define INTEGER_TYPE_NAME "Int32"
#define INTEGER_TYPE_ROOT_NAME int32Type
#define INTEGER_TYPE_SHORT_SUFFIX_NAME "i32"
#include "primitiveIntegers.inc"

#define integer_t uint64_t
#define primitiveInteger_decode sysbvm_tuple_uint64_decode
#define primitiveInteger_encode(v) sysbvm_tuple_uint64_encode(context, v)
#define primitiveInteger_highBit sysbvm_uint64_highBit
#define primitiveInteger_lowBit sysbvm_uint64_lowBit
#define IS_CHARACTER false
#define IS_SIGNED false
#define FUNCTION_PREFIX sysbvm_uint64
#define INTEGER_TYPE_NAME "UInt64"
#define INTEGER_TYPE_ROOT_NAME uint64Type
#define INTEGER_TYPE_SHORT_SUFFIX_NAME "u64"
#include "primitiveIntegers.inc"

#define integer_t int64_t
#define primitiveInteger_decode sysbvm_tuple_int64_decode
#define primitiveInteger_encode(v) sysbvm_tuple_int64_encode(context, v)
#define primitiveInteger_highBit sysbvm_uint64_highBit
#define primitiveInteger_lowBit sysbvm_uint64_lowBit
#define IS_CHARACTER false
#define IS_SIGNED true
#define FUNCTION_PREFIX sysbvm_int64
#define INTEGER_TYPE_NAME "Int64"
#define INTEGER_TYPE_ROOT_NAME int64Type
#define INTEGER_TYPE_SHORT_SUFFIX_NAME "i64"
#include "primitiveIntegers.inc"

void sysbvm_primitiveInteger_registerPrimitives(void)
{
    sysbvm_char8_registerPrimitives();
    sysbvm_uint8_registerPrimitives();
    sysbvm_int8_registerPrimitives();

    sysbvm_char16_registerPrimitives();
    sysbvm_uint16_registerPrimitives();
    sysbvm_int16_registerPrimitives();

    sysbvm_char32_registerPrimitives();
    sysbvm_uint32_registerPrimitives();
    sysbvm_int32_registerPrimitives();

    sysbvm_uint64_registerPrimitives();
    sysbvm_int64_registerPrimitives();
}

void sysbvm_primitiveInteger_setupPrimitives(sysbvm_context_t *context)
{
    sysbvm_char8_setupPrimitives(context);
    sysbvm_uint8_setupPrimitives(context);
    sysbvm_int8_setupPrimitives(context);

    sysbvm_char16_setupPrimitives(context);
    sysbvm_uint16_setupPrimitives(context);
    sysbvm_int16_setupPrimitives(context);

    sysbvm_char32_setupPrimitives(context);
    sysbvm_uint32_setupPrimitives(context);
    sysbvm_int32_setupPrimitives(context);

    sysbvm_uint64_setupPrimitives(context);
    sysbvm_int64_setupPrimitives(context);

    if(sizeof(sysbvm_bitflags_t) == 4)
    {
        sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Bitflags::fromInteger", context->roots.integerType, "bflgs", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_uint32_primitive_fromInteger);
    }
    else
    {
        sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Bitflags::fromInteger", context->roots.integerType, "bflgs", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_uint64_primitive_fromInteger);
    }

    if(context->roots.sizeType == context->roots.uint32Type)
    {
        sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "UIntPointer::fromInteger", context->roots.integerType, "uptr", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_uint32_primitive_fromInteger);
        sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "IntPointer::fromInteger", context->roots.integerType, "iptr", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_int32_primitive_fromInteger);
        sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Size::fromInteger", context->roots.integerType, "sz", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_uint32_primitive_fromInteger);
        sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "Size::+"), sysbvm_type_lookupSelector(context, context->roots.uint32Type, sysbvm_symbol_internWithCString(context, "+")));
    }
    else
    {
        sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "UIntPointer::fromInteger", context->roots.integerType, "uptr", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_uint64_primitive_fromInteger);
        sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "IntPointer::fromInteger", context->roots.integerType, "iptr", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_int64_primitive_fromInteger);
        sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Size::fromInteger", context->roots.integerType, "sz", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_uint64_primitive_fromInteger);
        sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "Size::+"), sysbvm_type_lookupSelector(context, context->roots.uint64Type, sysbvm_symbol_internWithCString(context, "+")));
    }
}
