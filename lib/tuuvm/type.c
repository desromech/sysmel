#include "tuuvm/type.h"
#include "tuuvm/dictionary.h"
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

TUUVM_API tuuvm_tuple_t tuuvm_type_getEqualsFunction(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_type_tuple_t*)type)->equalsFunction;
}

TUUVM_API void tuuvm_type_setEqualsFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t equalsFunction)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return;
    ((tuuvm_type_tuple_t*)type)->equalsFunction = equalsFunction;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_getHashFunction(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_type_tuple_t*)type)->hashFunction;
}

TUUVM_API void tuuvm_type_setHashFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t hashFunction)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return;
    ((tuuvm_type_tuple_t*)type)->hashFunction = hashFunction;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_getToStringFunction(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_type_tuple_t*)type)->toStringFunction;
}

TUUVM_API void tuuvm_type_setToStringFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t toStringFunction)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return;
    ((tuuvm_type_tuple_t*)type)->toStringFunction = toStringFunction;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_getPrintStringFunction(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_type_tuple_t*)type)->printStringFunction;
}

TUUVM_API void tuuvm_type_setPrintStringFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t printStringFunction)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return;
    ((tuuvm_type_tuple_t*)type)->printStringFunction = printStringFunction;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_getAstNodeAnalysisFunction(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_type_tuple_t*)type)->astNodeAnalysisFunction;
}

TUUVM_API void tuuvm_type_setAstNodeAnalysisFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t astNodeAnalysisFunction)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return;
    ((tuuvm_type_tuple_t*)type)->astNodeAnalysisFunction = astNodeAnalysisFunction;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_getAstNodeEvaluationFunction(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_type_tuple_t*)type)->astNodeEvaluationFunction;
}

TUUVM_API void tuuvm_type_setAstNodeEvaluationFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t astNodeEvaluationFunction)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return;
    ((tuuvm_type_tuple_t*)type)->astNodeEvaluationFunction = astNodeEvaluationFunction;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_getAstNodeAnalysisAndEvaluationFunction(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_type_tuple_t*)type)->astNodeAnalysisAndEvaluationFunction;
}

TUUVM_API void tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t astNodeAnalysisAndEvaluationFunction)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return;
    ((tuuvm_type_tuple_t*)type)->astNodeAnalysisAndEvaluationFunction = astNodeAnalysisAndEvaluationFunction;
}