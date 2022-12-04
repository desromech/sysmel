#include "tuuvm/integer.h"
#include "tuuvm/errors.h"
#include "tuuvm/function.h"
#include "tuuvm/string.h"
#include "tuuvm/type.h"
#include "internal/context.h"
#include <string.h>
#include <stdio.h>

TUUVM_API tuuvm_tuple_t tuuvm_tuple_integer_encodeBigInt32(tuuvm_context_t *context, int32_t value)
{
    /*tuuvm_object_tuple_t *result = tuuvm_context_allocateByteTuple(context, context->roots.integerType, sizeof(int64_t));
    *((int64_t*)result->bytes) = value;
    return (tuuvm_tuple_t)result;*/
    return TUUVM_NULL_TUPLE;
}

TUUVM_API tuuvm_tuple_t tuuvm_tuple_integer_encodeBigInt64(tuuvm_context_t *context, int64_t value)
{
    /*tuuvm_object_tuple_t *result = tuuvm_context_allocateByteTuple(context, context->roots.integerType, sizeof(int64_t));
    *((int64_t*)result->bytes) = value;
    return (tuuvm_tuple_t)result;*/
    return TUUVM_NULL_TUPLE;
}

/**
 * Parses an integer from a string.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_integer_parseString(tuuvm_context_t *context, size_t stringSize, const char *string)
{
    size_t index = 0;
    bool isNegative = false;

    // Parse the sign.
    if(index < stringSize)
    {
        if(string[index] == '-')
        {
            isNegative = true;
            ++index;
        }
        else if(string[index] == '+')
        {
            ++index;
        }
    }

    tuuvm_tuple_t result = tuuvm_tuple_integer_encodeSmall(0);
    tuuvm_tuple_t ten = tuuvm_tuple_integer_encodeSmall(10);
    for(; index < stringSize; ++index)
    {
        char digit = string[index];
        if('0' <= digit && digit <= '9')
        {
            tuuvm_tuple_t digitValue = tuuvm_tuple_integer_encodeSmall(digit - '0');
            result = tuuvm_integer_multiply(context, result, ten);
            result = tuuvm_integer_add(context, result, digitValue);
        }
    }

    if(isNegative)
        return tuuvm_integer_negate(context, result);

    return result;
}

TUUVM_API tuuvm_tuple_t tuuvm_integer_parseCString(tuuvm_context_t *context, const char *cstring)
{
    return tuuvm_integer_parseString(context, strlen(cstring), cstring);
}

TUUVM_API tuuvm_tuple_t tuuvm_integer_add(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right)
{
    if(tuuvm_tuple_isImmediate(left) && tuuvm_tuple_isImmediate(right))
    {
        tuuvm_stuple_t leftValue = tuuvm_tuple_integer_decodeSmall(left);
        tuuvm_stuple_t rightValue = tuuvm_tuple_integer_decodeSmall(right);
        return tuuvm_tuple_integer_encodeInt64(context, leftValue + rightValue);
    }

    // TODO: Implement the large integer addition.
    return TUUVM_NULL_TUPLE;
}

TUUVM_API tuuvm_tuple_t tuuvm_integer_subtract(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right)
{
    if(tuuvm_tuple_isImmediate(left) && tuuvm_tuple_isImmediate(right))
    {
        tuuvm_stuple_t leftValue = tuuvm_tuple_integer_decodeSmall(left);
        tuuvm_stuple_t rightValue = tuuvm_tuple_integer_decodeSmall(right);
        return tuuvm_tuple_integer_encodeInt64(context, leftValue - rightValue);
    }

    // TODO: Implement the large integer subtraction.
    return TUUVM_NULL_TUPLE;
}

TUUVM_API tuuvm_tuple_t tuuvm_integer_negate(tuuvm_context_t *context, tuuvm_tuple_t integer)
{
    if(tuuvm_tuple_isImmediate(integer))
    {
        tuuvm_stuple_t integerValue = tuuvm_tuple_integer_decodeSmall(integer);
        return tuuvm_tuple_integer_encodeInt64(context, -integerValue);
    }

    // TODO: Implement the large integer negation.
    return TUUVM_NULL_TUPLE;
}

TUUVM_API tuuvm_tuple_t tuuvm_integer_multiply(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right)
{
    if(tuuvm_tuple_isImmediate(left) && tuuvm_tuple_isImmediate(right))
    {
        tuuvm_stuple_t leftValue = tuuvm_tuple_integer_decodeSmall(left);
        tuuvm_stuple_t rightValue = tuuvm_tuple_integer_decodeSmall(right);

        // TODO: Implement integer overflow checking.
        return tuuvm_tuple_integer_encodeInt64(context, leftValue * rightValue);
    }

    // TODO: Implement the large integer multiplication.
    return TUUVM_NULL_TUPLE;
}

TUUVM_API tuuvm_tuple_t tuuvm_integer_divide(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right)
{
    if(tuuvm_tuple_isImmediate(left) && tuuvm_tuple_isImmediate(right))
    {
        tuuvm_stuple_t leftValue = tuuvm_tuple_integer_decodeSmall(left);
        tuuvm_stuple_t rightValue = tuuvm_tuple_integer_decodeSmall(right);
        return tuuvm_tuple_integer_encodeSmall(leftValue / rightValue);
    }

    return TUUVM_NULL_TUPLE;
}

TUUVM_API tuuvm_tuple_t tuuvm_integer_remainder(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right)
{
    if(tuuvm_tuple_isImmediate(left) && tuuvm_tuple_isImmediate(right))
    {
        tuuvm_stuple_t leftValue = tuuvm_tuple_integer_decodeSmall(left);
        tuuvm_stuple_t rightValue = tuuvm_tuple_integer_decodeSmall(right);
        return tuuvm_tuple_integer_encodeSmall(leftValue % rightValue);
    }

    return TUUVM_NULL_TUPLE;
}

TUUVM_API tuuvm_tuple_t tuuvm_integer_printString(tuuvm_context_t *context, tuuvm_tuple_t integer)
{
    // Decode the small integer.
    if(tuuvm_tuple_isImmediate(integer))
    {
        tuuvm_stuple_t value = tuuvm_tuple_integer_decodeSmall(integer);
        char buffer[64];
        size_t bufferSize = 0;

        // Work with positive integers.
        bool isNegative = false;
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

    return tuuvm_string_createWithCString(context, "TODO: tuuvm_integer_printString for large integer.");
}

tuuvm_tuple_t tuuvm_integer_primitive_printString(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);
    return tuuvm_integer_printString(context, arguments[0]);
}

void tuuvm_integer_setupPrimitives(tuuvm_context_t *context)
{
    tuuvm_type_setPrintStringFunction(context->roots.integerType, tuuvm_function_createPrimitive(context, 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_integer_primitive_printString));
}
