#include "tuuvm/type.h"
#include "tuuvm/dictionary.h"
#include "tuuvm/errors.h"
#include "tuuvm/string.h"
#include "tuuvm/type.h"
#include "internal/context.h"

TUUVM_API tuuvm_tuple_t tuuvm_type_createAnonymous(tuuvm_context_t *context)
{
    tuuvm_type_tuple_t* result = (tuuvm_type_tuple_t*)tuuvm_context_allocatePointerTuple(context, context->roots.typeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_type_tuple_t));
    result->supertype = context->roots.anyValueType;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_createWithName(tuuvm_context_t *context, tuuvm_tuple_t name)
{
    tuuvm_tuple_t result = tuuvm_type_createAnonymous(context);
    tuuvm_type_setName(result, name);
    return result;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_lookupSelector(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t selector)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    tuuvm_tuple_t methodDictionary = tuuvm_type_getMethodDictonary(type);
    if(methodDictionary)
    {
        tuuvm_tuple_t found = TUUVM_NULL_TUPLE;
        if(tuuvm_dictionary_find(context, methodDictionary, selector, &found))
            return found;
    }

    return tuuvm_type_lookupSelector(context, tuuvm_type_getSupertype(type), selector);
}

static tuuvm_tuple_t tuuvm_type_lookupSelectorOrFallbackWith(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t selector, tuuvm_tuple_t fallbackMethod)
{
    tuuvm_tuple_t found = tuuvm_type_lookupSelector(context, type, selector);
    if(found)
        return found;
    else
        return fallbackMethod;
}

TUUVM_API void tuuvm_type_setMethodWithSelector(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t selector, tuuvm_tuple_t method)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return;

    tuuvm_type_tuple_t* typeObject = (tuuvm_type_tuple_t*)type;
    if(!typeObject->methodDictionary)
        typeObject->methodDictionary = tuuvm_identityDictionary_create(context);
    tuuvm_dictionary_atPut(context, typeObject->methodDictionary, selector, method);
}

TUUVM_API tuuvm_tuple_t tuuvm_type_getEqualsFunction(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    return tuuvm_type_lookupSelectorOrFallbackWith(context, type, context->roots.equalsSelector, context->roots.identityEqualsFunction);
}

TUUVM_API void tuuvm_type_setEqualsFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t equalsFunction)
{
    tuuvm_type_setMethodWithSelector(context, type, context->roots.equalsSelector, equalsFunction);
}

TUUVM_API tuuvm_tuple_t tuuvm_type_getHashFunction(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    return tuuvm_type_lookupSelectorOrFallbackWith(context, type, context->roots.hashSelector, context->roots.identityHashFunction);
}

TUUVM_API void tuuvm_type_setHashFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t hashFunction)
{
    tuuvm_type_setMethodWithSelector(context, type, context->roots.hashSelector, hashFunction);
}

TUUVM_API tuuvm_tuple_t tuuvm_type_getAsStringFunction(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    return tuuvm_type_lookupSelector(context, type, context->roots.asStringSelector);
}

TUUVM_API void tuuvm_type_setAsStringFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t asStringFunction)
{
    tuuvm_type_setMethodWithSelector(context, type, context->roots.asStringSelector, asStringFunction);
}

TUUVM_API tuuvm_tuple_t tuuvm_type_getPrintStringFunction(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    return tuuvm_type_lookupSelector(context, type, context->roots.printStringSelector);
}

TUUVM_API void tuuvm_type_setPrintStringFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t printStringFunction)
{
    tuuvm_type_setMethodWithSelector(context, type, context->roots.printStringSelector, printStringFunction);
}

TUUVM_API tuuvm_tuple_t tuuvm_type_getAstNodeAnalysisFunction(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    return tuuvm_type_lookupSelector(context, type, context->roots.astNodeAnalysisSelector);
}

TUUVM_API void tuuvm_type_setAstNodeAnalysisFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t astNodeAnalysisFunction)
{
    tuuvm_type_setMethodWithSelector(context, type, context->roots.astNodeAnalysisSelector, astNodeAnalysisFunction);
}

TUUVM_API tuuvm_tuple_t tuuvm_type_getAstNodeEvaluationFunction(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    return tuuvm_type_lookupSelector(context, type, context->roots.astNodeEvaluationSelector);
}

TUUVM_API void tuuvm_type_setAstNodeEvaluationFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t astNodeEvaluationFunction)
{
    tuuvm_type_setMethodWithSelector(context, type, context->roots.astNodeEvaluationSelector, astNodeEvaluationFunction);
}

TUUVM_API tuuvm_tuple_t tuuvm_type_getAstNodeAnalysisAndEvaluationFunction(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    return tuuvm_type_lookupSelector(context, type, context->roots.astNodeAnalysisAndEvaluationSelector);
}

TUUVM_API void tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t astNodeAnalysisAndEvaluationFunction)
{
    tuuvm_type_setMethodWithSelector(context, type, context->roots.astNodeAnalysisAndEvaluationSelector, astNodeAnalysisAndEvaluationFunction);
}

TUUVM_API tuuvm_tuple_t tuuvm_type_coerceValue(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t value)
{
    if(!tuuvm_tuple_isKindOf(context, value, type))
        tuuvm_error("Cannot perform coercion of value into the required type.");
    return value;
}