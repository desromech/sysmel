#include "tuuvm/type.h"
#include "tuuvm/dictionary.h"
#include "tuuvm/assert.h"
#include "tuuvm/errors.h"
#include "tuuvm/function.h"
#include "tuuvm/string.h"
#include "tuuvm/type.h"
#include "internal/context.h"

TUUVM_API tuuvm_tuple_t tuuvm_typeSlot_create(tuuvm_context_t *context, tuuvm_tuple_t name, tuuvm_tuple_t flags, tuuvm_tuple_t type)
{
    tuuvm_typeSlot_t* result = (tuuvm_typeSlot_t*)tuuvm_context_allocatePointerTuple(context, context->roots.typeSlotType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_typeSlot_t));
    result->flags = flags;
    result->name = name;
    result->type = type;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_createAnonymous(tuuvm_context_t *context)
{
    tuuvm_type_tuple_t* result = (tuuvm_type_tuple_t*)tuuvm_context_allocatePointerTuple(context, context->roots.typeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_type_tuple_t));
    result->supertype = context->roots.anyValueType;
    result->totalSlotCount = tuuvm_tuple_size_encode(context, 0);
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_createAnonymousClass(tuuvm_context_t *context, tuuvm_tuple_t supertype, tuuvm_tuple_t metaclass)
{
    size_t classSlotCount = tuuvm_type_getTotalSlotCount(metaclass);
    TUUVM_ASSERT(classSlotCount >= TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_class_t));

    tuuvm_class_t* result = (tuuvm_class_t*)tuuvm_context_allocatePointerTuple(context, metaclass, classSlotCount);
    result->super.supertype = supertype;
    result->super.totalSlotCount = tuuvm_tuple_size_encode(context, 0);
    result->super.flags = tuuvm_tuple_size_encode(context, TUUVM_TYPE_FLAG_NULLABLE);
    if(supertype)
        result->super.totalSlotCount = tuuvm_tuple_size_encode(context, tuuvm_type_getTotalSlotCount(supertype));
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_createAnonymousMetaclass(tuuvm_context_t *context, tuuvm_tuple_t supertype)
{
    tuuvm_metaclass_t* result = (tuuvm_metaclass_t*)tuuvm_context_allocatePointerTuple(context, context->roots.metaclassType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_metaclass_t));
    result->super.supertype = supertype;
    result->super.flags = tuuvm_tuple_size_encode(context, TUUVM_TYPE_FLAG_NULLABLE);

    size_t slotCount = TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_class_t);
    if(supertype)
    {
        size_t superTypeSlotCount = tuuvm_type_getTotalSlotCount(supertype);
        if(superTypeSlotCount > slotCount)
            slotCount = superTypeSlotCount;
    }
    
    result->super.totalSlotCount = tuuvm_tuple_size_encode(context, slotCount);
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_createAnonymousClassAndMetaclass(tuuvm_context_t *context, tuuvm_tuple_t supertype)
{
    tuuvm_tuple_t metaclassSupertype = context->roots.classType;
    tuuvm_tuple_t actualSuperType = supertype;
    if(!supertype)
        actualSuperType = context->roots.objectType;

    if(tuuvm_tuple_isKindOf(context, actualSuperType, context->roots.classType))
        metaclassSupertype = tuuvm_tuple_getType(context, actualSuperType);
    else if(tuuvm_type_isSubtypeOf(actualSuperType, context->roots.typeType))
        metaclassSupertype = tuuvm_tuple_getType(context, tuuvm_type_getSupertype(actualSuperType));

    tuuvm_tuple_t metaclass = tuuvm_type_createAnonymousMetaclass(context, metaclassSupertype);
    tuuvm_tuple_t class = tuuvm_type_createAnonymousClass(context, actualSuperType, metaclass);

    // Link together the class with its metaclass.
    tuuvm_metaclass_t *metaclassObject = (tuuvm_metaclass_t*)metaclass;
    metaclassObject->thisClass = class;
    return class;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_createWithName(tuuvm_context_t *context, tuuvm_tuple_t name)
{
    tuuvm_tuple_t result = tuuvm_type_createAnonymous(context);
    tuuvm_type_setName(result, name);
    return result;
}

TUUVM_API size_t tuuvm_type_getTotalSlotCount(tuuvm_tuple_t type)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    return tuuvm_tuple_size_decode(((tuuvm_type_tuple_t*)type)->totalSlotCount);
}

TUUVM_API void tuuvm_type_setTotalSlotCount(tuuvm_context_t *context, tuuvm_tuple_t type, size_t totalSlotCount)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return;
    ((tuuvm_type_tuple_t*)type)->totalSlotCount = tuuvm_tuple_size_encode(context, totalSlotCount);
}

TUUVM_API size_t tuuvm_type_getFlags(tuuvm_tuple_t type)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    return tuuvm_tuple_size_decode(((tuuvm_type_tuple_t*)type)->flags);
}

TUUVM_API void tuuvm_type_setFlags(tuuvm_context_t *context, tuuvm_tuple_t type, size_t flags)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return;
    ((tuuvm_type_tuple_t*)type)->flags = tuuvm_tuple_size_encode(context, flags);
}

TUUVM_API tuuvm_tuple_t tuuvm_type_lookupMacroSelector(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t selector)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    tuuvm_tuple_t methodDictionary = tuuvm_type_getMacroMethodDictionary(type);
    if(methodDictionary)
    {
        tuuvm_tuple_t found = TUUVM_NULL_TUPLE;
        if(tuuvm_dictionary_find(context, methodDictionary, selector, &found))
            return found;
    }

    return tuuvm_type_lookupMacroSelector(context, tuuvm_type_getSupertype(type), selector);
}

TUUVM_API tuuvm_tuple_t tuuvm_type_lookupSelector(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t selector)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    tuuvm_tuple_t methodDictionary = tuuvm_type_getMethodDictionary(type);
    if(methodDictionary)
    {
        tuuvm_tuple_t found = TUUVM_NULL_TUPLE;
        if(tuuvm_dictionary_find(context, methodDictionary, selector, &found))
            return found;
    }

    return tuuvm_type_lookupSelector(context, tuuvm_type_getSupertype(type), selector);
}

TUUVM_API tuuvm_tuple_t tuuvm_type_lookupFallbackSelector(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t selector)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    tuuvm_tuple_t methodDictionary = tuuvm_type_getFallbackMethodDictionary(type);
    if(methodDictionary)
    {
        tuuvm_tuple_t found = TUUVM_NULL_TUPLE;
        if(tuuvm_dictionary_find(context, methodDictionary, selector, &found))
            return found;
    }

    return tuuvm_type_lookupFallbackSelector(context, tuuvm_type_getSupertype(type), selector);
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

TUUVM_API bool tuuvm_type_isSubtypeOf(tuuvm_tuple_t type, tuuvm_tuple_t supertype)
{
    if(!tuuvm_tuple_isNonNullPointer(supertype)) return false;
    if(!tuuvm_tuple_isNonNullPointer(type)) return false;
    if(type == supertype) return true;
    return tuuvm_type_isSubtypeOf(tuuvm_type_getSupertype(type), supertype);
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

TUUVM_API tuuvm_tuple_t tuuvm_type_getAnalyzeAndEvaluateMessageSendNodeForReceiverWithEnvironmentFunction(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    return tuuvm_type_lookupSelector(context, type, context->roots.analyzeAndEvaluateMessageSendNodeForReceiverWithEnvironmentSelector);
}

TUUVM_API void tuuvm_type_setAnalyzeAndEvaluateMessageSendNodeForReceiverWithEnvironmentFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t function)
{
    tuuvm_type_setMethodWithSelector(context, type, context->roots.analyzeAndEvaluateMessageSendNodeForReceiverWithEnvironmentSelector, function);
}

TUUVM_API tuuvm_tuple_t tuuvm_type_getAnalyzeMessageSendNodeWithEnvironmentFunction(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    return tuuvm_type_lookupSelector(context, type, context->roots.analyzeMessageSendNodeWithEnvironmentSelector);
}

TUUVM_API void tuuvm_type_setAnalyzeMessageSendNodeWithEnvironmentFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t function)
{
    tuuvm_type_setMethodWithSelector(context, type, context->roots.analyzeMessageSendNodeWithEnvironmentSelector, function);
}

TUUVM_API tuuvm_tuple_t tuuvm_type_getAnalyzeAndEvaluateMessageChainNodeForReceiverWithEnvironmentFunction(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    return tuuvm_type_lookupSelector(context, type, context->roots.analyzeAndEvaluateMessageChainNodeForReceiverWithEnvironmentSelector);
}

TUUVM_API void tuuvm_type_setAnalyzeAndEvaluateMessageChainNodeForReceiverWithEnvironmentFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t function)
{
    tuuvm_type_setMethodWithSelector(context, type, context->roots.analyzeAndEvaluateMessageChainNodeForReceiverWithEnvironmentSelector, function);
}

TUUVM_API tuuvm_tuple_t tuuvm_type_getAnalyzeMessageChainNodeWithEnvironmentFunction(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    return tuuvm_type_lookupSelector(context, type, context->roots.analyzeMessageChainNodeWithEnvironmentSelector);
}

TUUVM_API void tuuvm_type_setAnalyzeMessageChainNodeWithEnvironmentFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t function)
{
    tuuvm_type_setMethodWithSelector(context, type, context->roots.analyzeMessageChainNodeWithEnvironmentSelector, function);
}

TUUVM_API tuuvm_tuple_t tuuvm_type_getAnalyzeAndEvaluateConcreteMetaValueWithEnvironmentFunction(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    return tuuvm_type_lookupSelector(context, type, context->roots.analyzeAndEvaluateConcreteMetaValueWithEnvironmentSelector);
}

TUUVM_API void tuuvm_type_setAnalyzeAndEvaluateConcreteMetaValueWithEnvironmentFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t function)
{
    tuuvm_type_setMethodWithSelector(context, type, context->roots.analyzeAndEvaluateConcreteMetaValueWithEnvironmentSelector, function);
}

TUUVM_API tuuvm_tuple_t tuuvm_type_getAnalyzeConcreteMetaValueWithEnvironmentFunction(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    return tuuvm_type_lookupSelector(context, type, context->roots.analyzeConcreteMetaValueWithEnvironmentSelector);
}

TUUVM_API void tuuvm_type_setAnalyzeConcreteMetaValueWithEnvironmentFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t function)
{
    tuuvm_type_setMethodWithSelector(context, type, context->roots.analyzeConcreteMetaValueWithEnvironmentSelector, function);
}
TUUVM_API tuuvm_tuple_t tuuvm_type_getCoerceValueFunction(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    return tuuvm_type_lookupSelector(context, type, context->roots.coerceValueSelector);
}

TUUVM_API void tuuvm_type_setCoerceValueFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t coerceValueFunction)
{
    tuuvm_type_setMethodWithSelector(context, type, context->roots.coerceValueSelector, coerceValueFunction);
}

TUUVM_API tuuvm_tuple_t tuuvm_type_coerceValue(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t value)
{
    if(!type) return value;
    if(!value && tuuvm_type_isNullable(type)) return value;

    if(tuuvm_tuple_isKindOf(context, value, type))
        return value;

    tuuvm_tuple_t function = tuuvm_type_getCoerceValueFunction(context, type);
    if(function)
        return tuuvm_function_apply2(context, function, type, value);
    
    tuuvm_error("Cannot perform coercion of value into the required type.");
    return TUUVM_VOID_TUPLE;
}