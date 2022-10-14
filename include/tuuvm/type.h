#ifndef TUUVM_TYPE_H
#define TUUVM_TYPE_H

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef struct tuuvm_type_tuple_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t name;
    tuuvm_tuple_t supertype;
    tuuvm_tuple_t slotNames;

    tuuvm_tuple_t equalsFunction;
    tuuvm_tuple_t hashFunction;
    tuuvm_tuple_t toStringFunction;
    tuuvm_tuple_t printStringFunction;

    tuuvm_tuple_t astNodeAnalysisFunction;
    tuuvm_tuple_t astNodeEvaluationFunction;
    tuuvm_tuple_t astNodeAnalysisAndEvaluationFunction;

} tuuvm_type_tuple_t;

/**
 * Creates an anonymous type.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_createAnonymous(tuuvm_context_t *context);

/**
 * Creates a type with the specified name.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_createWithName(tuuvm_context_t *context, tuuvm_tuple_t name);

/**
 * Gets the name of a type
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_type_getName(tuuvm_tuple_t type)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_type_tuple_t*)type)->name;
}

/**
 * Sets the name of a type
 */
TUUVM_INLINE void tuuvm_type_setName(tuuvm_tuple_t type, tuuvm_tuple_t name)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return;
    ((tuuvm_type_tuple_t*)type)->name = name;
}

/**
 * Gets the equals function of a type
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_type_getEqualsFunction(tuuvm_tuple_t type)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_type_tuple_t*)type)->equalsFunction;
}

/**
 * Sets the equals function of a type
 */
TUUVM_INLINE void tuuvm_type_setEqualsFunction(tuuvm_tuple_t type, tuuvm_tuple_t equalsFunction)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return;
    ((tuuvm_type_tuple_t*)type)->equalsFunction = equalsFunction;
}

/**
 * Gets the hash function of a type
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_type_getHashFunction(tuuvm_tuple_t type)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_type_tuple_t*)type)->hashFunction;
}

/**
 * Sets the hash function of a type
 */
TUUVM_INLINE void tuuvm_type_setHashFunction(tuuvm_tuple_t type, tuuvm_tuple_t hashFunction)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return;
    ((tuuvm_type_tuple_t*)type)->hashFunction = hashFunction;
}

/**
 * Gets the toString function of a type.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_type_getToStringFunction(tuuvm_tuple_t type)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_type_tuple_t*)type)->toStringFunction;
}

/**
 * Sets the toString function of a type
 */
TUUVM_INLINE void tuuvm_type_setToStringFunction(tuuvm_tuple_t type, tuuvm_tuple_t toStringFunction)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return;
    ((tuuvm_type_tuple_t*)type)->toStringFunction = toStringFunction;
}

/**
 * Gets the printString function of a type.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_type_getPrintStringFunction(tuuvm_tuple_t type)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_type_tuple_t*)type)->printStringFunction;
}

/**
 * Sets the printString function of a type
 */
TUUVM_INLINE void tuuvm_type_setPrintStringFunction(tuuvm_tuple_t type, tuuvm_tuple_t printStringFunction)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return;
    ((tuuvm_type_tuple_t*)type)->printStringFunction = printStringFunction;
}

/**
 * Gets the astNodeAnalysisFunction function of a type.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_type_getAstNodeAnalysisFunction(tuuvm_tuple_t type)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_type_tuple_t*)type)->astNodeAnalysisFunction;
}

/**
 * Sets the astNodeAnalysisFunction function of a type
 */
TUUVM_INLINE void tuuvm_type_setAstNodeAnalysisFunction(tuuvm_tuple_t type, tuuvm_tuple_t astNodeAnalysisFunction)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return;
    ((tuuvm_type_tuple_t*)type)->astNodeAnalysisFunction = astNodeAnalysisFunction;
}

/**
 * Gets the astNodeEvaluationFunction function of a type.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_type_getAstNodeEvaluationFunction(tuuvm_tuple_t type)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_type_tuple_t*)type)->astNodeEvaluationFunction;
}

/**
 * Sets the astNodeEvaluationFunction function of a type
 */
TUUVM_INLINE void tuuvm_type_setAstNodeEvaluationFunction(tuuvm_tuple_t type, tuuvm_tuple_t astNodeEvaluationFunction)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return;
    ((tuuvm_type_tuple_t*)type)->astNodeEvaluationFunction = astNodeEvaluationFunction;
}

/**
 * Gets the astNodeAnalysisAndEvaluationFunction function of a type.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_type_getAstNodeAnalysisAndEvaluationFunction(tuuvm_tuple_t type)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_type_tuple_t*)type)->astNodeAnalysisAndEvaluationFunction;
}

/**
 * Sets the astNodeAnalysisAndEvaluationFunction function of a type
 */
TUUVM_INLINE void tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(tuuvm_tuple_t type, tuuvm_tuple_t astNodeAnalysisAndEvaluationFunction)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return;
    ((tuuvm_type_tuple_t*)type)->astNodeAnalysisAndEvaluationFunction = astNodeAnalysisAndEvaluationFunction;
}

#endif //TUUVM_TYPE_H