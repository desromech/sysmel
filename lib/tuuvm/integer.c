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

static uint32_t tuuvm_integer_sumInto(size_t leftWordCount, uint32_t *leftWords, size_t rightWordCount, uint32_t *rightWords, size_t resultWordCount, uint32_t *resultWords)
{
    uint32_t carry = 0;
    
    size_t wordsToSum = leftWordCount > rightWordCount ? leftWordCount : rightWordCount;
    for(size_t i = 0; i < wordsToSum; ++i)
    {
        uint32_t leftWord = i < leftWordCount ? leftWords[i] : 0;
        uint32_t rightWord = i < rightWordCount ? rightWords[i] : 0;

        uint64_t sum = (uint64_t)leftWord + (uint64_t)rightWord + (uint64_t)carry;
        uint32_t sumWord = (uint32_t)sum;
        if(i < resultWordCount)
            resultWords[i] = sumWord;

        carry = (uint32_t)(sum >> 32);
    }

    for(size_t i = wordsToSum; i < resultWordCount; ++i)
    {
        if(i < resultWordCount)
            resultWords[i] = carry;
        carry = 0;
    }

    return carry;
}

static void tuuvm_integer_multiplyByWordAddingInto(size_t leftWordCount, uint32_t *leftWords, uint32_t word, size_t resultWordCount, uint32_t *resultWords)
{
    uint32_t carry = 0;
    size_t wordCount = leftWordCount > resultWordCount ? leftWordCount : resultWordCount;
    for(size_t i = 0; i < wordCount; ++i)
    {
        uint32_t leftWord = i < leftWordCount ? leftWords[i] : 0;
        uint32_t resultWord = i < resultWordCount ? resultWords[i] : 0;

        uint64_t multiplication = (uint64_t)leftWord*(uint64_t)word + (uint64_t)resultWord + (uint64_t)carry;
        resultWords[i] = (uint32_t)multiplication;
        carry = (uint32_t)(multiplication >> 32);
    }
}

static void tuuvm_integer_multiplyInto(size_t leftWordCount, uint32_t *leftWords, size_t rightWordCount, uint32_t *rightWords, size_t resultWordCount, uint32_t *resultWords)
{
    for(size_t i = 0; i < resultWordCount; ++i)
        resultWords[i] = 0;

    for(size_t i = 0; i < rightWordCount && i < resultWordCount; ++i)
        tuuvm_integer_multiplyByWordAddingInto(leftWordCount, leftWords, rightWords[i], resultWordCount - i, resultWords + i);
}

static int32_t tuuvm_integer_subtractFromInto(size_t leftWordCount, uint32_t *leftWords, size_t rightWordCount, uint32_t *rightWords, size_t resultWordCount, uint32_t *resultWords)
{
    int32_t borrow = 0;
    
    size_t wordsToSum = leftWordCount > rightWordCount ? leftWordCount : rightWordCount;
    for(size_t i = 0; i < wordsToSum; ++i)
    {
        uint32_t leftWord = i < leftWordCount ? leftWords[i] : 0;
        uint32_t rightWord = i < rightWordCount ? rightWords[i] : 0;

        int64_t subtraction = (int64_t)leftWord - (int64_t)rightWord + (int64_t)borrow;
        uint32_t subtractionWord = (uint32_t)subtraction;
        if(i < resultWordCount)
            resultWords[i] = subtractionWord;

        borrow = (int32_t)(subtraction >> 32);
    }

    for(size_t i = wordsToSum; i < resultWordCount; ++i)
    {
        if(i < resultWordCount)
            resultWords[i] = 0;
    }

    return borrow;
}

static tuuvm_tuple_t tuuvm_integer_normalize(tuuvm_context_t *context, tuuvm_integer_t *integer)
{
    // Counte the required number of normalized words.
    size_t wordCount = tuuvm_tuple_getSizeInBytes((tuuvm_tuple_t)integer) / 4;
    size_t normalizedWordCount = wordCount;
    while(normalizedWordCount > 0 && integer->words[normalizedWordCount - 1])
        --normalizedWordCount;

    // Trivial cases.
    if(normalizedWordCount == wordCount)
    {
        return (tuuvm_tuple_t)integer;
    }
    else if(normalizedWordCount == 0)
    {
        return tuuvm_tuple_integer_encodeSmall(0);
    }
    
    // Attempt to fit in an immediate, if possible.
    tuuvm_tuple_t largeIntegerType = tuuvm_tuple_getType(context, (tuuvm_tuple_t)integer);
    bool isNegative = largeIntegerType == context->roots.negativeIntegerType;
    if(normalizedWordCount <= 2)
    {
        uint64_t valueUInt64 = (uint64_t)integer->words[0] | ((uint64_t)integer->words[1] << 32);
        if(isNegative)
        {
            if(valueUInt64 <= (uint64_t)-TUUVM_IMMEDIATE_INT_MIN)
                return tuuvm_tuple_integer_encodeSmall(-(int64_t)valueUInt64);
        }
        else
        {
            if(valueUInt64 <= TUUVM_IMMEDIATE_INT_MAX)
                return tuuvm_tuple_integer_encodeSmall((int64_t)valueUInt64);
        }
    }

    // Make a newer smaller large integer.
    tuuvm_integer_t *normalizedResult = (tuuvm_integer_t*)tuuvm_context_allocateByteTuple(context, largeIntegerType, normalizedWordCount*4);
    memcpy(normalizedResult->words, integer->words, 4*normalizedWordCount);
    return (tuuvm_tuple_t)normalizedWordCount;
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

TUUVM_API tuuvm_tuple_t tuuvm_tuple_integer_encodeBigUInt32(tuuvm_context_t *context, uint32_t value)
{
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
    if(value <= TUUVM_IMMEDIATE_INT_MAX)
        return tuuvm_tuple_integer_encodeSmall(value);

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

    tuuvm_integer_t *result = (tuuvm_integer_t*)tuuvm_context_allocateByteTuple(context, context->roots.positiveIntegerType, 4);
    result->words[0] = (uint32_t)value;
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

int32_t tuuvm_tuple_integer_decodeInt32(tuuvm_context_t *context, tuuvm_tuple_t value)
{
    if(tuuvm_tuple_isImmediate(value))
        return (int32_t)tuuvm_tuple_integer_decodeSmall(value);

    if(tuuvm_tuple_getSizeInBytes(value) < 4)
        return 0;

    tuuvm_integer_t *integer = (tuuvm_integer_t *)value;
    int32_t decodedValue = integer->words[0];
    if(tuuvm_tuple_getType(context, value) == context->roots.negativeIntegerType)
        decodedValue = -decodedValue;
    return decodedValue;
}

uint32_t tuuvm_tuple_integer_decodeUInt32(tuuvm_context_t *context, tuuvm_tuple_t value)
{
    return (uint32_t)tuuvm_tuple_integer_decodeInt32(context, value);
}

int64_t tuuvm_tuple_integer_decodeInt64(tuuvm_context_t *context, tuuvm_tuple_t value)
{
    if(tuuvm_tuple_isImmediate(value))
        return tuuvm_tuple_integer_decodeSmall(value);

    size_t byteSize = tuuvm_tuple_getSizeInBytes(value);
    if(byteSize < 4)
        return 0;

    tuuvm_integer_t *integer = (tuuvm_integer_t *)value;
    int64_t decodedValue = 0;
    if(byteSize < 8)
        decodedValue = (uint64_t)integer->words[0] | ((uint64_t)integer->words[1] << 32);
    else
        decodedValue = (uint64_t)integer->words[0];
    
    if(tuuvm_tuple_getType(context, value) == context->roots.negativeIntegerType)
        decodedValue = -decodedValue;
    return decodedValue;
}

uint64_t tuuvm_tuple_integer_decodeUInt64(tuuvm_context_t *context, tuuvm_tuple_t value)
{
    return (uint64_t)tuuvm_tuple_integer_decodeInt64(context, value);
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
    tuuvm_tuple_t radix = tuuvm_tuple_integer_encodeSmall(10);
    bool canParseRadix = true;
    for(; index < stringSize; ++index)
    {
        char digit = string[index];
        if('0' <= digit && digit <= '9')
        {
            tuuvm_tuple_t digitValue = tuuvm_tuple_integer_encodeSmall(digit - '0');
            if(tuuvm_integer_greaterEquals(context, digitValue, radix))
                tuuvm_error("Digit value is beyond the radix.");

            result = tuuvm_integer_multiply(context, result, radix);
            result = tuuvm_integer_add(context, result, digitValue);
        }
        else if(!canParseRadix && 'a' <= digit && digit <= 'z')
        {
            tuuvm_tuple_t digitValue = tuuvm_tuple_integer_encodeSmall(digit - 'a' + 10);
            if(tuuvm_integer_greaterEquals(context, digitValue, radix))
                tuuvm_error("Digit value is beyond the radix.");
            result = tuuvm_integer_multiply(context, result, radix);
            result = tuuvm_integer_add(context, result, digitValue);
        }
        else if(!canParseRadix && 'A' <= digit && digit <= 'Z')
        {
            tuuvm_tuple_t digitValue = tuuvm_tuple_integer_encodeSmall(digit - 'A' + 10);
            if(tuuvm_integer_greaterEquals(context, digitValue, radix))
                tuuvm_error("Digit value is beyond the radix.");
            result = tuuvm_integer_multiply(context, result, radix);
            result = tuuvm_integer_add(context, result, digitValue);
        }
        else if('_' == digit)
        {
            // Ignore it
        }
        else if(canParseRadix && ('r' == digit || 'r' == digit))
        {
            // We just saw the radix.
            radix = result;
            result = tuuvm_tuple_integer_encodeSmall(0);
            canParseRadix = false;
        }
        else
        {
            tuuvm_error("Invalid integer literal.");
        }
    }

    if(isNegative)
        return tuuvm_integer_negated(context, result);

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

    tuuvm_decoded_integer_t decodedLeftInteger = {0};
    tuuvm_decoded_integer_t decodedRightInteger = {0};
    tuuvm_integer_decodeLargeOrImmediate(context, &decodedLeftInteger, left);
    tuuvm_integer_decodeLargeOrImmediate(context, &decodedRightInteger, right);

    if(decodedLeftInteger.isNegative == decodedRightInteger.isNegative)
    {
        // Same sign, sum
        uint32_t carry = tuuvm_integer_sumInto(decodedLeftInteger.wordCount, decodedLeftInteger.words, decodedRightInteger.wordCount, decodedRightInteger.words, 0, NULL);
        size_t resultWordCount = (decodedLeftInteger.wordCount > decodedRightInteger.wordCount) ? decodedLeftInteger.wordCount : decodedRightInteger.wordCount;
        if(carry != 0)
            ++resultWordCount;

        tuuvm_integer_t *result = (tuuvm_integer_t*)tuuvm_context_allocateByteTuple(context, decodedLeftInteger.isNegative ? context->roots.negativeIntegerType : context->roots.positiveIntegerType, resultWordCount*4);
        tuuvm_integer_sumInto(decodedLeftInteger.wordCount, decodedLeftInteger.words, decodedRightInteger.wordCount, decodedRightInteger.words, resultWordCount, result->words);
        return (tuuvm_tuple_t)result;
    }
    else
    {
        // Different sign, subtract.
        int magnitudeComparison = tuuvm_integer_compareMagnitudes(decodedLeftInteger.wordCount, decodedLeftInteger.words, decodedRightInteger.wordCount, decodedRightInteger.words);
        if(magnitudeComparison == 0)
            return tuuvm_tuple_integer_encodeSmall(0);

        if(magnitudeComparison > 0)
        {
            size_t resultWordCount = decodedLeftInteger.wordCount;
            bool resultIsNegative = decodedLeftInteger.isNegative;
            tuuvm_integer_t *result = (tuuvm_integer_t*)tuuvm_context_allocateByteTuple(context, resultIsNegative ? context->roots.negativeIntegerType : context->roots.positiveIntegerType, resultWordCount*4);
            tuuvm_integer_subtractFromInto(decodedLeftInteger.wordCount, decodedLeftInteger.words, decodedRightInteger.wordCount, decodedRightInteger.words, resultWordCount, result->words);
            return tuuvm_integer_normalize(context, result);
        }
        else
        {
            size_t resultWordCount = decodedRightInteger.wordCount;
            bool resultIsNegative = decodedRightInteger.isNegative;
            tuuvm_integer_t *result = (tuuvm_integer_t*)tuuvm_context_allocateByteTuple(context, resultIsNegative ? context->roots.negativeIntegerType : context->roots.positiveIntegerType, resultWordCount*4);
            tuuvm_integer_subtractFromInto(decodedRightInteger.wordCount, decodedRightInteger.words, decodedLeftInteger.wordCount, decodedLeftInteger.words, resultWordCount, result->words);
            return tuuvm_integer_normalize(context, result);
        }
    }
}

TUUVM_API tuuvm_tuple_t tuuvm_integer_subtract(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right)
{
    if(tuuvm_tuple_isImmediate(left) && tuuvm_tuple_isImmediate(right))
    {
        tuuvm_stuple_t leftValue = tuuvm_tuple_integer_decodeSmall(left);
        tuuvm_stuple_t rightValue = tuuvm_tuple_integer_decodeSmall(right);
        return tuuvm_tuple_integer_encodeInt64(context, leftValue - rightValue);
    }

    tuuvm_decoded_integer_t decodedLeftInteger = {0};
    tuuvm_decoded_integer_t decodedRightInteger = {0};
    tuuvm_integer_decodeLargeOrImmediate(context, &decodedLeftInteger, left);
    tuuvm_integer_decodeLargeOrImmediate(context, &decodedRightInteger, right);

    if(decodedLeftInteger.isNegative == decodedRightInteger.isNegative)
    {
        // Same sign, subtract.
        int magnitudeComparison = tuuvm_integer_compareMagnitudes(decodedLeftInteger.wordCount, decodedLeftInteger.words, decodedRightInteger.wordCount, decodedRightInteger.words);
        if(magnitudeComparison == 0)
            return tuuvm_tuple_integer_encodeSmall(0);

        if(magnitudeComparison > 0)
        {
            size_t resultWordCount = decodedLeftInteger.wordCount;
            bool resultIsNegative = decodedLeftInteger.isNegative;
            tuuvm_integer_t *result = (tuuvm_integer_t*)tuuvm_context_allocateByteTuple(context, resultIsNegative ? context->roots.negativeIntegerType : context->roots.positiveIntegerType, resultWordCount*4);
            tuuvm_integer_subtractFromInto(decodedLeftInteger.wordCount, decodedLeftInteger.words, decodedRightInteger.wordCount, decodedRightInteger.words, resultWordCount, result->words);
            return tuuvm_integer_normalize(context, result);
        }
        else
        {
            size_t resultWordCount = decodedRightInteger.wordCount;
            bool resultIsNegative = decodedRightInteger.isNegative;
            tuuvm_integer_t *result = (tuuvm_integer_t*)tuuvm_context_allocateByteTuple(context, resultIsNegative ? context->roots.negativeIntegerType : context->roots.positiveIntegerType, resultWordCount*4);
            tuuvm_integer_subtractFromInto(decodedRightInteger.wordCount, decodedRightInteger.words, decodedLeftInteger.wordCount, decodedLeftInteger.words, resultWordCount, result->words);
            return tuuvm_integer_normalize(context, result);
        }
    }
    else
    {
        // Differing sign, sum
        uint32_t carry = tuuvm_integer_sumInto(decodedLeftInteger.wordCount, decodedLeftInteger.words, decodedRightInteger.wordCount, decodedRightInteger.words, 0, NULL);
        size_t resultWordCount = (decodedLeftInteger.wordCount > decodedRightInteger.wordCount) ? decodedLeftInteger.wordCount : decodedRightInteger.wordCount;
        if(carry != 0)
            ++resultWordCount;

        tuuvm_integer_t *result = (tuuvm_integer_t*)tuuvm_context_allocateByteTuple(context, decodedLeftInteger.isNegative ? context->roots.negativeIntegerType : context->roots.positiveIntegerType, resultWordCount*4);
        tuuvm_integer_sumInto(decodedLeftInteger.wordCount, decodedLeftInteger.words, decodedRightInteger.wordCount, decodedRightInteger.words, resultWordCount, result->words);
        return (tuuvm_tuple_t)result;
    }
}

TUUVM_API tuuvm_tuple_t tuuvm_integer_negated(tuuvm_context_t *context, tuuvm_tuple_t integer)
{
    if(tuuvm_tuple_isImmediate(integer))
    {
        tuuvm_stuple_t integerValue = tuuvm_tuple_integer_decodeSmall(integer);
        return tuuvm_tuple_integer_encodeInt64(context, -integerValue);
    }

    // Shallow copy and swap the integer type.
    tuuvm_tuple_t negatedInteger = tuuvm_context_shallowCopy(context, integer);
    tuuvm_tuple_t integerType = tuuvm_tuple_getType(context, negatedInteger);
    tuuvm_tuple_setType(TUUVM_CAST_OOP_TO_OBJECT_TUPLE(negatedInteger), integerType == context->roots.positiveIntegerType ? context->roots.negativeIntegerType : context->roots.positiveIntegerType);
    return tuuvm_integer_normalize(context, (tuuvm_integer_t*)negatedInteger);
}

TUUVM_API tuuvm_tuple_t tuuvm_integer_multiply(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right)
{
    if(tuuvm_tuple_isImmediate(left) && tuuvm_tuple_isImmediate(right))
    {
        tuuvm_stuple_t leftValue = tuuvm_tuple_integer_decodeSmall(left);
        tuuvm_stuple_t rightValue = tuuvm_tuple_integer_decodeSmall(right);

        tuuvm_stuple_t result = leftValue * rightValue;
        if(leftValue == 0 || rightValue == 0 || leftValue == result / rightValue)
            return tuuvm_tuple_integer_encodeInt64(context, leftValue * rightValue);
    }

    tuuvm_decoded_integer_t decodedLeftInteger = {0};
    tuuvm_decoded_integer_t decodedRightInteger = {0};
    tuuvm_integer_decodeLargeOrImmediate(context, &decodedLeftInteger, left);
    tuuvm_integer_decodeLargeOrImmediate(context, &decodedRightInteger, right);

    bool resultIsNegative = decodedLeftInteger.isNegative != decodedRightInteger.isNegative;
    size_t resultWordCount = decodedLeftInteger.wordCount + decodedRightInteger.wordCount + 1;
    tuuvm_integer_t *result = (tuuvm_integer_t*)tuuvm_context_allocateByteTuple(context, resultIsNegative ? context->roots.negativeIntegerType : context->roots.positiveIntegerType, resultWordCount*4);
    tuuvm_integer_multiplyInto(decodedLeftInteger.wordCount, decodedLeftInteger.words, decodedRightInteger.wordCount, decodedRightInteger.words, resultWordCount, result->words);
    return tuuvm_integer_normalize(context, result);
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

TUUVM_API int tuuvm_integer_compare(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right)
{
    if(tuuvm_tuple_isImmediate(left) && tuuvm_tuple_isImmediate(right))
    {
        tuuvm_stuple_t leftValue = tuuvm_tuple_integer_decodeSmall(left);
        tuuvm_stuple_t rightValue = tuuvm_tuple_integer_decodeSmall(right);
        if(leftValue < rightValue)
            return -1;
        else if(leftValue > rightValue)
            return 1;
        else
            return 0;
    }

    tuuvm_decoded_integer_t decodedLeftInteger = {0};
    tuuvm_decoded_integer_t decodedRightInteger = {0};
    tuuvm_integer_decodeLargeOrImmediate(context, &decodedLeftInteger, left);
    tuuvm_integer_decodeLargeOrImmediate(context, &decodedRightInteger, right);

    // Check the differing signs.
    if(decodedLeftInteger.isNegative != decodedRightInteger.isNegative)
    {
        if(decodedLeftInteger.isNegative)
            return -1;
        else
            return 1;
    }

    // Check the word count.
    int magnitudeComparison = tuuvm_integer_compareMagnitudes(decodedLeftInteger.wordCount, decodedLeftInteger.words, decodedRightInteger.wordCount, decodedRightInteger.words);
    if(decodedLeftInteger.isNegative)
        magnitudeComparison = -magnitudeComparison;

    return magnitudeComparison;
}

TUUVM_API tuuvm_tuple_t tuuvm_integer_factorial(tuuvm_context_t *context, tuuvm_tuple_t integer)
{
    tuuvm_tuple_t one = tuuvm_tuple_integer_encodeSmall(1);
    if(tuuvm_integer_lessEquals(context, integer, one))
        return one;
    
    tuuvm_tuple_t n1 = tuuvm_integer_factorial(context, tuuvm_integer_subtract(context, integer, tuuvm_tuple_integer_encodeSmall(1)));
    return tuuvm_integer_multiply(context, n1, integer);
}

TUUVM_API bool tuuvm_integer_equals(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right)
{
    return tuuvm_integer_compare(context, left, right) == 0;
}

TUUVM_API bool tuuvm_integer_notEquals(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right)
{
    return tuuvm_integer_compare(context, left, right) != 0;
}

TUUVM_API bool tuuvm_integer_lessThan(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right)
{
    return tuuvm_integer_compare(context, left, right) < 0;
}

TUUVM_API bool tuuvm_integer_lessEquals(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right)
{
    return tuuvm_integer_compare(context, left, right) <= 0;
}

TUUVM_API bool tuuvm_integer_greaterThan(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right)
{
    return tuuvm_integer_compare(context, left, right) > 0;
}

TUUVM_API bool tuuvm_integer_greaterEquals(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right)
{
    return tuuvm_integer_compare(context, left, right) >= 0;
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

TUUVM_API tuuvm_tuple_t tuuvm_integer_toHexString(tuuvm_context_t *context, tuuvm_tuple_t integer)
{
    tuuvm_decoded_integer_t decodedInteger = {0};
    tuuvm_integer_decodeLargeOrImmediate(context, &decodedInteger, integer);
    
    // Count the required nibbles.
    size_t nibbleCount = 1; // We require at least one nibble.
    for(size_t i = 0; i < decodedInteger.wordCount; ++i)
    {
        uint32_t word = decodedInteger.words[i];
        for(uint32_t j = 0; j < 8; ++j)
        {
            size_t nibbleIndex = i*8 + j;
            if(((word >> (j*4)) & 0xF) != 0)
                nibbleCount = nibbleIndex + 1;
        }
    }

    // Compute the character count.
    size_t characterCount = nibbleCount + (decodedInteger.isNegative ? 1 : 0);
    
    tuuvm_tuple_t resultString = tuuvm_string_createEmptyWithSize(context, characterCount);
    uint8_t *resultBytes = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(resultString)->bytes;
    if(decodedInteger.isNegative)
    {
        resultBytes[0] = '-';
        ++resultBytes;
        --characterCount;
    }

    // Emit the hex characters
    for(size_t i = 0; i < decodedInteger.wordCount; ++i)
    {
        uint32_t word = decodedInteger.words[i];
        for(uint32_t j = 0; j < 8; ++j)
        {
            size_t nibbleIndex = i*8 + j;
            if(nibbleIndex >= nibbleCount)
                break;

            uint32_t nibble = (word >> (j*4)) & 0xF;
            uint8_t nibbleCharacter = 0;
            if(nibble < 10)
                nibbleCharacter = '0' + nibble;
            else
                nibbleCharacter = 'A' + nibble - 10;

            resultBytes[nibbleCount - nibbleIndex - 1] = nibbleCharacter;
        }
    }

    return resultString;
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

static tuuvm_tuple_t tuuvm_integer_primitive_negated(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_integer_negated(context, arguments[0]);
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

    return tuuvm_tuple_integer_encodeSmall(tuuvm_integer_compare(context, arguments[0], arguments[1]));
}

static tuuvm_tuple_t tuuvm_integer_primitive_equals(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return tuuvm_tuple_boolean_encode(tuuvm_integer_equals(context, arguments[0], arguments[1]));
}

static tuuvm_tuple_t tuuvm_integer_primitive_notEquals(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return tuuvm_tuple_boolean_encode(tuuvm_integer_notEquals(context, arguments[0], arguments[1]));
}

static tuuvm_tuple_t tuuvm_integer_primitive_lessThan(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return tuuvm_tuple_boolean_encode(tuuvm_integer_lessThan(context, arguments[0], arguments[1]));
}

static tuuvm_tuple_t tuuvm_integer_primitive_lessEquals(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return tuuvm_tuple_boolean_encode(tuuvm_integer_lessEquals(context, arguments[0], arguments[1]));
}

static tuuvm_tuple_t tuuvm_integer_primitive_greaterThan(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return tuuvm_tuple_boolean_encode(tuuvm_integer_greaterThan(context, arguments[0], arguments[1]));
}

static tuuvm_tuple_t tuuvm_integer_primitive_greaterEquals(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return tuuvm_tuple_boolean_encode(tuuvm_integer_greaterEquals(context, arguments[0], arguments[1]));
}

static tuuvm_tuple_t tuuvm_integer_primitive_factorial(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_integer_factorial(context, arguments[0]);
}

void tuuvm_integer_registerPrimitives(void)
{
    tuuvm_primitiveTable_registerFunction(tuuvm_integer_primitive_printString, "Integer::printString");

    tuuvm_primitiveTable_registerFunction(tuuvm_integer_primitive_add, "Integer::+");
    tuuvm_primitiveTable_registerFunction(tuuvm_integer_primitive_subtract, "Integer::-");
    tuuvm_primitiveTable_registerFunction(tuuvm_integer_primitive_negated, "Integer::negated");
    tuuvm_primitiveTable_registerFunction(tuuvm_integer_primitive_multiply, "Integer::*");
    tuuvm_primitiveTable_registerFunction(tuuvm_integer_primitive_divide, "Integer::/");
    tuuvm_primitiveTable_registerFunction(tuuvm_integer_primitive_remainder, "Integer::%");

    tuuvm_primitiveTable_registerFunction(tuuvm_integer_primitive_compare, "Integer::<=>");
    tuuvm_primitiveTable_registerFunction(tuuvm_integer_primitive_equals, "Integer::=");
    tuuvm_primitiveTable_registerFunction(tuuvm_integer_primitive_notEquals, "Integer::~=");
    tuuvm_primitiveTable_registerFunction(tuuvm_integer_primitive_lessThan, "Integer::<");
    tuuvm_primitiveTable_registerFunction(tuuvm_integer_primitive_lessEquals, "Integer::<=");
    tuuvm_primitiveTable_registerFunction(tuuvm_integer_primitive_greaterThan, "Integer::>");
    tuuvm_primitiveTable_registerFunction(tuuvm_integer_primitive_greaterEquals, "Integer::>=");

    tuuvm_primitiveTable_registerFunction(tuuvm_integer_primitive_factorial, "Integer::factorial");
}

void tuuvm_integer_setupPrimitives(tuuvm_context_t *context)
{
    tuuvm_tuple_t printString = tuuvm_function_createPrimitive(context, 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_integer_primitive_printString);
    tuuvm_type_setPrintStringFunction(context, context->roots.integerType, printString);
    tuuvm_type_setPrintStringFunction(context, context->roots.positiveIntegerType, printString);
    tuuvm_type_setPrintStringFunction(context, context->roots.negativeIntegerType, printString);
    
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Integer::+", context->roots.integerType, "+", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_integer_primitive_add);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Integer::-", context->roots.integerType, "-", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_integer_primitive_subtract);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Integer::negated", context->roots.integerType, "negated", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_integer_primitive_negated);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Integer::*", context->roots.integerType, "*", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_integer_primitive_multiply);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Integer::/", context->roots.integerType, "/", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_integer_primitive_divide);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Integer::%", context->roots.integerType, "%", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_integer_primitive_remainder);

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Integer::<=>", context->roots.integerType, "<=>", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_integer_primitive_compare);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Integer::=", context->roots.integerType, "=", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_integer_primitive_equals);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Integer::~=", context->roots.integerType, "~=", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_integer_primitive_notEquals);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Integer::<", context->roots.integerType, "<", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_integer_primitive_lessThan);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Integer::<=", context->roots.integerType, "<=", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_integer_primitive_lessEquals);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Integer::>", context->roots.integerType, ">", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_integer_primitive_greaterThan);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Integer::>=", context->roots.integerType, ">=", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_integer_primitive_greaterEquals);

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Integer::factorial", context->roots.integerType, "factorial", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_integer_primitive_factorial);
}
