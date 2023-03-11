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

static tuuvm_tuple_t tuuvm_float32_primitive_negate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
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

static tuuvm_tuple_t tuuvm_float64_primitive_negate(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
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

void tuuvm_float_registerPrimitives(void)
{
    // Float32
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_printString);
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_fromFloat64);

    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_add);
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_subtract);
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_negate);
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_sqrt);
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_multiply);
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_divide);

    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_compare);
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_equals);
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_notEquals);
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_lessThan);
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_lessEquals);
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_greaterThan);
    tuuvm_primitiveTable_registerFunction(tuuvm_float32_primitive_greaterEquals);

    // Float64
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_fromFloat32);

    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_printString);
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_add);
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_subtract);
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_negate);
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_sqrt);
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_multiply);
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_divide);

    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_compare);
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_equals);
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_notEquals);
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_lessThan);
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_lessEquals);
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_greaterThan);
    tuuvm_primitiveTable_registerFunction(tuuvm_float64_primitive_greaterEquals);
}

void tuuvm_float_setupPrimitives(tuuvm_context_t *context)
{
    // Float32
    tuuvm_type_setPrintStringFunction(context, context->roots.float32Type, tuuvm_function_createPrimitive(context, 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_printString));

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::fromFloat64", context->roots.float64Type, "f32", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_fromFloat64);

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::+", context->roots.float32Type, "+", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_add);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::-", context->roots.float32Type, "-", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_subtract);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::negated", context->roots.float32Type, "negated", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float32_primitive_negate);
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

    // Float64
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float32::fromFloat32", context->roots.float32Type, "f64", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_fromFloat32);

    tuuvm_type_setPrintStringFunction(context, context->roots.float64Type, tuuvm_function_createPrimitive(context, 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_printString));
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float64::+", context->roots.float64Type, "+", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_add);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float64::-", context->roots.float64Type, "-", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_subtract);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Float64::negated", context->roots.float64Type, "negated", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_float64_primitive_negate);
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
}
