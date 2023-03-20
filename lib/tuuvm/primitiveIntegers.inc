static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_printString)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

#if IS_SIGNED
    return tuuvm_primitiveInteger_signed_printString(context, primitiveInteger_decode(arguments[0]));
#else
    return tuuvm_primitiveInteger_unsigned_printString(context, primitiveInteger_decode(arguments[0]));
#endif
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_fromInteger)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

#if IS_SIGNED
    integer_t value = (integer_t)tuuvm_tuple_integer_decodeInt64(context, arguments[0]);
#else 
    integer_t value = (integer_t)tuuvm_tuple_integer_decodeUInt64(context, arguments[0]);
#endif
    return primitiveInteger_encode(value);
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_toInteger)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    integer_t value = (integer_t)primitiveInteger_decode(arguments[0]);
#if IS_SIGNED
    return tuuvm_tuple_integer_encodeInt64(context, value);
#else
    return tuuvm_tuple_integer_encodeUInt64(context, value);
#endif
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_add)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return primitiveInteger_encode(left + right);
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_subtract)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return primitiveInteger_encode(left - right);
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_negated)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    integer_t operand = primitiveInteger_decode(arguments[0]);
    return primitiveInteger_encode(-operand);
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_bitInvert)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    integer_t operand = primitiveInteger_decode(arguments[0]);
    return primitiveInteger_encode(~operand);
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_multiply)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return primitiveInteger_encode(left * right);
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_divide)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return primitiveInteger_encode(left / right);
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_remainder)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return primitiveInteger_encode(left % right);
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_bitAnd)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return primitiveInteger_encode(left & right);
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_bitOr)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return primitiveInteger_encode(left | right);
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_bitXor)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return primitiveInteger_encode(left ^ right);
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_shiftLeft)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return primitiveInteger_encode(left << right);
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_shiftRight)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return primitiveInteger_encode(left >> right);
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_equals)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return tuuvm_tuple_boolean_encode(left == right);
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_notEquals)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return tuuvm_tuple_boolean_encode(left != right);
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_lessThan)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return tuuvm_tuple_boolean_encode(left < right);
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_lessEquals)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return tuuvm_tuple_boolean_encode(left <= right);
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_greaterThan)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return tuuvm_tuple_boolean_encode(left > right);
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_greaterEquals)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return tuuvm_tuple_boolean_encode(left >= right);
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt8)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_uint8_encode((uint8_t)primitiveInteger_decode(arguments[0]));
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asInt8)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_int8_encode((int8_t)primitiveInteger_decode(arguments[0]));
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asChar8)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_char8_encode((tuuvm_char8_t)primitiveInteger_decode(arguments[0]));
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt16)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_uint16_encode((uint16_t)primitiveInteger_decode(arguments[0]));
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asInt16)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_int16_encode((int16_t)primitiveInteger_decode(arguments[0]));
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asChar16)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_char16_encode((tuuvm_char16_t)primitiveInteger_decode(arguments[0]));
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt32)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_uint32_encode(context, (uint32_t)primitiveInteger_decode(arguments[0]));
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asInt32)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_int32_encode(context, (int32_t)primitiveInteger_decode(arguments[0]));
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asChar32)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_char32_encode(context, (tuuvm_char32_t)primitiveInteger_decode(arguments[0]));
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt64)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_uint64_encode(context, (uint64_t)primitiveInteger_decode(arguments[0]));
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asInt64)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_int64_encode(context, (int64_t)primitiveInteger_decode(arguments[0]));
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asFloat32)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_float32_encode(context, (tuuvm_float32_t)primitiveInteger_decode(arguments[0]));
}

static tuuvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asFloat64)(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_tuple_float64_encode(context, (tuuvm_float64_t)primitiveInteger_decode(arguments[0]));
}

static void PRIMITIVE_INTEGER_FUNCTION(registerPrimitives)(void)
{
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_printString), INTEGER_TYPE_NAME "::printString");

    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_fromInteger), INTEGER_TYPE_NAME "::fromInteger");
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_toInteger), INTEGER_TYPE_NAME "::toInteger");

    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_negated), INTEGER_TYPE_NAME "::negated");
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_bitInvert), INTEGER_TYPE_NAME "::bitInvert");

    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_add), INTEGER_TYPE_NAME "::+");
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_subtract), INTEGER_TYPE_NAME "::-");
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_multiply), INTEGER_TYPE_NAME "::*");
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_divide), INTEGER_TYPE_NAME "::/");
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_remainder), INTEGER_TYPE_NAME "::%");

    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_bitAnd), INTEGER_TYPE_NAME "::&");
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_bitOr), INTEGER_TYPE_NAME "::|");
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_bitXor), INTEGER_TYPE_NAME "::^");
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_shiftLeft), INTEGER_TYPE_NAME "::<<");
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_shiftRight), INTEGER_TYPE_NAME "::>>");

    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_equals), INTEGER_TYPE_NAME "::=");
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_notEquals), INTEGER_TYPE_NAME "::~=");
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_lessThan), INTEGER_TYPE_NAME "::<");
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_lessEquals), INTEGER_TYPE_NAME "::<=");
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_greaterThan), INTEGER_TYPE_NAME "::>");
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_greaterEquals), INTEGER_TYPE_NAME "::>=");

    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt8), INTEGER_TYPE_NAME "::asUInt8");
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asInt8), INTEGER_TYPE_NAME "::asInt8");
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asChar8), INTEGER_TYPE_NAME "::asChar8");
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt16), INTEGER_TYPE_NAME "::asUInt16");
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asInt16), INTEGER_TYPE_NAME "::asInt16");
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asChar16), INTEGER_TYPE_NAME "::asChar16");
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt32), INTEGER_TYPE_NAME "::asUInt32");
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asInt32), INTEGER_TYPE_NAME "::asInt32");
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asChar32), INTEGER_TYPE_NAME "::asChar32");
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt64), INTEGER_TYPE_NAME "::asUInt64");
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asInt64), INTEGER_TYPE_NAME "::asInt64");
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asFloat32), INTEGER_TYPE_NAME "::asFloat32");
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asFloat64), INTEGER_TYPE_NAME "::asFloat64");
}

static void PRIMITIVE_INTEGER_FUNCTION(setupPrimitives)(tuuvm_context_t *context)
{
    tuuvm_type_setPrintStringFunction(context, context->roots.INTEGER_TYPE_ROOT_NAME, tuuvm_function_createPrimitive(context, 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_printString)));

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::fromInteger", context->roots.integerType, INTEGER_TYPE_SHORT_SUFFIX_NAME, 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_fromInteger));
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::toInteger", context->roots.INTEGER_TYPE_ROOT_NAME, "toInteger", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_toInteger));

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::negated", context->roots.INTEGER_TYPE_ROOT_NAME, "negated", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_negated));
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::bitInvert", context->roots.INTEGER_TYPE_ROOT_NAME, "bitInvert", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_bitInvert));

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::+", context->roots.INTEGER_TYPE_ROOT_NAME, "+", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_add));
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::-", context->roots.INTEGER_TYPE_ROOT_NAME, "-", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_subtract));
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::*", context->roots.INTEGER_TYPE_ROOT_NAME, "*", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_multiply));
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::/", context->roots.INTEGER_TYPE_ROOT_NAME, "/", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_divide));
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::%", context->roots.INTEGER_TYPE_ROOT_NAME, "%", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_remainder));

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::&", context->roots.INTEGER_TYPE_ROOT_NAME, "&", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_bitAnd));
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::|", context->roots.INTEGER_TYPE_ROOT_NAME, "|", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_bitOr));
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::^", context->roots.INTEGER_TYPE_ROOT_NAME, "^", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_bitXor));
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::<<", context->roots.INTEGER_TYPE_ROOT_NAME, "<<", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_shiftLeft));
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::>>", context->roots.INTEGER_TYPE_ROOT_NAME, ">>", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_shiftRight));

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::=", context->roots.INTEGER_TYPE_ROOT_NAME, "=", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_equals));
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::~=", context->roots.INTEGER_TYPE_ROOT_NAME, "~=", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_notEquals));
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::<", context->roots.INTEGER_TYPE_ROOT_NAME, "<", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_lessThan));
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::<=", context->roots.INTEGER_TYPE_ROOT_NAME, "<=", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_lessEquals));
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::>", context->roots.INTEGER_TYPE_ROOT_NAME, ">", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_greaterThan));
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::>=", context->roots.INTEGER_TYPE_ROOT_NAME, ">=", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_greaterEquals));

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::asUInt8", context->roots.INTEGER_TYPE_ROOT_NAME, "asUInt8", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt8));
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::asInt8", context->roots.INTEGER_TYPE_ROOT_NAME, "asInt8", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asInt8));
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::asChar8", context->roots.INTEGER_TYPE_ROOT_NAME, "asChar8", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asChar8));
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::asUInt16", context->roots.INTEGER_TYPE_ROOT_NAME, "asUInt16", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt16));
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::asInt16", context->roots.INTEGER_TYPE_ROOT_NAME, "asInt16", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asInt16));
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::asChar16", context->roots.INTEGER_TYPE_ROOT_NAME, "asChar16", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asChar16));
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::asUInt32", context->roots.INTEGER_TYPE_ROOT_NAME, "asUInt32", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt32));
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::asInt32", context->roots.INTEGER_TYPE_ROOT_NAME, "asInt32", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asInt32));
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::asChar32", context->roots.INTEGER_TYPE_ROOT_NAME, "asChar32", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asChar32));
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::asUInt64", context->roots.INTEGER_TYPE_ROOT_NAME, "asUInt64", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt64));
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::asInt64", context->roots.INTEGER_TYPE_ROOT_NAME, "asUInt64", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asInt64));
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::asFloat32", context->roots.INTEGER_TYPE_ROOT_NAME, "asFloat32", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asFloat32));
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::asFloat64", context->roots.INTEGER_TYPE_ROOT_NAME, "asFloat64", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asFloat64));

    if(context->roots.sizeType == context->roots.uint32Type)
    {
        tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::asUIntPointer", context->roots.INTEGER_TYPE_ROOT_NAME, "asUIntPointer", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt32));
        tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::asIntPointer", context->roots.INTEGER_TYPE_ROOT_NAME, "asIntPointer", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asInt32));
        tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::asSize", context->roots.INTEGER_TYPE_ROOT_NAME, "asSize", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt32));
    }
    else
    {
        tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::asUIntPointer", context->roots.INTEGER_TYPE_ROOT_NAME, "asUIntPointer", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt64));
        tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::asIntPointer", context->roots.INTEGER_TYPE_ROOT_NAME, "asIntPointer", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asInt64));
        tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, INTEGER_TYPE_NAME "::asSize", context->roots.INTEGER_TYPE_ROOT_NAME, "asSize", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt64));
    }
}

#undef integer_t
#undef primitiveInteger_decode
#undef primitiveInteger_encode
#undef IS_SIGNED
#undef FUNCTION_PREFIX
#undef INTEGER_TYPE_NAME
#undef INTEGER_TYPE_ROOT_NAME
#undef INTEGER_TYPE_SHORT_SUFFIX_NAME