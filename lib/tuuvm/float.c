#include "tuuvm/float.h"
#include "tuuvm/errors.h"
#include "tuuvm/function.h"
#include "tuuvm/string.h"
#include "tuuvm/type.h"
#include "internal/context.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

TUUVM_API float tuuvm_tuple_float32_decode(tuuvm_tuple_t tuple)
{
    if(tuuvm_tuple_isNonNullPointer(tuple))
    {
        tuuvm_object_tuple_t *floatObject = (tuuvm_object_tuple_t*)tuple;
        return *((tuuvm_float32_t*)floatObject->bytes);
    }
    else
    {
        uint32_t floatBits = (uint32_t)(tuple >> TUUVM_TUPLE_TAG_BIT_COUNT);
        float result = 0;
        memcpy(&result, &floatBits, 4);
        return result;
    }
}

TUUVM_API tuuvm_tuple_t tuuvm_tuple_float32_encode(tuuvm_context_t *context, float value)
{
    if(sizeof(float) < sizeof(tuuvm_tuple_t))
    {
        uint32_t floatBits = 0;
        memcpy(&floatBits, &value, 4);
        return ((tuuvm_tuple_t)floatBits << TUUVM_TUPLE_TAG_BIT_COUNT) | TUUVM_TUPLE_TAG_FLOAT32;
    }
    else
    {
        tuuvm_object_tuple_t *result = tuuvm_context_allocateByteTuple(context, context->roots.float32Type, sizeof(tuuvm_float32_t));
        *((tuuvm_float32_t*)result->bytes) = value;
        return (tuuvm_tuple_t)result;
    }
}

TUUVM_API tuuvm_tuple_t tuuvm_float32_parseCString(tuuvm_context_t *context, const char *cstring)
{
    return tuuvm_tuple_float32_encode(context, (float)atof(cstring));
}

TUUVM_API tuuvm_tuple_t tuuvm_float32_parseString(tuuvm_context_t *context, size_t stringSize, const char *string)
{
    char *buffer = (char*)malloc(stringSize + 1);
    memcpy(buffer, string, stringSize);
    buffer[stringSize] = 0;

    tuuvm_tuple_t result = tuuvm_float32_parseCString(context, buffer);
    free(buffer);
    return result;
}

TUUVM_API double tuuvm_tuple_float64_decode(tuuvm_tuple_t tuple)
{
    if(tuuvm_tuple_isNonNullPointer(tuple))
    {
        tuuvm_object_tuple_t *floatObject = (tuuvm_object_tuple_t*)tuple;
        return *((tuuvm_float64_t*)floatObject->bytes);
    }
    else
    {
        // TODO: Use the Sista encoding (See: https://clementbera.wordpress.com/2018/11/09/64-bits-immediate-floats/)
        return 0.0;
    }
}

TUUVM_API tuuvm_tuple_t tuuvm_tuple_float64_encode(tuuvm_context_t *context, double value)
{
    // TODO: Use the Sista encoding (See: https://clementbera.wordpress.com/2018/11/09/64-bits-immediate-floats/)
    tuuvm_object_tuple_t *result = tuuvm_context_allocateByteTuple(context, context->roots.float64Type, sizeof(tuuvm_float64_t));
    *((tuuvm_float64_t*)result->bytes) = value;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_float64_parseCString(tuuvm_context_t *context, const char *cstring)
{
    return tuuvm_tuple_float64_encode(context, atof(cstring));
}

TUUVM_API tuuvm_tuple_t tuuvm_float64_parseString(tuuvm_context_t *context, size_t stringSize, const char *string)
{
    char *buffer = (char*)malloc(stringSize + 1);
    memcpy(buffer, string, stringSize);
    buffer[stringSize] = 0;

    tuuvm_tuple_t result = tuuvm_float64_parseCString(context, buffer);
    free(buffer);
    return result;
}

static tuuvm_tuple_t tuuvm_float32_primitive_printString(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    char buffer[32] = {0};
    tuuvm_float32_t value = tuuvm_tuple_float32_decode(arguments[0]);

    int stringSize = snprintf(buffer, sizeof(buffer), "%g", value);
    if(stringSize < 0)
        return tuuvm_string_createEmptyWithSize(context, 0);
    else
        return tuuvm_string_createWithString(context, stringSize, buffer);
}

static tuuvm_tuple_t tuuvm_float32_primitive_fromFloat64(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_float32_encode(context, (float)tuuvm_tuple_float64_decode(arguments[0]));
}

static tuuvm_tuple_t tuuvm_float32_primitive_add(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_float32_t left = tuuvm_tuple_float32_decode(arguments[0]);
    tuuvm_float32_t right = tuuvm_tuple_float32_decode(arguments[1]);
    return tuuvm_tuple_float32_encode(context, left + right);
}

static tuuvm_tuple_t tuuvm_float32_primitive_subtract(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_float32_t left = tuuvm_tuple_float32_decode(arguments[0]);
    tuuvm_float32_t right = tuuvm_tuple_float32_decode(arguments[1]);
    return tuuvm_tuple_float32_encode(context, left - right);
}

static tuuvm_tuple_t tuuvm_float32_primitive_negated(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    tuuvm_float32_t operand = tuuvm_tuple_float32_decode(arguments[0]);
    return tuuvm_tuple_float32_encode(context, -operand);
}

static tuuvm_tuple_t tuuvm_float32_primitive_sqrt(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    tuuvm_float32_t operand = tuuvm_tuple_float32_decode(arguments[0]);
    return tuuvm_tuple_float32_encode(context, sqrtf(operand));
}

static tuuvm_tuple_t tuuvm_float32_primitive_multiply(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_float32_t left = tuuvm_tuple_float32_decode(arguments[0]);
    tuuvm_float32_t right = tuuvm_tuple_float32_decode(arguments[1]);
    return tuuvm_tuple_float32_encode(context, left * right);
}

static tuuvm_tuple_t tuuvm_float32_primitive_divide(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_float32_t left = tuuvm_tuple_float32_decode(arguments[0]);
    tuuvm_float32_t right = tuuvm_tuple_float32_decode(arguments[1]);
    return tuuvm_tuple_float32_encode(context, left / right);
}

static tuuvm_tuple_t tuuvm_float32_primitive_compare(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_float32_t left = tuuvm_tuple_float32_decode(arguments[0]);
    tuuvm_float32_t right = tuuvm_tuple_float32_decode(arguments[1]);
    if(left < right)
        return tuuvm_tuple_integer_encodeSmall(-1);
    else if(left > right)
        return tuuvm_tuple_integer_encodeSmall(1);
    else
        return tuuvm_tuple_integer_encodeSmall(0);
}

static tuuvm_tuple_t tuuvm_float32_primitive_equals(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_float32_t left = tuuvm_tuple_float32_decode(arguments[0]);
    tuuvm_float32_t right = tuuvm_tuple_float32_decode(arguments[1]);
    return tuuvm_tuple_boolean_encode(left == right);
}

static tuuvm_tuple_t tuuvm_float32_primitive_notEquals(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_float32_t left = tuuvm_tuple_float32_decode(arguments[0]);
    tuuvm_float32_t right = tuuvm_tuple_float32_decode(arguments[1]);
    return tuuvm_tuple_boolean_encode(left != right);
}

static tuuvm_tuple_t tuuvm_float32_primitive_lessThan(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_float32_t left = tuuvm_tuple_float32_decode(arguments[0]);
    tuuvm_float32_t right = tuuvm_tuple_float32_decode(arguments[1]);
    return tuuvm_tuple_boolean_encode(left < right);
}

static tuuvm_tuple_t tuuvm_float32_primitive_lessEquals(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_float32_t left = tuuvm_tuple_float32_decode(arguments[0]);
    tuuvm_float32_t right = tuuvm_tuple_float32_decode(arguments[1]);
    return tuuvm_tuple_boolean_encode(left <= right);
}

static tuuvm_tuple_t tuuvm_float32_primitive_greaterThan(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_float32_t left = tuuvm_tuple_float32_decode(arguments[0]);
    tuuvm_float32_t right = tuuvm_tuple_float32_decode(arguments[1]);
    return tuuvm_tuple_boolean_encode(left > right);
}

static tuuvm_tuple_t tuuvm_float32_primitive_greaterEquals(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_float32_t left = tuuvm_tuple_float32_decode(arguments[0]);
    tuuvm_float32_t right = tuuvm_tuple_float32_decode(arguments[1]);
    return tuuvm_tuple_boolean_encode(left >= right);
}

static tuuvm_tuple_t tuuvm_float32_primitive_asUInt8(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_uint8_encode((uint8_t)tuuvm_tuple_float32_decode(arguments[0]));
}

static tuuvm_tuple_t tuuvm_float32_primitive_asInt8(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_int8_encode((int8_t)tuuvm_tuple_float32_decode(arguments[0]));
}

static tuuvm_tuple_t tuuvm_float32_primitive_asChar8(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_char8_encode((tuuvm_char8_t)tuuvm_tuple_float32_decode(arguments[0]));
}

static tuuvm_tuple_t tuuvm_float32_primitive_asUInt16(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_uint16_encode((uint16_t)tuuvm_tuple_float32_decode(arguments[0]));
}

static tuuvm_tuple_t tuuvm_float32_primitive_asInt16(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_int16_encode((int16_t)tuuvm_tuple_float32_decode(arguments[0]));
}

static tuuvm_tuple_t tuuvm_float32_primitive_asChar16(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_char16_encode((tuuvm_char16_t)tuuvm_tuple_float32_decode(arguments[0]));
}

static tuuvm_tuple_t tuuvm_float32_primitive_asUInt32(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_uint32_encode(context, (uint32_t)tuuvm_tuple_float32_decode(arguments[0]));
}

static tuuvm_tuple_t tuuvm_float32_primitive_asInt32(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_int32_encode(context, (int32_t)tuuvm_tuple_float32_decode(arguments[0]));
}

static tuuvm_tuple_t tuuvm_float32_primitive_asChar32(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_char32_encode(context, (tuuvm_char32_t)tuuvm_tuple_float32_decode(arguments[0]));
}

static tuuvm_tuple_t tuuvm_float32_primitive_asUInt64(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_uint64_encode(context, (uint64_t)tuuvm_tuple_float32_decode(arguments[0]));
}

static tuuvm_tuple_t tuuvm_float32_primitive_asInt64(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_int64_encode(context, (int64_t)tuuvm_tuple_float32_decode(arguments[0]));
}

static tuuvm_tuple_t tuuvm_float64_primitive_printString(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    char buffer[32] = {0};
    tuuvm_float64_t value = tuuvm_tuple_float64_decode(arguments[0]);

    int stringSize = snprintf(buffer, sizeof(buffer), "%g", value);
    if(stringSize < 0)
        return tuuvm_string_createEmptyWithSize(context, 0);
    else
        return tuuvm_string_createWithString(context, stringSize, buffer);
}

static tuuvm_tuple_t tuuvm_float64_primitive_fromFloat32(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_float64_encode(context, tuuvm_tuple_float32_decode(arguments[0]));
}

static tuuvm_tuple_t tuuvm_float64_primitive_add(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_float64_t left = tuuvm_tuple_float64_decode(arguments[0]);
    tuuvm_float64_t right = tuuvm_tuple_float64_decode(arguments[1]);
    return tuuvm_tuple_float64_encode(context, left + right);
}

static tuuvm_tuple_t tuuvm_float64_primitive_subtract(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_float64_t left = tuuvm_tuple_float64_decode(arguments[0]);
    tuuvm_float64_t right = tuuvm_tuple_float64_decode(arguments[1]);
    return tuuvm_tuple_float64_encode(context, left - right);
}

static tuuvm_tuple_t tuuvm_float64_primitive_negated(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    tuuvm_float64_t operand = tuuvm_tuple_float64_decode(arguments[0]);
    return tuuvm_tuple_float64_encode(context, -operand);
}

static tuuvm_tuple_t tuuvm_float64_primitive_sqrt(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    tuuvm_float64_t operand = tuuvm_tuple_float64_decode(arguments[0]);
    return tuuvm_tuple_float64_encode(context, sqrt(operand));
}

static tuuvm_tuple_t tuuvm_float64_primitive_multiply(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_float64_t left = tuuvm_tuple_float64_decode(arguments[0]);
    tuuvm_float64_t right = tuuvm_tuple_float64_decode(arguments[1]);
    return tuuvm_tuple_float64_encode(context, left * right);
}

static tuuvm_tuple_t tuuvm_float64_primitive_divide(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_float64_t left = tuuvm_tuple_float64_decode(arguments[0]);
    tuuvm_float64_t right = tuuvm_tuple_float64_decode(arguments[1]);
    return tuuvm_tuple_float64_encode(context, left / right);
}

static tuuvm_tuple_t tuuvm_float64_primitive_compare(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_float64_t left = tuuvm_tuple_float64_decode(arguments[0]);
    tuuvm_float64_t right = tuuvm_tuple_float64_decode(arguments[1]);
    if(left < right)
        return tuuvm_tuple_integer_encodeSmall(-1);
    else if(left > right)
        return tuuvm_tuple_integer_encodeSmall(1);
    else
        return tuuvm_tuple_integer_encodeSmall(0);
}

static tuuvm_tuple_t tuuvm_float64_primitive_equals(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_float64_t left = tuuvm_tuple_float64_decode(arguments[0]);
    tuuvm_float64_t right = tuuvm_tuple_float64_decode(arguments[1]);
    return tuuvm_tuple_boolean_encode(left == right);
}

static tuuvm_tuple_t tuuvm_float64_primitive_notEquals(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_float64_t left = tuuvm_tuple_float64_decode(arguments[0]);
    tuuvm_float64_t right = tuuvm_tuple_float64_decode(arguments[1]);
    return tuuvm_tuple_boolean_encode(left != right);
}

static tuuvm_tuple_t tuuvm_float64_primitive_lessThan(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_float64_t left = tuuvm_tuple_float64_decode(arguments[0]);
    tuuvm_float64_t right = tuuvm_tuple_float64_decode(arguments[1]);
    return tuuvm_tuple_boolean_encode(left < right);
}

static tuuvm_tuple_t tuuvm_float64_primitive_lessEquals(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_float64_t left = tuuvm_tuple_float64_decode(arguments[0]);
    tuuvm_float64_t right = tuuvm_tuple_float64_decode(arguments[1]);
    return tuuvm_tuple_boolean_encode(left <= right);
}

static tuuvm_tuple_t tuuvm_float64_primitive_greaterThan(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_float64_t left = tuuvm_tuple_float64_decode(arguments[0]);
    tuuvm_float64_t right = tuuvm_tuple_float64_decode(arguments[1]);
    return tuuvm_tuple_boolean_encode(left > right);
}

static tuuvm_tuple_t tuuvm_float64_primitive_greaterEquals(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_float64_t left = tuuvm_tuple_float64_decode(arguments[0]);
    tuuvm_float64_t right = tuuvm_tuple_float64_decode(arguments[1]);
    return tuuvm_tuple_boolean_encode(left >= right);
}

static tuuvm_tuple_t tuuvm_float64_primitive_asUInt8(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_uint8_encode((uint8_t)tuuvm_tuple_float64_decode(arguments[0]));
}

static tuuvm_tuple_t tuuvm_float64_primitive_asInt8(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_int8_encode((int8_t)tuuvm_tuple_float64_decode(arguments[0]));
}

static tuuvm_tuple_t tuuvm_float64_primitive_asChar8(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_char8_encode((tuuvm_char8_t)tuuvm_tuple_float64_decode(arguments[0]));
}

static tuuvm_tuple_t tuuvm_float64_primitive_asUInt16(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_uint16_encode((uint16_t)tuuvm_tuple_float64_decode(arguments[0]));
}

static tuuvm_tuple_t tuuvm_float64_primitive_asInt16(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_int16_encode((int16_t)tuuvm_tuple_float64_decode(arguments[0]));
}

static tuuvm_tuple_t tuuvm_float64_primitive_asChar16(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_char16_encode((tuuvm_char16_t)tuuvm_tuple_float64_decode(arguments[0]));
}

static tuuvm_tuple_t tuuvm_float64_primitive_asUInt32(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_uint32_encode(context, (uint32_t)tuuvm_tuple_float64_decode(arguments[0]));
}

static tuuvm_tuple_t tuuvm_float64_primitive_asInt32(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_int32_encode(context, (int32_t)tuuvm_tuple_float64_decode(arguments[0]));
}

static tuuvm_tuple_t tuuvm_float64_primitive_asChar32(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_char32_encode(context, (tuuvm_char32_t)tuuvm_tuple_float64_decode(arguments[0]));
}

static tuuvm_tuple_t tuuvm_float64_primitive_asUInt64(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_uint64_encode(context, (uint64_t)tuuvm_tuple_float64_decode(arguments[0]));
}

static tuuvm_tuple_t tuuvm_float64_primitive_asInt64(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_int64_encode(context, (int64_t)tuuvm_tuple_float64_decode(arguments[0]));
}

void tuuvm_float_registerPrimitives(void)
{
    // Float32
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_printString, "Float32::printString");
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_fromFloat64, "Float32::fromFloat64");

    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_add, "Float32::+");
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_subtract, "Float32::-");
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_negated, "Float32::negated");
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_sqrt, "Float32::sqrt");
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_multiply, "Float32::*");
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_divide, "Float32::/");

    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_compare, "Float32::<=>");
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_equals, "Float32::=");
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_notEquals, "Float32::~=");
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_lessThan, "Float32::<");
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_lessEquals, "Float32::<=");
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_greaterThan, "Float32::>");
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_greaterEquals, "Float32::>=");

    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_asUInt8, "Float32::asUInt8");
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_asInt8, "Float32::asInt8");
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_asChar8, "Float32::asChar8");
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_asUInt16, "Float32::asUInt16");
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_asInt16, "Float32::asInt16");
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_asChar16, "Float32::asChar16");
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_asUInt32, "Float32::asUInt32");
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_asInt32, "Float32::asInt32");
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_asChar32, "Float32::asChar32");
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_asUInt64, "Float32::asUInt64");
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_asInt64, "Float32::asInt64");

    // Float64
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_printString, "Float64::printString");
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_fromFloat32, "Float64::fromFloat32");

    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_add, "Float64::+");
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_subtract, "Float64::-");
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_negated, "Float64::negated");
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_sqrt, "Float64::sqrt");
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_multiply, "Float64::*");
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_divide, "Float64::/");

    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_compare, "Float64::<=>");
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_equals, "Float64::=");
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_notEquals, "Float64::~=");
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_lessThan, "Float64::<");
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_lessEquals, "Float64::<=");
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_greaterThan, "Float64::>");
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_greaterEquals, "Float64::>=");

    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_asUInt8, "Float64::asUInt8");
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_asInt8, "Float64::asInt8");
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_asChar8, "Float64::asChar8");
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_asUInt16, "Float64::asUInt16");
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_asInt16, "Float64::asInt16");
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_asChar16, "Float64::asChar16");
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_asUInt32, "Float64::asUInt32");
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_asInt32, "Float64::asInt32");
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_asChar32, "Float64::asChar32");
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_asUInt64, "Float64::asUInt64");
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_asInt64, "Float64::asInt64");
}

void tuuvm_float_setupPrimitives(tuuvm_context_t *context)
{
    // Float32
    tuuvm_type_setPrintStringFunction(context, context->roots.float32Type, tuuvm_function_createPrimitive(context, 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_printString));

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::fromFloat64", context->roots.float64Type, "f32", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_fromFloat64);

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::+", context->roots.float32Type, "+", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_add);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::-", context->roots.float32Type, "-", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_subtract);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::negated", context->roots.float32Type, "negated", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_negated);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::sqrt", context->roots.float32Type, "sqrt", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_sqrt);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::*", context->roots.float32Type, "*", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_multiply);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::/", context->roots.float32Type, "/", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_divide);

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::<=>", context->roots.float32Type, "<=>", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_compare);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::=", context->roots.float32Type, "=", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_equals);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::~=", context->roots.float32Type, "~=", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_notEquals);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::<", context->roots.float32Type, "<", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_lessThan);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::<=", context->roots.float32Type, "<=", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_lessEquals);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::>", context->roots.float32Type, ">", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_greaterThan);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::>=", context->roots.float32Type, ">=", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_greaterEquals);

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::asUInt8", context->roots.float32Type, "asUInt8", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_asUInt8);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::asInt8", context->roots.float32Type, "asInt8", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_asInt8);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::asChar8", context->roots.float32Type, "asChar8", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_asChar8);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::asUInt16", context->roots.float32Type, "asUInt16", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_asUInt16);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::asInt16", context->roots.float32Type, "asInt16", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_asInt16);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::asChar16", context->roots.float32Type, "asChar16", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_asChar16);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::asUInt32", context->roots.float32Type, "asUInt32", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_asUInt32);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::asInt32", context->roots.float32Type, "asInt32", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_asInt32);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::asChar32", context->roots.float32Type, "asChar32", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_asChar32);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::asUInt64", context->roots.float32Type, "asUInt64", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_asUInt64);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::asInt64", context->roots.float32Type, "asInt64", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_asInt64);

    // Float64
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::fromFloat32", context->roots.float32Type, "f64", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_fromFloat32);

    tuuvm_type_setPrintStringFunction(context, context->roots.float64Type, tuuvm_function_createPrimitive(context, 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_printString));
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float64::+", context->roots.float64Type, "+", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_add);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float64::-", context->roots.float64Type, "-", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_subtract);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float64::negated", context->roots.float64Type, "negated", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_negated);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float64::sqrt", context->roots.float64Type, "sqrt", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_sqrt);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float64::*", context->roots.float64Type, "*", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_multiply);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float64::/", context->roots.float64Type, "/", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_divide);

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float64::<=>", context->roots.float64Type, "<=>", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_compare);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float64::=", context->roots.float64Type, "=", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_equals);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float64::~=", context->roots.float64Type, "~=", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_notEquals);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float64::<", context->roots.float64Type, "<", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_lessThan);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float64::<=", context->roots.float64Type, "<=", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_lessEquals);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float64::>", context->roots.float64Type, ">", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_greaterThan);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float64::>=", context->roots.float64Type, ">=", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_greaterEquals);

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float64::asUInt8", context->roots.float64Type, "asUInt8", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_asUInt8);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float64::asInt8", context->roots.float64Type, "asInt8", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_asInt8);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float64::asChar8", context->roots.float64Type, "asChar8", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_asChar8);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float64::asUInt16", context->roots.float64Type, "asUInt16", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_asUInt16);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float64::asInt16", context->roots.float64Type, "asInt16", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_asInt16);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float64::asChar16", context->roots.float64Type, "asChar16", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_asChar16);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float64::asUInt32", context->roots.float64Type, "asUInt32", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_asUInt32);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float64::asInt32", context->roots.float64Type, "asInt32", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_asInt32);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float64::asChar32", context->roots.float64Type, "asChar32", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_asChar32);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float64::asUInt64", context->roots.float64Type, "asUInt64", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_asUInt64);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float64::asInt64", context->roots.float64Type, "asInt64", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_asInt64);
}
