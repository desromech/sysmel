#ifndef TUUVM_TYPE_H
#define TUUVM_TYPE_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef struct tuuvm_type_tuple_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t name;
    tuuvm_tuple_t owner;
    tuuvm_tuple_t supertype;
    tuuvm_tuple_t slots;
    tuuvm_tuple_t totalSlotCount;
    tuuvm_tuple_t flags;

    tuuvm_tuple_t macroMethodDictionary;
    tuuvm_tuple_t methodDictionary;
    tuuvm_tuple_t fallbackMethodDictionary;

    tuuvm_tuple_t pendingSlots;
    tuuvm_tuple_t subtypes;
} tuuvm_type_tuple_t;

typedef enum tuuvm_typeFlags_e
{
    TUUVM_TYPE_FLAGS_NONE = 0,
    TUUVM_TYPE_FLAGS_NULLABLE = 1<<0,
    TUUVM_TYPE_FLAGS_BYTES = 1<<1,
    TUUVM_TYPE_FLAGS_IMMEDIATE = 1<<2,

    TUUVM_TYPE_FLAGS_FINAL = 1<<3,
    TUUVM_TYPE_FLAGS_ABSTRACT = 1<<4,

    TUUVM_TYPE_FLAGS_POINTER_VALUE = 1<<5,
    TUUVM_TYPE_FLAGS_REFERENCE_VALUE = 1<<6,
    TUUVM_TYPE_FLAGS_POINTER_LIKE_VALUE = TUUVM_TYPE_FLAGS_POINTER_VALUE | TUUVM_TYPE_FLAGS_REFERENCE_VALUE,

    TUUVM_TYPE_FLAGS_CLASS_DEFAULT_FLAGS = TUUVM_TYPE_FLAGS_NULLABLE,
    TUUVM_TYPE_FLAGS_METATYPE_REQUIRED_FLAGS = TUUVM_TYPE_FLAGS_NULLABLE,

    TUUVM_TYPE_FLAGS_POINTER_TYPE_FLAGS = TUUVM_TYPE_FLAGS_NULLABLE | TUUVM_TYPE_FLAGS_POINTER_VALUE | TUUVM_TYPE_FLAGS_FINAL,
    TUUVM_TYPE_FLAGS_REFERENCE_TYPE_FLAGS = TUUVM_TYPE_FLAGS_FINAL | TUUVM_TYPE_FLAGS_REFERENCE_VALUE,

    TUUVM_TYPE_FLAGS_PRIMITIVE_VALUE_TYPE_DEFAULT_FLAGS = TUUVM_TYPE_FLAGS_IMMEDIATE | TUUVM_TYPE_FLAGS_FINAL,
    TUUVM_TYPE_FLAGS_PRIMITIVE_VALUE_TYPE_METATYPE_FLAGS = TUUVM_TYPE_FLAGS_METATYPE_REQUIRED_FLAGS | TUUVM_TYPE_FLAGS_FINAL,
} tuuvm_typeFlags_t;

typedef struct tuuvm_class_s
{
    tuuvm_type_tuple_t super;
} tuuvm_class_t;

typedef struct tuuvm_valueType_s
{
    tuuvm_type_tuple_t super;
} tuuvm_valueType_t;

typedef struct tuuvm_primitiveValueType_s
{
    tuuvm_valueType_t super;
} tuuvm_primitiveValueType_t;

typedef struct tuuvm_structureType_s
{
    tuuvm_valueType_t super;
} tuuvm_structureType_t;

typedef struct tuuvm_pointerLikeType_s
{
    tuuvm_valueType_t super;
    tuuvm_tuple_t baseType;
    tuuvm_tuple_t addressSpace;
    tuuvm_tuple_t loadValueFunction;
    tuuvm_tuple_t storeValueFunction;
} tuuvm_pointerLikeType_t;

typedef struct tuuvm_pointerType_s
{
    tuuvm_pointerLikeType_t super;
} tuuvm_pointerType_t;

typedef struct tuuvm_referenceType_s
{
    tuuvm_pointerLikeType_t super;
} tuuvm_referenceType_t;

typedef struct tuuvm_metatype_s
{
    tuuvm_type_tuple_t super;
    tuuvm_tuple_t thisType;
} tuuvm_metatype_t;

typedef struct tuuvm_metaclass_s
{
    tuuvm_metatype_t super;
} tuuvm_metaclass_t;

typedef struct tuuvm_valueMetatype_s
{
    tuuvm_metatype_t super;
} tuuvm_valueMetatype_t;

typedef struct tuuvm_typeSlot_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t name;
    tuuvm_tuple_t flags;
    tuuvm_tuple_t type;
} tuuvm_typeSlot_t;

typedef struct tuuvm_functionType_s
{
    tuuvm_type_tuple_t super;
} tuuvm_functionType_t;

typedef struct tuuvm_simpleFunctionType_s
{
    tuuvm_type_tuple_t super;
    tuuvm_tuple_t argumentTypes;
    tuuvm_tuple_t isVariadic;
    tuuvm_tuple_t resultType;
} tuuvm_simpleFunctionType_t;

typedef struct tuuvm_dependentFunctionType_s
{
    tuuvm_type_tuple_t super;
    tuuvm_tuple_t sourcePosition;

    tuuvm_tuple_t argumentNodes;
    tuuvm_tuple_t isVariadic;
    tuuvm_tuple_t resultTypeNode;

    tuuvm_tuple_t environment;
    tuuvm_tuple_t captureBindings;
    tuuvm_tuple_t argumentBindings;
    tuuvm_tuple_t localBindings;
} tuuvm_dependentFunctionType_t;

typedef enum tuuvm_typeSlotFlags_e
{
    TUUVM_TYPE_SLOT_FLAG_NONE = 0,
    TUUVM_TYPE_SLOT_FLAG_PUBLIC = 1<<0,
    TUUVM_TYPE_SLOT_FLAG_READONLY = 1<<1,
} tuuvm_typeSlotFlags_t;

/**
 * Creates a type slot
 */
TUUVM_API tuuvm_tuple_t tuuvm_typeSlot_create(tuuvm_context_t *context, tuuvm_tuple_t name, tuuvm_tuple_t flags, tuuvm_tuple_t type);

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
 * Creates an anonymous metaclass type.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_createAnonymousValueMetatype(tuuvm_context_t *context, tuuvm_tuple_t supertype, size_t minimumSlotCount);

/**
 * Creates an anonymous primitive type with respective meta value type.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_createAnonymousPrimitiveValueTypeAndValueMetatype(tuuvm_context_t *context, tuuvm_tuple_t supertype);

/**
 * Creates a type with the specified name.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_createWithName(tuuvm_context_t *context, tuuvm_tuple_t name);

/**
 * Creates a dependent function type.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_createDependentFunctionType(tuuvm_context_t *context, tuuvm_tuple_t argumentNodes, bool isVariadic, tuuvm_tuple_t resultTypeNode,
    tuuvm_tuple_t environment, tuuvm_tuple_t captureBindings, tuuvm_tuple_t argumentBindings, tuuvm_tuple_t localBindings);

/**
 * Creates a simple function type.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_createSimpleFunctionType(tuuvm_context_t *context, tuuvm_tuple_t argumentTypes, bool isVariadic, tuuvm_tuple_t resultType);

/**
 * Creates a simple function type with no arguments.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_createSimpleFunctionTypeWithArguments0(tuuvm_context_t *context, tuuvm_tuple_t resultType);

/**
 * Creates a simple function type with one arguments.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_createSimpleFunctionTypeWithArguments1(tuuvm_context_t *context, tuuvm_tuple_t argument, tuuvm_tuple_t resultType);

/**
 * Creates a simple function type with two arguments.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_createSimpleFunctionTypeWithArguments2(tuuvm_context_t *context, tuuvm_tuple_t argument0, tuuvm_tuple_t argument1, tuuvm_tuple_t resultType);

/**
 * Creates a pointer type.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_createPointerType(tuuvm_context_t *context, tuuvm_tuple_t baseType, tuuvm_tuple_t addressSpace);

/**
 * Creates a reference type.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_createReferenceType(tuuvm_context_t *context, tuuvm_tuple_t baseType, tuuvm_tuple_t addressSpace);

/**
 * Creates a function local reference type.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_createFunctionLocalReferenceType(tuuvm_context_t *context, tuuvm_tuple_t baseType);

/**
 * Creates a pointer type.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_createPointerType(tuuvm_context_t *context, tuuvm_tuple_t baseType, tuuvm_tuple_t addressSpace);

/**
 * Creates a value box with the given value.
 */
TUUVM_API tuuvm_tuple_t tuuvm_valueBox_with(tuuvm_context_t *context, tuuvm_tuple_t boxedValue);

/**
 * Creates a reference with the specified storage
 */
TUUVM_API tuuvm_tuple_t tuuvm_referenceType_withStorage(tuuvm_context_t *context, tuuvm_tuple_t referenceType, tuuvm_tuple_t storage);

/**
 * Creates a reference that boxes a value.
 */
TUUVM_API tuuvm_tuple_t tuuvm_referenceType_withBoxForValue(tuuvm_context_t *context, tuuvm_tuple_t referenceType, tuuvm_tuple_t boxedValue);

/**
 * Canonicalizes a function type
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_canonicalizeFunctionType(tuuvm_context_t *context, tuuvm_tuple_t functionType);

/**
 * Performs the lookup of the given macro selector.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_lookupMacroSelector(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t selector);

/**
 * Performs the lookup of the given selector.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_lookupSelector(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t selector);

/**
 * Performs the lookup of the given macro fallback selector.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_lookupFallbackSelector(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t selector);

/**
 * Sets the specified method with the given selector in the method dictionary.
 */
TUUVM_API void tuuvm_type_setMethodWithSelector(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t selector, tuuvm_tuple_t method);

/**
 * Is this type a subtype of?
 */
TUUVM_INLINE bool tuuvm_type_isDirectSubtypeOf(tuuvm_tuple_t type, tuuvm_tuple_t supertype)
{
    if(!tuuvm_tuple_isNonNullPointer(supertype)) return false;
    if(!tuuvm_tuple_isNonNullPointer(type)) return false;

    while(type)
    {
        if(type == supertype)
            return true;

        type = ((tuuvm_type_tuple_t *)type)->supertype;
    }

    return false;
}

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
TUUVM_INLINE tuuvm_tuple_t tuuvm_type_getSlots(tuuvm_tuple_t type)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_type_tuple_t*)type)->slots;
}

/**
 * Sets the slot names
 */
TUUVM_INLINE void tuuvm_type_setSlots(tuuvm_tuple_t type, tuuvm_tuple_t slots)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return;
    ((tuuvm_type_tuple_t*)type)->slots = slots;
}

/**
 * Gets the total slot count.
 */
TUUVM_API size_t tuuvm_type_getTotalSlotCount(tuuvm_tuple_t type);

/**
 * Sets the total slot count
 */
TUUVM_API void tuuvm_type_setTotalSlotCount(tuuvm_context_t *context, tuuvm_tuple_t type, size_t totalSlotCount);

/**
 * Gets the type flags.
 */
TUUVM_API size_t tuuvm_type_getFlags(tuuvm_tuple_t type);

/**
 * Sets the type flags.
 */
TUUVM_API void tuuvm_type_setFlags(tuuvm_context_t *context, tuuvm_tuple_t type, size_t flags);

/**
 * Sets the type and meta type flags.
 */
TUUVM_API void tuuvm_typeAndMetatype_setFlags(tuuvm_context_t *context, tuuvm_tuple_t type, size_t flags, size_t metatypeFlags);

/**
 * Is this a nullable type?
 */
TUUVM_INLINE bool tuuvm_type_isNullable(tuuvm_tuple_t type)
{
    return (tuuvm_type_getFlags(type) & TUUVM_TYPE_FLAGS_NULLABLE) != 0;
}

/**
 * Is this a pointer like value?
 */
TUUVM_INLINE bool tuuvm_type_isPointerLikeType(tuuvm_tuple_t type)
{
    return (tuuvm_type_getFlags(type) & TUUVM_TYPE_FLAGS_POINTER_LIKE_VALUE) != 0;
}

/**
 * Is this a pointer type?
 */
TUUVM_INLINE bool tuuvm_type_isPointerType(tuuvm_tuple_t type)
{
    return (tuuvm_type_getFlags(type) & TUUVM_TYPE_FLAGS_POINTER_VALUE) != 0;
}

/**
 * Is this a reference type?
 */
TUUVM_INLINE bool tuuvm_type_isReferenceType(tuuvm_tuple_t type)
{
    return (tuuvm_type_getFlags(type) & TUUVM_TYPE_FLAGS_REFERENCE_VALUE) != 0;
}

/**
 * Gets the macro method dictionary.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_type_getMacroMethodDictionary(tuuvm_tuple_t type)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_type_tuple_t*)type)->macroMethodDictionary;
}

/**
 * Gets the method dictionary.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_type_getMethodDictionary(tuuvm_tuple_t type)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_type_tuple_t*)type)->methodDictionary;
}

/**
 * Gets the macro fallback method dictionary.
 */
TUUVM_INLINE tuuvm_tuple_t tuuvm_type_getFallbackMethodDictionary(tuuvm_tuple_t type)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_type_tuple_t*)type)->fallbackMethodDictionary;
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
 * Gets the analyzeAndEvaluateMessageSendNode with environment function function of a type.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_getAnalyzeAndEvaluateMessageSendNodeForReceiverWithEnvironmentFunction(tuuvm_context_t *context, tuuvm_tuple_t type);

/**
 * Sets the analyzeAndEvaluateMessageSendNode with environment function function of a type.
 */
TUUVM_API void tuuvm_type_setAnalyzeAndEvaluateMessageSendNodeForReceiverWithEnvironmentFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t function);

/**
 * Gets the analyzeMessageSendNode with environment function function of a type.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_getAnalyzeMessageSendNodeWithEnvironmentFunction(tuuvm_context_t *context, tuuvm_tuple_t type);

/**
 * Sets the analyzeMessageSendNode with environment function function of a type.
 */
TUUVM_API void tuuvm_type_setAnalyzeMessageSendNodeWithEnvironmentFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t function);

/**
 * Gets the analyzeAndEvaluateMessageChainNode with environment function function of a type.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_getAnalyzeAndEvaluateMessageChainNodeForReceiverWithEnvironmentFunction(tuuvm_context_t *context, tuuvm_tuple_t type);

/**
 * Sets the analyzeAndEvaluateMessageChainNode with environment function function of a type.
 */
TUUVM_API void tuuvm_type_setAnalyzeAndEvaluateMessageChainNodeForReceiverWithEnvironmentFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t function);

/**
 * Gets the analyzeMessageChainNode with environment function function of a type.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_getAnalyzeMessageChainNodeWithEnvironmentFunction(tuuvm_context_t *context, tuuvm_tuple_t type);

/**
 * Sets the analyzeMessageChainNode with environment function function of a type.
 */
TUUVM_API void tuuvm_type_setAnalyzeMessageChainNodeWithEnvironmentFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t function);

/**
 * Gets the analyzeAndEvaluateMessageChainNode with environment function function of a type.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_getAnalyzeAndEvaluateConcreteMetaValueWithEnvironmentFunction(tuuvm_context_t *context, tuuvm_tuple_t type);

/**
 * Sets the analyzeAndEvaluateMessageChainNode with environment function function of a type.
 */
TUUVM_API void tuuvm_type_setAnalyzeAndEvaluateConcreteMetaValueWithEnvironmentFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t function);

/**
 * Gets the analyzeMessageChainNode with environment function function of a type.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_getAnalyzeConcreteMetaValueWithEnvironmentFunction(tuuvm_context_t *context, tuuvm_tuple_t type);

/**
 * Sets the analyzeMessageChainNode with environment function function of a type.
 */
TUUVM_API void tuuvm_type_setAnalyzeConcreteMetaValueWithEnvironmentFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t function);

/**
 * Gets the coerceValueFunction function of a type.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_getCoerceValueFunction(tuuvm_context_t *context, tuuvm_tuple_t type);

/**
 * Sets the coerceValueFunction function of a type
 */
TUUVM_API void tuuvm_type_setCoerceValueFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t coerceValueFunction);

/**
 * Gets the typeCheckFunction function of a type.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_getTypeCheckFunctionApplicationWithEnvironmentNode(tuuvm_context_t *context, tuuvm_tuple_t type);

/**
 * Sets the typeCheckFunctionApplication function of a type
 */
TUUVM_API void tuuvm_type_setTypeCheckFunctionApplicationWithEnvironmentNode(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t coerceValueFunction);

/**
 * Coerces a value into the specified type. Error if the coercion is not possible.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_coerceValue(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t value);

/**
 * Coerces a value into the specified type. Passing references in case of no type to coerce-
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_coerceValuePassingReferences(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t value);

/**
 * Loads the value from a pointer like type instance.
 */
TUUVM_API tuuvm_tuple_t tuuvm_pointerLikeType_load(tuuvm_context_t *context, tuuvm_tuple_t pointerLikeValue);

/**
 * Gets a decayed version of the type.
 */
TUUVM_API tuuvm_tuple_t tuuvm_type_decay(tuuvm_context_t *context, tuuvm_tuple_t type);

#endif //TUUVM_TYPE_H