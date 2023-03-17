#include "tuuvm/type.h"
#include "tuuvm/array.h"
#include "tuuvm/assert.h"
#include "tuuvm/dictionary.h"
#include "tuuvm/errors.h"
#include "tuuvm/function.h"
#include "tuuvm/stackFrame.h"
#include "tuuvm/string.h"
#include "tuuvm/type.h"
#include "internal/context.h"
#include <string.h>

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
    result->super.flags = tuuvm_tuple_size_encode(context, TUUVM_TYPE_FLAGS_CLASS_DEFAULT_FLAGS);
    if(supertype)
        result->super.totalSlotCount = tuuvm_tuple_size_encode(context, tuuvm_type_getTotalSlotCount(supertype));
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_createAnonymousMetaclass(tuuvm_context_t *context, tuuvm_tuple_t supertype)
{
    tuuvm_metaclass_t* result = (tuuvm_metaclass_t*)tuuvm_context_allocatePointerTuple(context, context->roots.metaclassType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_metaclass_t));
    result->super.super.supertype = supertype;
    result->super.super.flags = tuuvm_tuple_size_encode(context, TUUVM_TYPE_FLAGS_METATYPE_REQUIRED_FLAGS);

    size_t slotCount = TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_class_t);
    if(supertype)
    {
        size_t superTypeSlotCount = tuuvm_type_getTotalSlotCount(supertype);
        if(superTypeSlotCount > slotCount)
            slotCount = superTypeSlotCount;
    }
    
    result->super.super.totalSlotCount = tuuvm_tuple_size_encode(context, slotCount);
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
    metaclassObject->super.thisType = class;
    return class;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_createAnonymousPrimitiveValueType(tuuvm_context_t *context, tuuvm_tuple_t supertype, tuuvm_tuple_t metaclass)
{
    size_t typeSlotCount = tuuvm_type_getTotalSlotCount(metaclass);
    TUUVM_ASSERT(typeSlotCount >= TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_primitiveValueType_t));

    tuuvm_primitiveValueType_t* result = (tuuvm_primitiveValueType_t*)tuuvm_context_allocatePointerTuple(context, metaclass, typeSlotCount);
    result->super.super.supertype = supertype;
    result->super.super.totalSlotCount = tuuvm_tuple_size_encode(context, 0);
    result->super.super.flags = tuuvm_tuple_size_encode(context, TUUVM_TYPE_FLAGS_PRIMITIVE_VALUE_TYPE_DEFAULT_FLAGS);
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_createAnonymousValueMetatype(tuuvm_context_t *context, tuuvm_tuple_t supertype, size_t minimumSlotCount)
{
    tuuvm_valueMetatype_t* result = (tuuvm_valueMetatype_t*)tuuvm_context_allocatePointerTuple(context, context->roots.valueMetatypeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_valueMetatype_t));
    result->super.super.supertype = supertype;
    result->super.super.flags = tuuvm_tuple_size_encode(context, TUUVM_TYPE_FLAGS_METATYPE_REQUIRED_FLAGS);

    size_t slotCount = minimumSlotCount;
    if(supertype)
    {
        size_t superTypeSlotCount = tuuvm_type_getTotalSlotCount(supertype);
        if(superTypeSlotCount > slotCount)
            slotCount = superTypeSlotCount;
    }
    
    result->super.super.totalSlotCount = tuuvm_tuple_size_encode(context, slotCount);
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_createAnonymousPrimitiveValueTypeAndValueMetatype(tuuvm_context_t *context, tuuvm_tuple_t supertype)
{
    tuuvm_tuple_t metatypeSupertype = context->roots.primitiveValueType;
    tuuvm_tuple_t actualSuperType = supertype;

    tuuvm_tuple_t metatype = tuuvm_type_createAnonymousValueMetatype(context, metatypeSupertype, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_primitiveValueType_t));
    tuuvm_tuple_t primitiveValueType = tuuvm_type_createAnonymousPrimitiveValueType(context, actualSuperType, metatype);

    tuuvm_type_setFlags(context, metatype, TUUVM_TYPE_FLAGS_PRIMITIVE_VALUE_TYPE_METATYPE_FLAGS);

    // Link together the type with its metatype.
    tuuvm_metatype_t *metatypeObject = (tuuvm_metatype_t*)metatype;
    metatypeObject->thisType = primitiveValueType;
    return primitiveValueType;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_createWithName(tuuvm_context_t *context, tuuvm_tuple_t name)
{
    tuuvm_tuple_t result = tuuvm_type_createAnonymous(context);
    tuuvm_type_setName(result, name);
    return result;
}

static tuuvm_tuple_t tuuvm_type_doCreateSimpleFunctionType(tuuvm_context_t *context, tuuvm_tuple_t argumentTypes, tuuvm_tuple_t isVariadic, tuuvm_tuple_t resultType)
{
    tuuvm_type_tuple_t *supertype = (tuuvm_type_tuple_t*)context->roots.functionType;
    
    tuuvm_simpleFunctionType_t* result = (tuuvm_simpleFunctionType_t*)tuuvm_context_allocatePointerTuple(context, context->roots.simpleFunctionTypeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_simpleFunctionType_t));
    result->super.supertype = (tuuvm_tuple_t)supertype;
    result->super.flags = supertype->flags;
    result->super.totalSlotCount = supertype->totalSlotCount;
    result->argumentTypes = argumentTypes;
    result->isVariadic = isVariadic;
    result->resultType = resultType;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_createSimpleFunctionType(tuuvm_context_t *context, tuuvm_tuple_t argumentTypes, bool isVariadic, tuuvm_tuple_t resultType)
{
    return tuuvm_function_apply3(context, context->roots.simpleFunctionTypeTemplate, argumentTypes, tuuvm_tuple_boolean_encode(isVariadic), resultType);
}

static tuuvm_tuple_t tuuvm_pointerLikeType_createPointerLikeLoadFunction(tuuvm_context_t *context, tuuvm_tuple_t pointerLikeType_, tuuvm_tuple_t baseType_, size_t extraFlags)
{
    struct {
        tuuvm_tuple_t pointerLikeType;
        tuuvm_tuple_t baseType;
        tuuvm_tuple_t function;
        tuuvm_tuple_t functionTypeArguments;
        tuuvm_tuple_t functionType;
    } gcFrame = {
        .pointerLikeType = pointerLikeType_,
        .baseType = baseType_,
    };
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.function = tuuvm_context_shallowCopy(context, context->roots.pointerLikeLoadPrimitive);
    tuuvm_function_addFlags(context, gcFrame.function, extraFlags);

    gcFrame.functionTypeArguments = tuuvm_array_create(context, 1);
    tuuvm_array_atPut(gcFrame.functionTypeArguments, 0, gcFrame.pointerLikeType);

    gcFrame.functionType = tuuvm_type_createSimpleFunctionType(context, gcFrame.functionTypeArguments, false, gcFrame.baseType);
    tuuvm_tuple_setType((tuuvm_object_tuple_t*)gcFrame.function, gcFrame.functionType);

    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.function;
}

static tuuvm_tuple_t tuuvm_pointerLikeType_createPointerLikeStoreFunction(tuuvm_context_t *context, tuuvm_tuple_t pointerLikeType_, tuuvm_tuple_t baseType_, size_t extraFlags)
{
    struct {
        tuuvm_tuple_t pointerLikeType;
        tuuvm_tuple_t baseType;
        tuuvm_tuple_t function;
        tuuvm_tuple_t functionTypeArguments;
        tuuvm_tuple_t functionType;
    } gcFrame = {
        .pointerLikeType = pointerLikeType_,
        .baseType = baseType_,
    };
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.function = tuuvm_context_shallowCopy(context, context->roots.pointerLikeStorePrimitive);
    tuuvm_function_addFlags(context, gcFrame.function, extraFlags);

    gcFrame.functionTypeArguments = tuuvm_array_create(context, 2);
    tuuvm_array_atPut(gcFrame.functionTypeArguments, 0, gcFrame.pointerLikeType);
    tuuvm_array_atPut(gcFrame.functionTypeArguments, 1, gcFrame.baseType);

    gcFrame.functionType = tuuvm_type_createSimpleFunctionType(context, gcFrame.functionTypeArguments, false, gcFrame.pointerLikeType);
    tuuvm_tuple_setType((tuuvm_object_tuple_t*)gcFrame.function, gcFrame.functionType);

    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.function;
}

static tuuvm_tuple_t tuuvm_type_doCreatePointerType(tuuvm_context_t *context, tuuvm_tuple_t baseType_, tuuvm_tuple_t addressSpace_)
{
    struct {
        tuuvm_tuple_t baseType;
        tuuvm_tuple_t addressSpace;
        tuuvm_pointerType_t* result;
        tuuvm_tuple_t storeFunction;
        tuuvm_tuple_t loadFunction;

    } gcFrame = {
        .baseType = baseType_,
        .addressSpace = addressSpace_,
    };
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.result = (tuuvm_pointerType_t*)tuuvm_context_allocatePointerTuple(context, context->roots.pointerType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_pointerType_t));
    gcFrame.result->super.super.super.flags = tuuvm_tuple_size_encode(context, TUUVM_TYPE_FLAGS_POINTER_TYPE_FLAGS);
    gcFrame.result->super.super.super.totalSlotCount = tuuvm_tuple_size_encode(context, 0);
    gcFrame.result->super.super.super.supertype = context->roots.anyPointerType;
    gcFrame.result->super.baseType = gcFrame.baseType;
    gcFrame.result->super.addressSpace = gcFrame.addressSpace;

    gcFrame.storeFunction = tuuvm_pointerLikeType_createPointerLikeStoreFunction(context, (tuuvm_tuple_t)gcFrame.result, gcFrame.baseType, 0);
    gcFrame.result->super.storeValueFunction = gcFrame.storeFunction;

    gcFrame.loadFunction = tuuvm_pointerLikeType_createPointerLikeLoadFunction(context, (tuuvm_tuple_t)gcFrame.result, gcFrame.baseType, 0);
    gcFrame.result->super.loadValueFunction = gcFrame.loadFunction;

    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (tuuvm_tuple_t)gcFrame.result;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_createPointerType(tuuvm_context_t *context, tuuvm_tuple_t baseType, tuuvm_tuple_t addressSpace)
{
    return tuuvm_function_apply2(context, context->roots.pointerTypeTemplate, baseType, addressSpace);
}

static tuuvm_tuple_t tuuvm_type_doCreateReferenceType(tuuvm_context_t *context, tuuvm_tuple_t baseType_, tuuvm_tuple_t addressSpace_)
{
    struct {
        tuuvm_tuple_t baseType;
        tuuvm_tuple_t addressSpace;
        tuuvm_referenceType_t* result;
        tuuvm_tuple_t storeFunction;
        tuuvm_tuple_t loadFunction;

    } gcFrame = {
        .baseType = baseType_,
        .addressSpace = addressSpace_,
    };
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.result = (tuuvm_referenceType_t*)tuuvm_context_allocatePointerTuple(context, context->roots.referenceType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_referenceType_t));
    gcFrame.result->super.super.super.flags = tuuvm_tuple_size_encode(context, TUUVM_TYPE_FLAGS_REFERENCE_TYPE_FLAGS);
    gcFrame.result->super.super.super.totalSlotCount = tuuvm_tuple_size_encode(context, 0);
    gcFrame.result->super.super.super.supertype = context->roots.anyReferenceType;
    gcFrame.result->super.baseType = gcFrame.baseType;
    gcFrame.result->super.addressSpace = gcFrame.addressSpace;

    gcFrame.storeFunction = tuuvm_pointerLikeType_createPointerLikeStoreFunction(context, (tuuvm_tuple_t)gcFrame.result, gcFrame.baseType, TUUVM_FUNCTION_FLAGS_ALLOW_REFERENCE_IN_RECEIVER);
    tuuvm_type_setMethodWithSelector(context, (tuuvm_tuple_t)gcFrame.result, context->roots.assignmentSelector, gcFrame.storeFunction);
    gcFrame.result->super.storeValueFunction = gcFrame.storeFunction;

    gcFrame.loadFunction = tuuvm_pointerLikeType_createPointerLikeLoadFunction(context, (tuuvm_tuple_t)gcFrame.result, gcFrame.baseType, TUUVM_FUNCTION_FLAGS_ALLOW_REFERENCE_IN_RECEIVER);
    gcFrame.result->super.loadValueFunction = gcFrame.loadFunction;

    return (tuuvm_tuple_t)gcFrame.result;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_createReferenceType(tuuvm_context_t *context, tuuvm_tuple_t baseType, tuuvm_tuple_t addressSpace)
{
    return tuuvm_function_apply2(context, context->roots.referenceTypeTemplate, baseType, addressSpace);
}

TUUVM_API tuuvm_tuple_t tuuvm_type_createFunctionLocalReferenceType(tuuvm_context_t *context, tuuvm_tuple_t baseType)
{
    return tuuvm_type_createReferenceType(context, baseType, TUUVM_NULL_TUPLE);
}

TUUVM_API tuuvm_tuple_t tuuvm_valueBox_with(tuuvm_context_t *context, tuuvm_tuple_t boxedValue)
{
    tuuvm_object_tuple_t *box = tuuvm_context_allocatePointerTuple(context, context->roots.valueBoxType, 1);
    box->pointers[0] = boxedValue;
    return (tuuvm_tuple_t)box;
}

TUUVM_API tuuvm_tuple_t tuuvm_referenceType_withStorage(tuuvm_context_t *context, tuuvm_tuple_t referenceType, tuuvm_tuple_t storage)
{
    if(!tuuvm_tuple_isKindOf(context, referenceType, context->roots.referenceType))
        tuuvm_error("Expected a reference type");

    tuuvm_object_tuple_t *reference = tuuvm_context_allocatePointerTuple(context, referenceType, 1);
    reference->pointers[0] = storage;
    return (tuuvm_tuple_t)reference;
}

TUUVM_API tuuvm_tuple_t tuuvm_referenceType_withBoxForValue(tuuvm_context_t *context, tuuvm_tuple_t referenceType, tuuvm_tuple_t boxedValue)
{
    return tuuvm_referenceType_withStorage(context, referenceType, tuuvm_valueBox_with(context, boxedValue));
}

TUUVM_API tuuvm_tuple_t tuuvm_type_createDependentFunctionType(tuuvm_context_t *context, tuuvm_tuple_t argumentNodes, bool isVariadic, tuuvm_tuple_t resultTypeNode,
    tuuvm_tuple_t environment, tuuvm_tuple_t captureBindings, tuuvm_tuple_t argumentBindings, tuuvm_tuple_t localBindings)
{
    tuuvm_type_tuple_t *supertype = (tuuvm_type_tuple_t*)context->roots.functionType;

    tuuvm_dependentFunctionType_t* result = (tuuvm_dependentFunctionType_t*)tuuvm_context_allocatePointerTuple(context, context->roots.dependentFunctionTypeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_dependentFunctionType_t));
    result->super.supertype = (tuuvm_tuple_t)supertype;
    result->super.flags = supertype->flags;
    result->super.totalSlotCount = supertype->totalSlotCount;
    result->argumentNodes = argumentNodes;
    result->isVariadic = tuuvm_tuple_boolean_encode(isVariadic);
    result->resultTypeNode = resultTypeNode;

    result->environment = environment;
    result->captureBindings = captureBindings;
    result->argumentBindings = argumentBindings;
    result->localBindings = localBindings;
    return (tuuvm_tuple_t)result;
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

TUUVM_API void tuuvm_typeAndMetatype_setFlags(tuuvm_context_t *context, tuuvm_tuple_t type, size_t flags, size_t metatypeFlags)
{
    tuuvm_type_setFlags(context, type, flags);
    tuuvm_type_setFlags(context, tuuvm_tuple_getType(context, type), TUUVM_TYPE_FLAGS_METATYPE_REQUIRED_FLAGS | metatypeFlags);
}

TUUVM_API tuuvm_tuple_t tuuvm_type_lookupMacroSelector(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t selector)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    tuuvm_tuple_t methodDictionary = tuuvm_type_getMacroMethodDictionary(type);
    if(methodDictionary)
    {
        tuuvm_tuple_t found = TUUVM_NULL_TUPLE;
        if(tuuvm_methodDictionary_find(methodDictionary, selector, &found))
            return found;
    }

    return tuuvm_type_lookupMacroSelector(context, tuuvm_type_getSupertype(type), selector);
}

static inline size_t computeLookupCacheEntryIndexFor(tuuvm_tuple_t type, tuuvm_tuple_t selector)
{
    return tuuvm_hashConcatenate(tuuvm_tuple_identityHash(type), tuuvm_tuple_identityHash(selector)) % GLOBAL_LOOKUP_CACHE_ENTRY_COUNT;
}

static tuuvm_tuple_t tuuvm_type_lookupSelectorRecursively(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t selector)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    tuuvm_tuple_t methodDictionary = tuuvm_type_getMethodDictionary(type);
    if(methodDictionary)
    {
        tuuvm_tuple_t found = TUUVM_NULL_TUPLE;
        if(tuuvm_methodDictionary_find(methodDictionary, selector, &found))
            return found;
    }

    return tuuvm_type_lookupSelectorRecursively(context, tuuvm_type_getSupertype(type), selector);
}

TUUVM_API tuuvm_tuple_t tuuvm_type_lookupSelector(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t selector)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;

    // FIXME: Make this cache atomic and thread safe.
    size_t cacheEntryIndex = computeLookupCacheEntryIndexFor(type, selector);
    tuuvm_globalLookupCacheEntry_t *cacheEntry = context->roots.globalMethodLookupCache + cacheEntryIndex;
    if(cacheEntry->type != type || cacheEntry->selector != selector)
    {
        tuuvm_tuple_t method = tuuvm_type_lookupSelectorRecursively(context, type, selector);
        if(method)
        {
            cacheEntry->type = type;
            cacheEntry->selector = selector;
            cacheEntry->method = method;
        }
        return method;
    }
    else
    {
        return cacheEntry->method;
    }
}

TUUVM_API tuuvm_tuple_t tuuvm_type_lookupFallbackSelector(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t selector)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;
    tuuvm_tuple_t methodDictionary = tuuvm_type_getFallbackMethodDictionary(type);
    if(methodDictionary)
    {
        tuuvm_tuple_t found = TUUVM_NULL_TUPLE;
        if(tuuvm_methodDictionary_find(methodDictionary, selector, &found))
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
        typeObject->methodDictionary = tuuvm_methodDictionary_create(context);
    tuuvm_methodDictionary_atPut(context, typeObject->methodDictionary, selector, method);

    if(tuuvm_tuple_isKindOf(context, method, context->roots.functionType))
    {
        tuuvm_function_t *functionObject = (tuuvm_function_t*)method;
        if(!functionObject->owner && !functionObject->name)
        {
            functionObject->owner = type;
            functionObject->name = selector;
        }
    }
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

TUUVM_API tuuvm_tuple_t tuuvm_type_getTypeCheckFunctionApplicationWithEnvironmentNode(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    return tuuvm_type_lookupSelector(context, type, context->roots.typeCheckFunctionApplicationWithEnvironmentSelector);
}

TUUVM_API void tuuvm_type_setTypeCheckFunctionApplicationWithEnvironmentNode(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t typeCheckFunction)
{
    tuuvm_type_setMethodWithSelector(context, type, context->roots.typeCheckFunctionApplicationWithEnvironmentSelector, typeCheckFunction);
}

TUUVM_API tuuvm_tuple_t tuuvm_pointerLikeType_load(tuuvm_context_t *context, tuuvm_tuple_t pointerLikeValue)
{
    if(!pointerLikeValue)
        tuuvm_error("Accessing null pointer-");
    if(!tuuvm_type_isPointerLikeType(tuuvm_tuple_getType(context, pointerLikeValue)))
        tuuvm_error("Expected a pointer like value.");

    size_t slotCount = tuuvm_tuple_getSizeInSlots(pointerLikeValue);
    if(slotCount == 0)
        tuuvm_error("Expected a pointer like value with at least single slot.");

    tuuvm_object_tuple_t *pointerObject = (tuuvm_object_tuple_t*)pointerLikeValue;
    tuuvm_tuple_t storage = pointerObject->pointers[0];
    intptr_t offset = tuuvm_tuple_intptr_encode(context, 0);
    if(slotCount >= 2)
        offset = tuuvm_tuple_intptr_decode(pointerObject->pointers[1]);

    tuuvm_pointerLikeType_t *pointerLikeType = (tuuvm_pointerLikeType_t*)tuuvm_tuple_getType(context, pointerLikeValue);
    return tuuvm_tuple_send2(context, context->roots.loadAtOffsetWithTypeSelector, storage, offset, pointerLikeType->baseType);
}

TUUVM_API tuuvm_tuple_t tuuvm_type_coerceValue(tuuvm_context_t *context, tuuvm_tuple_t type_, tuuvm_tuple_t value_)
{
    struct {
        tuuvm_tuple_t type;
        tuuvm_tuple_t value;
        tuuvm_tuple_t valueType;
        tuuvm_tuple_t function;
    } gcFrame = {
        .type = type_,
        .value = value_
    };
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.valueType = tuuvm_tuple_getType(context, gcFrame.value);
    if(!gcFrame.type)
    {
        if(tuuvm_type_isReferenceType(gcFrame.valueType))
        {
            TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
            return tuuvm_pointerLikeType_load(context, gcFrame.value);
        }
        
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return gcFrame.value;
    }

    // Load references.
    if(tuuvm_type_isReferenceType(gcFrame.valueType) && !tuuvm_type_isSubtypeOf(gcFrame.valueType, gcFrame.type))
    {
        gcFrame.value = tuuvm_pointerLikeType_load(context, gcFrame.value);
        gcFrame.valueType = tuuvm_tuple_getType(context, gcFrame.value);
    }

    if(!gcFrame.value && tuuvm_type_isNullable(gcFrame.type))
    {
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return gcFrame.value;
    }

    if(tuuvm_type_isSubtypeOf(gcFrame.valueType, gcFrame.type))
    {
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return gcFrame.value;
    }

    tuuvm_tuple_t function = tuuvm_type_getCoerceValueFunction(context, tuuvm_tuple_getType(context, gcFrame.type));
    if(function)
    {
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return tuuvm_function_apply2(context, function, gcFrame.type, gcFrame.value);
    }

    if(!gcFrame.valueType && tuuvm_tuple_isDummyValue(gcFrame.value))
    {
        tuuvm_tuple_t coercedDummyValue = (tuuvm_tuple_t)tuuvm_context_allocatePointerTuple(context, gcFrame.type, 0);
        tuuvm_tuple_markDummyValue(coercedDummyValue);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return coercedDummyValue;
    }
    
    tuuvm_error("Cannot perform coercion of value into the required type.");
    return TUUVM_VOID_TUPLE;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_coerceValuePassingReferences(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t value)
{
    if(type)
        return value;
    return tuuvm_type_coerceValue(context, type, value);
}
TUUVM_API tuuvm_tuple_t tuuvm_type_typeCheckFunctionApplicationNode(tuuvm_context_t *context, tuuvm_tuple_t functionType, tuuvm_tuple_t functionApplicationNode, tuuvm_tuple_t environment)
{
    if(!functionType)
        return functionApplicationNode;

    tuuvm_tuple_t function = tuuvm_type_getTypeCheckFunctionApplicationWithEnvironmentNode(context, tuuvm_tuple_getType(context, functionType));
    if(function)
        return tuuvm_function_apply3(context, function, functionType, functionApplicationNode, environment);

    return functionApplicationNode;
}

static tuuvm_tuple_t tuuvm_type_primitive_flushLookupSelector(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *type = &arguments[0];
    tuuvm_tuple_t *selector = &arguments[1];

    size_t cacheEntryIndex = computeLookupCacheEntryIndexFor(*type, *selector);
    tuuvm_globalLookupCacheEntry_t *cacheEntry = context->roots.globalMethodLookupCache + cacheEntryIndex;
    memset(cacheEntry, 0, sizeof(tuuvm_globalLookupCacheEntry_t));

    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_type_primitive_flushMacroLookupSelector(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_type_primitive_flushFallbackLookupSelector(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_type_primitive_coerceValue(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return tuuvm_type_coerceValue(context, arguments[1], arguments[0]);
}

static tuuvm_tuple_t tuuvm_type_primitive_createSimpleFunctionType(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 3) tuuvm_error_argumentCountMismatch(3, argumentCount);

    return tuuvm_type_doCreateSimpleFunctionType(context, arguments[0], arguments[1], arguments[2]);
}

static tuuvm_tuple_t tuuvm_type_primitive_createPointerType(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return tuuvm_type_doCreatePointerType(context, arguments[0], arguments[1]);
}

static tuuvm_tuple_t tuuvm_type_primitive_createReferenceType(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return tuuvm_type_doCreateReferenceType(context, arguments[0], arguments[1]);
}

static tuuvm_tuple_t tuuvm_type_primitive_pointerLikeLoad(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_pointerLikeType_load(context, arguments[0]);
}

static tuuvm_tuple_t tuuvm_type_primitive_pointerLikeStore(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(1, argumentCount);

    tuuvm_tuple_t *pointerLikeValue = &arguments[0];
    tuuvm_tuple_t *valueToStore = &arguments[1];
    if(!pointerLikeValue) tuuvm_error("Accessing null pointer.");
    if(!tuuvm_type_isPointerLikeType(tuuvm_tuple_getType(context, *pointerLikeValue)))
        tuuvm_error("Expected a pointer like value.");

    size_t slotCount = tuuvm_tuple_getSizeInSlots(*pointerLikeValue);
    if(slotCount == 0) tuuvm_error("Expected a pointer like value with at least single slot.");

    struct {
        tuuvm_pointerLikeType_t *pointerLikeType;
        tuuvm_tuple_t coercedValue;
        tuuvm_tuple_t storage;
        tuuvm_tuple_t offset;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.pointerLikeType = (tuuvm_pointerLikeType_t*)tuuvm_tuple_getType(context, *pointerLikeValue);
    gcFrame.coercedValue = tuuvm_type_coerceValue(context, gcFrame.pointerLikeType->baseType, *valueToStore);

    tuuvm_object_tuple_t **pointerObject = (tuuvm_object_tuple_t**)pointerLikeValue;
    gcFrame.storage = (*pointerObject)->pointers[0];
    gcFrame.offset = tuuvm_tuple_intptr_encode(context, 0);
    if(slotCount >= 2)
        gcFrame.offset = (*pointerObject)->pointers[1];

    tuuvm_tuple_send3(context, context->roots.storeAtOffsetWithTypeSelector, gcFrame.storage, *valueToStore, gcFrame.offset, gcFrame.pointerLikeType->baseType);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return *pointerLikeValue;
}

void tuuvm_type_registerPrimitives(void)
{
    tuuvm_primitiveTable_registerFunction(tuuvm_type_primitive_createSimpleFunctionType, "SimpleFunctionTypeTemplate");
    tuuvm_primitiveTable_registerFunction(tuuvm_type_primitive_createPointerType, "PointerTypeTemplate");
    tuuvm_primitiveTable_registerFunction(tuuvm_type_primitive_createReferenceType, "ReferenceTypeTemplate");

    tuuvm_primitiveTable_registerFunction(tuuvm_type_primitive_pointerLikeLoad, "PointerLikeType::load");
    tuuvm_primitiveTable_registerFunction(tuuvm_type_primitive_pointerLikeStore, "PointerLikeType::store:");

    tuuvm_primitiveTable_registerFunction(tuuvm_type_primitive_coerceValue, "Type::coerceValue:into:");

    tuuvm_primitiveTable_registerFunction(tuuvm_type_primitive_flushLookupSelector, "Type::flushLookupSelector:");
    tuuvm_primitiveTable_registerFunction(tuuvm_type_primitive_flushMacroLookupSelector, "Type::flushMacroLookupSelector:");
    tuuvm_primitiveTable_registerFunction(tuuvm_type_primitive_flushFallbackLookupSelector, "Type::flushFallbackLookupSelector:");
}

void tuuvm_type_setupPrimitives(tuuvm_context_t *context)
{
    context->roots.simpleFunctionTypeTemplate = tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "SimpleFunctionTypeTemplate", 3, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_MEMOIZED | TUUVM_FUNCTION_FLAGS_TEMPLATE, NULL, tuuvm_type_primitive_createSimpleFunctionType);
    context->roots.pointerTypeTemplate = tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "PointerTypeTemplate", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_MEMOIZED | TUUVM_FUNCTION_FLAGS_TEMPLATE, NULL, tuuvm_type_primitive_createPointerType);
    context->roots.referenceTypeTemplate = tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "ReferenceTypeTemplate", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_MEMOIZED | TUUVM_FUNCTION_FLAGS_TEMPLATE, NULL, tuuvm_type_primitive_createReferenceType);
    context->roots.pointerLikeLoadPrimitive = tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "PointerLikeType::load", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_type_primitive_pointerLikeLoad);
    context->roots.pointerLikeStorePrimitive = tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "PointerLikeType::store:", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_type_primitive_pointerLikeStore);

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "Type::coerceValue:into:", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_type_primitive_coerceValue);

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Type::flushLookupSelector:", context->roots.typeType, "flushLookupSelector:", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_type_primitive_flushLookupSelector);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Type::flushMacroLookupSelector:", context->roots.typeType, "flushMacroLookupSelector:", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_type_primitive_flushMacroLookupSelector);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Type::flushFallbackLookupSelector:", context->roots.typeType, "flushFallbackLookupSelector:", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_type_primitive_flushFallbackLookupSelector);
}