#ifndef TUUVM_TYPE_H
#define TUUVM_TYPE_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef struct tuuvm_type_tuple_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t name;
    tuuvm_tuple_t supertype;
    tuuvm_tuple_t slotNames;
    tuuvm_tuple_t sumTypeAlternatives;
    tuuvm_tuple_t totalSlotCount;
    tuuvm_tuple_t flags;

    tuuvm_tuple_t macroMethodDictionary;
    tuuvm_tuple_t methodDictionary;
    tuuvm_tuple_t fallbackMethodDictionary;
} tuuvm_type_tuple_t;

typedef struct tuuvm_class_s
{
    tuuvm_type_tuple_t super;
} tuuvm_class_t;

typedef struct tuuvm_metaclass_s
{
    tuuvm_type_tuple_t super;
    tuuvm_tuple_t thisClass;
} tuuvm_metaclass_t;

/**
 * Creates an anonymous type.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_createAnonymous(tuuvm_context_t *context);

/**
 * Creates an anonymous class type.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_createAnonymousClass(tuuvm_context_t *context, tuuvm_tuple_t supertype, tuuvm_tuple_t metaclass);

/**
 * Creates an anonymous metaclass type.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_createAnonymousMetaclass(tuuvm_context_t *context, tuuvm_tuple_t supertype);

/**
 * Creates an anonymous class with respective metaclass.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_createAnonymousClassAndMetaclass(tuuvm_context_t *context, tuuvm_tuple_t supertype);

/**
 * Creates a type with the specified name.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_createWithName(tuuvm_context_t *context, tuuvm_tuple_t name);

/**
 * Performs the lookup of the given selector.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_lookupSelector(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t selector);

/**
 * Sets the specified method with the given selector in the method dictionary.
 */
TUUVM_API void tuuvm_type_setMethodWithSelector(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t selector, tuuvm_tuple_t method);

/**
 * Is this type a subtype of?
 */
TUUVM_API bool tuuvm_type_isSubtypeOf(tuuvm_tuple_t type, tuuvm_tuple_t supertype);

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
 * Gets the supertype
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_type_getSupertype(tuuvm_tuple_t type)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_type_tuple_t*)type)->supertype;
}

/**
 * Sets the supertype
 */
TUUVM_INLINE void tuuvm_type_setSupertype(tuuvm_tuple_t type, tuuvm_tuple_t supertype)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return;
    ((tuuvm_type_tuple_t*)type)->supertype = supertype;
}


/**
 * Gets the slot names
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_type_getSlotNames(tuuvm_tuple_t type)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_type_tuple_t*)type)->slotNames;
}

/**
 * Sets the slot names
 */
TUUVM_INLINE void tuuvm_type_setSlotNames(tuuvm_tuple_t type, tuuvm_tuple_t slotNames)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return;
    ((tuuvm_type_tuple_t*)type)->slotNames = slotNames;
}

/**
 * Gets the total slot count.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_type_getTotalSlotCount(tuuvm_tuple_t type)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_type_tuple_t*)type)->totalSlotCount;
}

/**
 * Sets the total slot count
 */
TUUVM_INLINE void tuuvm_type_setTotalSlotCount(tuuvm_tuple_t type, tuuvm_tuple_t totalSlotCount)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return;
    ((tuuvm_type_tuple_t*)type)->totalSlotCount = totalSlotCount;
}

/**
 * Gets the type flags.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_type_getFlags(tuuvm_tuple_t type)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_type_tuple_t*)type)->flags;
}

/**
 * Sets the type flags.
 */
TUUVM_INLINE void tuuvm_type_setFlags(tuuvm_tuple_t type, tuuvm_tuple_t flags)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return;
    ((tuuvm_type_tuple_t*)type)->flags = flags;
}

/**
 * Gets the method dictionary.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_type_getMethodDictonary(tuuvm_tuple_t type)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_type_tuple_t*)type)->methodDictionary;
}

/**
 * Sets the method dictionary.
 */
TUUVM_INLINE void tuuvm_type_setMethodDictionary(tuuvm_tuple_t type, tuuvm_tuple_t methodDictionary)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return;
    ((tuuvm_type_tuple_t*)type)->methodDictionary = methodDictionary;
}

/**
 * Gets the equals function of a type
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_getEqualsFunction(tuuvm_context_t *context, tuuvm_tuple_t type);

/**
 * Sets the equals function of a type
 */
TUUVM_API void tuuvm_type_setEqualsFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t equalsFunction);

/**
 * Gets the hash function of a type
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_getHashFunction(tuuvm_context_t *context, tuuvm_tuple_t type);

/**
 * Sets the hash function of a type
 */
TUUVM_API void tuuvm_type_setHashFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t hashFunction);

/**
 * Gets the asString function of a type.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_getAsStringFunction(tuuvm_context_t *context, tuuvm_tuple_t type);

/**
 * Sets the asString function of a type
 */
TUUVM_API void tuuvm_type_setAsStringFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t asStringFunction);

/**
 * Gets the printString function of a type.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_getPrintStringFunction(tuuvm_context_t *context, tuuvm_tuple_t type);

/**
 * Sets the printString function of a type
 */
TUUVM_API void tuuvm_type_setPrintStringFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t printStringFunction);

/**
 * Gets the astNodeAnalysisFunction function of a type.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_getAstNodeAnalysisFunction(tuuvm_context_t *context, tuuvm_tuple_t type);

/**
 * Sets the astNodeAnalysisFunction function of a type
 */
TUUVM_API void tuuvm_type_setAstNodeAnalysisFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t astNodeAnalysisFunction);

/**
 * Gets the astNodeEvaluationFunction function of a type.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_getAstNodeEvaluationFunction(tuuvm_context_t *context, tuuvm_tuple_t type);

/**
 * Sets the astNodeEvaluationFunction function of a type
 */
TUUVM_API void tuuvm_type_setAstNodeEvaluationFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t astNodeEvaluationFunction);

/**
 * Gets the astNodeAnalysisAndEvaluationFunction function of a type.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_getAstNodeAnalysisAndEvaluationFunction(tuuvm_context_t *context, tuuvm_tuple_t type);

/**
 * Sets the astNodeAnalysisAndEvaluationFunction function of a type
 */
TUUVM_API void tuuvm_type_setAstNodeAnalysisAndEvaluationFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t astNodeAnalysisAndEvaluationFunction);

/**
 * Coerces a value into the specified type. Error if the coercion is not possible.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_coerceValue(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t value);


#endif //TUUVM_TYPE_H