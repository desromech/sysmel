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

static void PRIMITIVE_INTEGER_FUNCTION(registerPrimitives)(void)
{
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_printString));

    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_fromInteger));
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_toInteger));

    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_negated));
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_bitInvert));

    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_add));
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_subtract));
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_multiply));
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_divide));
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_remainder));

    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_bitAnd));
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_bitOr));
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_bitXor));
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_shiftLeft));
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_shiftRight));

    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_equals));
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_notEquals));
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_lessThan));
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_lessEquals));
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_greaterThan));
    tuuvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_greaterEquals));
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
}

#undef integer_t
#undef primitiveInteger_decode
#undef primitiveInteger_encode
#undef IS_SIGNED
#undef FUNCTION_PREFIX
#undef INTEGER_TYPE_NAME
#undef INTEGER_TYPE_ROOT_NAME
#undef INTEGER_TYPE_SHORT_SUFFIX_NAME