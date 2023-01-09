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

TUUVM_API tuuvm_tuple_t tuuvm_tuple_integer_encodeBigUInt64(tuuvm_context_t *context, uint64_t value)
{
    return TUUVM_NULL_TUPLE;
}

int64_t tuuvm_tuple_integer_decodeInt64(tuuvm_tuple_t value)
{
    if(tuuvm_tuple_isImmediate(value))
        return tuuvm_tuple_integer_decodeSmall(value);
    return 0;
}

uint64_t tuuvm_tuple_integer_decodeUInt64(tuuvm_tuple_t value)
{
    if(tuuvm_tuple_isImmediate(value))
        return tuuvm_tuple_integer_decodeSmall(value);
    return 0;
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

TUUVM_API tuuvm_tuple_t tuuvm_integer_compare(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right)
{
    if(tuuvm_tuple_isImmediate(left) && tuuvm_tuple_isImmediate(right))
    {
        tuuvm_stuple_t leftValue = tuuvm_tuple_integer_decodeSmall(left);
        tuuvm_stuple_t rightValue = tuuvm_tuple_integer_decodeSmall(right);
        if(leftValue < rightValue)
            return tuuvm_tuple_integer_encodeSmall(-1);
        else if(leftValue > rightValue)
            return tuuvm_tuple_integer_encodeSmall(1);
        else
            return tuuvm_tuple_integer_encodeSmall(0);
    }

    return TUUVM_NULL_TUPLE;
}

TUUVM_API tuuvm_tuple_t tuuvm_integer_equals(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right)
{
    tuuvm_stuple_t comparisonResult = tuuvm_tuple_integer_decodeSmall(tuuvm_integer_compare(context, left, right));
    return tuuvm_tuple_boolean_encode(comparisonResult == 0);
}

TUUVM_API tuuvm_tuple_t tuuvm_integer_notEquals(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right)
{
    tuuvm_stuple_t comparisonResult = tuuvm_tuple_integer_decodeSmall(tuuvm_integer_compare(context, left, right));
    return tuuvm_tuple_boolean_encode(comparisonResult != 0);
}

TUUVM_API tuuvm_tuple_t tuuvm_integer_lessThan(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right)
{
    tuuvm_stuple_t comparisonResult = tuuvm_tuple_integer_decodeSmall(tuuvm_integer_compare(context, left, right));
    return tuuvm_tuple_boolean_encode(comparisonResult < 0);
}

TUUVM_API tuuvm_tuple_t tuuvm_integer_lessEquals(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right)
{
    tuuvm_stuple_t comparisonResult = tuuvm_tuple_integer_decodeSmall(tuuvm_integer_compare(context, left, right));
    return tuuvm_tuple_boolean_encode(comparisonResult <= 0);
}

TUUVM_API tuuvm_tuple_t tuuvm_integer_greaterThan(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right)
{
    tuuvm_stuple_t comparisonResult = tuuvm_tuple_integer_decodeSmall(tuuvm_integer_compare(context, left, right));
    return tuuvm_tuple_boolean_encode(comparisonResult > 0);
}

TUUVM_API tuuvm_tuple_t tuuvm_integer_greaterEquals(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right)
{
    tuuvm_stuple_t comparisonResult = tuuvm_tuple_integer_decodeSmall(tuuvm_integer_compare(context, left, right));
    return tuuvm_tuple_boolean_encode(comparisonResult >= 0);
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

static tuuvm_tuple_t tuuvm_integer_primitive_printString(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);
    return tuuvm_integer_printString(context, arguments[0]);
}

static tuuvm_tuple_t tuuvm_integer_primitive_add(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return tuuvm_integer_add(context, arguments[0], arguments[1]);
}

static tuuvm_tuple_t tuuvm_integer_primitive_subtract(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return tuuvm_integer_subtract(context, arguments[0], arguments[1]);
}

static tuuvm_tuple_t tuuvm_integer_primitive_negate(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_integer_negate(context, arguments[0]);
}

static tuuvm_tuple_t tuuvm_integer_primitive_multiply(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return tuuvm_integer_multiply(context, arguments[0], arguments[1]);
}

static tuuvm_tuple_t tuuvm_integer_primitive_divide(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return tuuvm_integer_divide(context, arguments[0], arguments[1]);
}

static tuuvm_tuple_t tuuvm_integer_primitive_remainder(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return tuuvm_integer_remainder(context, arguments[0], arguments[1]);
}

static tuuvm_tuple_t tuuvm_integer_primitive_compare(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return tuuvm_integer_compare(context, arguments[0], arguments[1]);
}

static tuuvm_tuple_t tuuvm_integer_primitive_equals(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return tuuvm_integer_equals(context, arguments[0], arguments[1]);
}

static tuuvm_tuple_t tuuvm_integer_primitive_notEquals(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return tuuvm_integer_notEquals(context, arguments[0], arguments[1]);
}

static tuuvm_tuple_t tuuvm_integer_primitive_lessThan(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return tuuvm_integer_lessThan(context, arguments[0], arguments[1]);
}

static tuuvm_tuple_t tuuvm_integer_primitive_lessEquals(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return tuuvm_integer_lessEquals(context, arguments[0], arguments[1]);
}

static tuuvm_tuple_t tuuvm_integer_primitive_greaterThan(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return tuuvm_integer_greaterThan(context, arguments[0], arguments[1]);
}

static tuuvm_tuple_t tuuvm_integer_primitive_greaterEquals(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return tuuvm_integer_greaterEquals(context, arguments[0], arguments[1]);
}

void tuuvm_integer_setupPrimitives(tuuvm_context_t *context)
{
    tuuvm_type_setPrintStringFunction(context->roots.integerType, tuuvm_function_createPrimitive(context, 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_integer_primitive_printString));
    
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "Integer::+", 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_integer_primitive_add);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "Integer::-", 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_integer_primitive_subtract);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "Integer::negated", 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_integer_primitive_negate);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "Integer::*", 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_integer_primitive_multiply);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "Integer::/", 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_integer_primitive_divide);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "Integer::%", 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_integer_primitive_remainder);

    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "Integer::<=>", 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_integer_primitive_compare);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "Integer::=", 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_integer_primitive_equals);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "Integer::~=", 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_integer_primitive_notEquals);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "Integer::<", 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_integer_primitive_lessThan);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "Integer::<=", 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_integer_primitive_lessEquals);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "Integer::>", 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_integer_primitive_greaterThan);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(context, "Integer::>=", 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_integer_primitive_greaterEquals);

}
