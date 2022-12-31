#include "tuuvm/type.h"
#include "tuuvm/string.h"
#include "tuuvm/type.h"
#include "internal/context.h"

TUUVM_API tuuvm_tuple_t tuuvm_type_createAnonymous(tuuvm_context_t *context)
{
    tuuvm_type_tuple_t* result = (tuuvm_type_tuple_t*)tuuvm_context_allocatePointerTuple(context, context->roots.typeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_type_tuple_t));
    result->equalsFunction = context->roots.identityEqualsFunction;
    result->hashFunction = context->roots.identityHashFunction;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_createWithName(tuuvm_context_t *context, tuuvm_tuple_t name)
{
    tuuvm_tuple_t result = tuuvm_type_createAnonymous(context);
    tuuvm_type_setName(result, name);
    return result;
}
/*
static tuuvm_tuple_t tuuvm_type_primitive_create(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 0) tuuvm_error_argumentCountMismatch(0, argumentCount);

    return tuuvm_type_createAnonymous(context);
}

static tuuvm_tuple_t tuuvm_type_primitive_setPrintStringFunction(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t type = arguments[0];
    tuuvm_tuple_t function = arguments[1];
    tuuvm_type_setPrintStringFunction(type, function);

    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_type_primitive_setToStringFunction(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t type = arguments[0];
    tuuvm_tuple_t function = arguments[1];
    tuuvm_type_setToStringFunction(type, function);

    return TUUVM_VOID_TUPLE;
}

void tuuvm_type_setupPrimitives(tuuvm_context_t *context)
{
    tuuvm_context_setIntrinsicSymbolBinding(context, tuuvm_symbol_internWithCString(context, "Type::create"), tuuvm_function_createPrimitive(context, 0, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_type_primitive_create));
    
    tuuvm_context_setIntrinsicSymbolBinding(context, tuuvm_symbol_internWithCString(context, "Type::setPrintStringFunction:"), tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_type_primitive_setPrintStringFunction));
    tuuvm_context_setIntrinsicSymbolBinding(context, tuuvm_symbol_internWithCString(context, "Type::setToStringFunction:"), tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_type_primitive_setToStringFunction));
}
*/