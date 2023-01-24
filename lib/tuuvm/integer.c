#include "tuuvm/integer.h"
#include "tuuvm/errors.h"
#include "tuuvm/function.h"
#include "tuuvm/string.h"
#include "tuuvm/type.h"
#include "internal/context.h"
#include <string.h>
#include <stdio.h>

typedef struct tuuvm_decoded_integer_s
{
    bool isNegative;
    size_t wordCount;
    uint32_t *words;
    uint32_t inlineWordBuffer[2];
} tuuvm_decoded_integer_t;

static void tuuvm_integer_decodeLargeOrImmediate(tuuvm_context_t *context, tuuvm_decoded_integer_t *decodedInteger, tuuvm_tuple_t integer)
{
    if(tuuvm_tuple_isImmediate(integer))
    {
        // Decode the small integer.
        tuuvm_stuple_t decodedSmall = tuuvm_tuple_integer_decodeSmall(integer);
        uint64_t decodedSmallPositive = decodedSmall >= 0 ? (uint64_t)decodedSmall : (uint64_t)(-decodedSmall);

        decodedInteger->isNegative = decodedSmall < 0;
        decodedInteger->inlineWordBuffer[0] = (uint32_t)decodedSmallPositive;
        decodedInteger->inlineWordBuffer[1] = (uint32_t)(decodedSmallPositive >> 32);
        decodedInteger->words = decodedInteger->inlineWordBuffer;
        decodedInteger->wordCount = decodedInteger->inlineWordBuffer[1] == 0 ? 1 : 2;
    }
    else
    {
        // Decode the large integer.
        tuuvm_integer_t *integerObject = (tuuvm_integer_t*)integer;
        decodedInteger->isNegative = tuuvm_tuple_getType(context, integer) == context->roots.negativeIntegerType;
        decodedInteger->words = integerObject->words;
        decodedInteger->wordCount = tuuvm_tuple_getSizeInBytes(integer) / 4;

        // Normalize the word count.
        while(decodedInteger->wordCount > 1 && decodedInteger->words[decodedInteger->wordCount - 1] == 0)
            --decodedInteger->wordCount;
    }
}

static int tuuvm_integer_compareMagnitudes(size_t leftWordCount, uint32_t *leftWords, size_t rightWordCount, uint32_t *rightWords)
{
    size_t wordsToCompare = leftWordCount > rightWordCount ? leftWordCount : rightWordCount;
    for(size_t i = 0; i < wordsToCompare; ++i)
    {
        size_t wordIndex = wordsToCompare - i - 1;
        uint32_t leftWord = wordIndex < leftWordCount ? leftWords[wordIndex] : 0;
        uint32_t rightWord = wordIndex < rightWordCount ? rightWords[wordIndex] : 0;
        if(leftWord < rightWord)
            return -1;
        else if(leftWord > rightWord)
            return 1;
    }

    return 0;
}

TUUVM_API tuuvm_tuple_t tuuvm_tuple_integer_encodeBigInt32(tuuvm_context_t *context, int32_t value)
{
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
    if(TUUVM_IMMEDIATE_INT_MIN <= value && value <= TUUVM_IMMEDIATE_INT_MAX)
        return tuuvm_tuple_integer_encodeSmall(value);

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

    tuuvm_integer_t *result = (tuuvm_integer_t*)tuuvm_context_allocateByteTuple(context, value >= 0 ? context->roots.positiveIntegerType : context->roots.negativeIntegerType, 4);
    if(value >= 0)
        result->words[0] = (uint32_t)value;
    else
        result->words[0] = (uint32_t)(-value);
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_tuple_integer_encodeBigInt64(tuuvm_context_t *context, int64_t value)
{
    if(TUUVM_IMMEDIATE_INT_MIN <= value && value <= TUUVM_IMMEDIATE_INT_MAX)
        return tuuvm_tuple_integer_encodeSmall(value);

    uint64_t positiveValue = value >= 0 ? (uint64_t)value : (uint64_t)(-value);

    if(positiveValue <= UINT32_MAX)
    {
        tuuvm_integer_t *result = (tuuvm_integer_t*)tuuvm_context_allocateByteTuple(context, value >= 0 ? context->roots.positiveIntegerType : context->roots.negativeIntegerType, 4);
        result->words[0] = (uint32_t)value;
        return (tuuvm_tuple_t)result;
    }
    else
    {
        tuuvm_integer_t *result = (tuuvm_integer_t*)tuuvm_context_allocateByteTuple(context, value >= 0 ? context->roots.positiveIntegerType : context->roots.negativeIntegerType, 8);
        result->words[0] = (uint32_t)value;
        result->words[1] = (uint32_t)(value >> 32);
        return (tuuvm_tuple_t)result;
    }
}

TUUVM_API tuuvm_tuple_t tuuvm_tuple_integer_encodeBigUInt64(tuuvm_context_t *context, uint64_t value)
{
    if(value <= TUUVM_IMMEDIATE_INT_MAX)
        return tuuvm_tuple_integer_encodeSmall(value);

    if(value <= UINT32_MAX)
    {
        tuuvm_integer_t *result = (tuuvm_integer_t*)tuuvm_context_allocateByteTuple(context, context->roots.positiveIntegerType, 4);
        result->words[0] = (uint32_t)value;
        return (tuuvm_tuple_t)result;
    }
    else
    {
        tuuvm_integer_t *result = (tuuvm_integer_t*)tuuvm_context_allocateByteTuple(context, context->roots.positiveIntegerType, 8);
        result->words[0] = (uint32_t)value;
        result->words[1] = (uint32_t)(value >> 32);
        return (tuuvm_tuple_t)result;
    }
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

    tuuvm_decoded_integer_t decodedLeftInteger = {};
    tuuvm_decoded_integer_t decodedRightInteger = {};
    tuuvm_integer_decodeLargeOrImmediate(context, &decodedLeftInteger, left);
    tuuvm_integer_decodeLargeOrImmediate(context, &decodedRightInteger, right);

    // Check the differing signs.
    if(decodedLeftInteger.isNegative != decodedRightInteger.isNegative)
    {
        if(decodedLeftInteger.isNegative)
            return tuuvm_tuple_integer_encodeSmall(-1);
        else
            return tuuvm_tuple_integer_encodeSmall(1);
    }

    // Check the word count.
    int magnitudeComparison = tuuvm_integer_compareMagnitudes(decodedLeftInteger.wordCount, decodedLeftInteger.words, decodedRightInteger.wordCount, decodedRightInteger.words);
    if(decodedLeftInteger.isNegative)
        magnitudeComparison = -magnitudeComparison;

    return tuuvm_tuple_integer_encodeSmall(magnitudeComparison);
}

TUUVM_API tuuvm_tuple_t tuuvm_integer_factorial(tuuvm_context_t *context, tuuvm_tuple_t integer)
{
    tuuvm_tuple_t one = tuuvm_tuple_integer_encodeSmall(1);
    if(tuuvm_integer_lessEquals(context, integer, one))
        return one;
    
    tuuvm_tuple_t n1 = tuuvm_integer_factorial(context, tuuvm_integer_subtract(context, integer, tuuvm_tuple_integer_encodeSmall(1)));
    return tuuvm_integer_multiply(context, n1, integer);
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
