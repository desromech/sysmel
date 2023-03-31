#include "tuuvm/type.h"
#include "tuuvm/array.h"
#include "tuuvm/assert.h"
#include "tuuvm/ast.h"
#include "tuuvm/dictionary.h"
#include "tuuvm/errors.h"
#include "tuuvm/environment.h"
#include "tuuvm/function.h"
#include "tuuvm/stackFrame.h"
#include "tuuvm/string.h"
#include "tuuvm/type.h"
#include "internal/context.h"
#include <string.h>

TUUVM_API tuuvm_tuple_t tuuvm_typeSlot_create(tuuvm_context_t *context, tuuvm_tuple_t name, tuuvm_tuple_t flags, tuuvm_tuple_t type, size_t localIndex, size_t index)
{
    tuuvm_typeSlot_t* result = (tuuvm_typeSlot_t*)tuuvm_context_allocatePointerTuple(context, context->roots.typeSlotType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_typeSlot_t));
    result->flags = flags;
    result->name = name;
    result->type = type;
    result->localIndex = tuuvm_tuple_size_encode(context, localIndex);
    result->index = tuuvm_tuple_size_encode(context, index);
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_createAnonymous(tuuvm_context_t *context)
{
    tuuvm_type_tuple_t* result = (tuuvm_type_tuple_t*)tuuvm_context_allocatePointerTuple(context, context->roots.typeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_type_tuple_t));
    result->supertype = context->roots.anyValueType;
    result->totalSlotCount = tuuvm_tuple_size_encode(context, 0);
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_createAnonymousMetatype(tuuvm_context_t *context)
{
    tuuvm_metatype_t* result = (tuuvm_metatype_t*)tuuvm_context_allocatePointerTuple(context, context->roots.metatypeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_metatype_t));
    result->super.flags = tuuvm_tuple_bitflags_encode(TUUVM_TYPE_FLAGS_METATYPE_REQUIRED_FLAGS);

    size_t slotCount = TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_type_tuple_t);
    
    result->super.totalSlotCount = tuuvm_tuple_size_encode(context, slotCount);
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_createAnonymousAndMetatype(tuuvm_context_t *context)
{
    tuuvm_tuple_t metatype = tuuvm_type_createAnonymousMetatype(context);
    tuuvm_type_tuple_t* result = (tuuvm_type_tuple_t*)tuuvm_context_allocatePointerTuple(context, metatype, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_type_tuple_t));
    result->supertype = context->roots.anyValueType;
    result->totalSlotCount = tuuvm_tuple_size_encode(context, 0);
    ((tuuvm_metatype_t*)metatype)->thisType = (tuuvm_tuple_t)result;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_createAnonymousClass(tuuvm_context_t *context, tuuvm_tuple_t supertype, tuuvm_tuple_t metaclass)
{
    size_t classSlotCount = tuuvm_type_getTotalSlotCount(metaclass);
    TUUVM_ASSERT(classSlotCount >= TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_class_t));

    tuuvm_class_t* result = (tuuvm_class_t*)tuuvm_context_allocatePointerTuple(context, metaclass, classSlotCount);
    result->super.supertype = supertype;
    result->super.totalSlotCount = tuuvm_tuple_size_encode(context, 0);
    result->super.flags = tuuvm_tuple_bitflags_encode(TUUVM_TYPE_FLAGS_CLASS_DEFAULT_FLAGS);
    if(supertype)
        result->super.totalSlotCount = tuuvm_tuple_size_encode(context, tuuvm_type_getTotalSlotCount(supertype));
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_createAnonymousMetaclass(tuuvm_context_t *context, tuuvm_tuple_t supertype)
{
    tuuvm_metaclass_t* result = (tuuvm_metaclass_t*)tuuvm_context_allocatePointerTuple(context, context->roots.metaclassType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_metaclass_t));
    result->super.super.supertype = supertype;
    result->super.super.flags = tuuvm_tuple_bitflags_encode(TUUVM_TYPE_FLAGS_METATYPE_REQUIRED_FLAGS);

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
    else if(tuuvm_type_isDirectSubtypeOf(actualSuperType, context->roots.typeType))
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
    result->super.super.flags = tuuvm_tuple_bitflags_encode(TUUVM_TYPE_FLAGS_PRIMITIVE_VALUE_TYPE_DEFAULT_FLAGS);
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_createAnonymousValueMetatype(tuuvm_context_t *context, tuuvm_tuple_t supertype, size_t minimumSlotCount)
{
    tuuvm_valueMetatype_t* result = (tuuvm_valueMetatype_t*)tuuvm_context_allocatePointerTuple(context, context->roots.valueMetatypeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_valueMetatype_t));
    result->super.super.supertype = supertype;
    result->super.super.flags = tuuvm_tuple_bitflags_encode(TUUVM_TYPE_FLAGS_METATYPE_REQUIRED_FLAGS);

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
    tuuvm_tuple_t supertypeMetatype = tuuvm_tuple_getType(context, supertype);
    if(tuuvm_type_isDirectSubtypeOf(supertypeMetatype, context->roots.primitiveValueType))
        metatypeSupertype = supertypeMetatype;

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
    result->super.flags = tuuvm_tuple_bitflags_encode(tuuvm_tuple_bitflags_decode(supertype->flags) | TUUVM_TYPE_FLAGS_FUNCTION);
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

TUUVM_API tuuvm_tuple_t tuuvm_type_createSimpleFunctionTypeWithArguments0(tuuvm_context_t *context, tuuvm_tuple_t resultType)
{
    struct {
        tuuvm_tuple_t arguments;
        tuuvm_tuple_t result;
    } gcFrame = {};

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.arguments = tuuvm_array_create(context, 0);
    gcFrame.result = tuuvm_type_createSimpleFunctionType(context, gcFrame.arguments, false, resultType);

    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_createSimpleFunctionTypeWithArguments1(tuuvm_context_t *context, tuuvm_tuple_t argument, tuuvm_tuple_t resultType)
{
    struct {
        tuuvm_tuple_t arguments;
        tuuvm_tuple_t result;
    } gcFrame = {};

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.arguments = tuuvm_array_create(context, 1);
    tuuvm_array_atPut(gcFrame.arguments, 0, argument);

    gcFrame.result = tuuvm_type_createSimpleFunctionType(context, gcFrame.arguments, false, resultType);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_createSimpleFunctionTypeWithArguments2(tuuvm_context_t *context, tuuvm_tuple_t argument0, tuuvm_tuple_t argument1, tuuvm_tuple_t resultType)
{
    struct {
        tuuvm_tuple_t arguments;
        tuuvm_tuple_t result;
    } gcFrame = {};

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.arguments = tuuvm_array_create(context, 2);
    tuuvm_array_atPut(gcFrame.arguments, 0, argument0);
    tuuvm_array_atPut(gcFrame.arguments, 1, argument1);

    gcFrame.result = tuuvm_type_createSimpleFunctionType(context, gcFrame.arguments, false, resultType);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_pointerLikeType_createPointerLikeLoadFunction(tuuvm_context_t *context, tuuvm_tuple_t pointerLikeType_, tuuvm_tuple_t baseType_, tuuvm_bitflags_t extraFlags)
{
    struct {
        tuuvm_tuple_t pointerLikeType;
        tuuvm_tuple_t baseType;
        tuuvm_tuple_t function;
        tuuvm_tuple_t functionType;
    } gcFrame = {
        .pointerLikeType = pointerLikeType_,
        .baseType = baseType_,
    };
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.function = tuuvm_context_shallowCopy(context, context->roots.pointerLikeLoadPrimitive);
    tuuvm_function_addFlags(context, gcFrame.function, extraFlags);

    gcFrame.functionType = tuuvm_type_createSimpleFunctionTypeWithArguments1(context, gcFrame.pointerLikeType, gcFrame.baseType);
    tuuvm_tuple_setType((tuuvm_object_tuple_t*)gcFrame.function, gcFrame.functionType);

    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.function;
}

static tuuvm_tuple_t tuuvm_pointerLikeType_createPointerLikeStoreFunction(tuuvm_context_t *context, tuuvm_tuple_t pointerLikeType_, tuuvm_tuple_t baseType_, tuuvm_bitflags_t extraFlags)
{
    struct {
        tuuvm_tuple_t pointerLikeType;
        tuuvm_tuple_t baseType;
        tuuvm_tuple_t function;
        tuuvm_tuple_t functionType;
    } gcFrame = {
        .pointerLikeType = pointerLikeType_,
        .baseType = baseType_,
    };
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.function = tuuvm_context_shallowCopy(context, context->roots.pointerLikeStorePrimitive);
    tuuvm_function_addFlags(context, gcFrame.function, extraFlags);

    gcFrame.functionType = tuuvm_type_createSimpleFunctionTypeWithArguments2(context, gcFrame.pointerLikeType, gcFrame.baseType, gcFrame.pointerLikeType);
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
    gcFrame.result->super.super.super.flags = tuuvm_tuple_bitflags_encode(TUUVM_TYPE_FLAGS_POINTER_TYPE_FLAGS);
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
    gcFrame.result->super.super.super.flags = tuuvm_tuple_bitflags_encode(TUUVM_TYPE_FLAGS_REFERENCE_TYPE_FLAGS);
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

TUUVM_API tuuvm_tuple_t tuuvm_pointerLikeType_withStorage(tuuvm_context_t *context, tuuvm_tuple_t pointerLikeType, tuuvm_tuple_t storage)
{
    tuuvm_object_tuple_t *reference = tuuvm_context_allocatePointerTuple(context, pointerLikeType, 1);
    reference->pointers[0] = storage;
    return (tuuvm_tuple_t)reference;
}

TUUVM_API tuuvm_tuple_t tuuvm_pointerLikeType_withBoxForValue(tuuvm_context_t *context, tuuvm_tuple_t pointerLikeType, tuuvm_tuple_t boxedValue)
{
    return tuuvm_pointerLikeType_withStorage(context, pointerLikeType, tuuvm_valueBox_with(context, boxedValue));
}

TUUVM_API tuuvm_tuple_t tuuvm_pointerLikeType_withEmptyBox(tuuvm_context_t *context, tuuvm_tuple_t pointerLikeType)
{
    return tuuvm_pointerLikeType_withBoxForValue(context, pointerLikeType, TUUVM_NULL_TUPLE);
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
    result->super.flags = tuuvm_tuple_bitflags_encode(tuuvm_tuple_bitflags_decode(supertype->flags) | TUUVM_TYPE_FLAGS_FUNCTION);
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

TUUVM_API void tuuvm_type_setFlags(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_bitflags_t flags)
{
    (void)context;
    if(!tuuvm_tuple_isNonNullPointer(type)) return;
    ((tuuvm_type_tuple_t*)type)->flags = tuuvm_tuple_bitflags_encode(flags);
}

TUUVM_API void tuuvm_typeAndMetatype_setFlags(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_bitflags_t flags, tuuvm_bitflags_t metatypeFlags)
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

TUUVM_API tuuvm_tuple_t tuuvm_type_lookupSlot(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t slotName)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return TUUVM_NULL_TUPLE;

    tuuvm_tuple_t slotDictionary = tuuvm_type_getSlotDictionary(type);
    if(slotDictionary)
    {
        tuuvm_tuple_t found = TUUVM_NULL_TUPLE;
        if(tuuvm_methodDictionary_find(slotDictionary, slotName, &found))
            return found;
    }

    return tuuvm_type_lookupSlot(context, tuuvm_type_getSupertype(type), slotName);
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

TUUVM_API tuuvm_tuple_t tuuvm_type_lookupSelectorWithInlineCache(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t selector, tuuvm_inlineLookupCacheEntry_t *inlineCache)
{
    // FIXME: Make this thread safe
    if(inlineCache->receiverType == type)
        return inlineCache->method;
    
    inlineCache->receiverType = type;
    return inlineCache->method = tuuvm_type_lookupSelector(context, type, selector);
}

TUUVM_API tuuvm_tuple_t tuuvm_type_lookupSelectorWithPIC(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t selector, tuuvm_polymorphicInlineLookupCache_t *pic)
{
    size_t expectedEntryIndex = tuuvm_tuple_identityHash(type) % PIC_ENTRY_COUNT;
    tuuvm_inlineLookupCacheEntry_t *entry = pic->entries + expectedEntryIndex;
    if(entry->receiverType == type)
        return entry->method;
    
    entry->receiverType = type;
    return entry->method = tuuvm_type_lookupSelector(context, type, selector);
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

    if(tuuvm_tuple_isFunction(context, method))
    {
        tuuvm_function_t *functionObject = (tuuvm_function_t*)method;
        if(!functionObject->owner && !functionObject->name)
        {
            functionObject->owner = type;
            functionObject->name = selector;
        }
    }
}

TUUVM_API void tuuvm_type_buildSlotDictionary(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    if(!tuuvm_tuple_isNonNullPointer(type)) return;

    tuuvm_type_tuple_t* typeObject = (tuuvm_type_tuple_t*)type;
    size_t slotCount = tuuvm_array_getSize(typeObject->slots);
    if(slotCount == 0) return;

    if(!typeObject->slotDictionary)
        typeObject->slotDictionary = tuuvm_methodDictionary_createWithCapacity(context, slotCount);

    for(size_t i = 0; i < slotCount; ++i)
    {
        tuuvm_typeSlot_t *typeSlot = (tuuvm_typeSlot_t*)tuuvm_array_at(typeObject->slots, i);
        if(typeSlot->name)
            tuuvm_methodDictionary_atPut(context, typeObject->slotDictionary, typeSlot->name, (tuuvm_tuple_t)typeSlot);
    }
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
    return tuuvm_type_lookupSelectorWithPIC(context, type, context->roots.astNodeAnalysisSelector, &context->roots.inlineCaches.analyzeASTWithEnvironment);
}

TUUVM_API void tuuvm_type_setAstNodeAnalysisFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t astNodeAnalysisFunction)
{
    tuuvm_type_setMethodWithSelector(context, type, context->roots.astNodeAnalysisSelector, astNodeAnalysisFunction);
}

TUUVM_API tuuvm_tuple_t tuuvm_type_getAstNodeEvaluationFunction(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    return tuuvm_type_lookupSelectorWithPIC(context, type, context->roots.astNodeEvaluationSelector, &context->roots.inlineCaches.evaluateASTWithEnvironment);
}

TUUVM_API void tuuvm_type_setAstNodeEvaluationFunction(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t astNodeEvaluationFunction)
{
    tuuvm_type_setMethodWithSelector(context, type, context->roots.astNodeEvaluationSelector, astNodeEvaluationFunction);
}

TUUVM_API tuuvm_tuple_t tuuvm_type_getAstNodeAnalysisAndEvaluationFunction(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    return tuuvm_type_lookupSelectorWithPIC(context, type, context->roots.astNodeAnalysisAndEvaluationSelector, &context->roots.inlineCaches.evaluateAndAnalyzeASTWithEnvironment);
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

TUUVM_API tuuvm_tuple_t tuuvm_type_getAnalyzeAndTypeCheckFunctionApplicationNodeWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    return tuuvm_type_lookupSelector(context, type, context->roots.analyzeAndTypeCheckFunctionApplicationNodeWithEnvironmentSelector);
}

TUUVM_API void tuuvm_type_setAnalyzeAndTypeCheckFunctionApplicationNodeWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t typeCheckFunction)
{
    tuuvm_type_setMethodWithSelector(context, type, context->roots.analyzeAndTypeCheckFunctionApplicationNodeWithEnvironmentSelector, typeCheckFunction);
}

TUUVM_API tuuvm_tuple_t tuuvm_type_getAnalyzeAndTypeCheckMessageSendNodeWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    return tuuvm_type_lookupSelector(context, type, context->roots.analyzeAndTypeCheckMessageSendNodeWithEnvironmentSelector);
}

TUUVM_API void tuuvm_type_setAnalyzeAndTypeCheckMessageSendNodeWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t typeCheckFunction)
{
    tuuvm_type_setMethodWithSelector(context, type, context->roots.analyzeAndTypeCheckMessageSendNodeWithEnvironmentSelector, typeCheckFunction);
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

    if(tuuvm_tuple_getType(context, storage) == context->roots.valueBoxType)
        return TUUVM_CAST_OOP_TO_OBJECT_TUPLE(storage)->pointers[0];

    tuuvm_pointerLikeType_t *pointerLikeType = (tuuvm_pointerLikeType_t*)tuuvm_tuple_getType(context, pointerLikeValue);
    return tuuvm_tuple_send2(context, context->roots.loadAtOffsetWithTypeSelector, storage, offset, pointerLikeType->baseType);
}

TUUVM_API tuuvm_tuple_t tuuvm_pointerLikeType_store(tuuvm_context_t *context, tuuvm_tuple_t pointerLikeValue_, tuuvm_tuple_t valueToStore_)
{
    struct {
        tuuvm_tuple_t pointerLikeValue;
        tuuvm_tuple_t valueToStore;

        tuuvm_pointerLikeType_t *pointerLikeType;
        tuuvm_tuple_t coercedValue;
        tuuvm_tuple_t storage;
        tuuvm_tuple_t offset;
    } gcFrame = {
        .pointerLikeValue = pointerLikeValue_,
        .valueToStore = valueToStore_
    };
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    if(!gcFrame.pointerLikeValue) tuuvm_error("Accessing null pointer.");
    if(!tuuvm_type_isPointerLikeType(tuuvm_tuple_getType(context, gcFrame.pointerLikeValue)))
        tuuvm_error("Expected a pointer like value.");

    size_t slotCount = tuuvm_tuple_getSizeInSlots(gcFrame.pointerLikeValue);
    if(slotCount == 0) tuuvm_error("Expected a pointer like value with at least single slot.");

    gcFrame.pointerLikeType = (tuuvm_pointerLikeType_t*)tuuvm_tuple_getType(context, gcFrame.pointerLikeValue);
    gcFrame.coercedValue = tuuvm_type_coerceValue(context, gcFrame.pointerLikeType->baseType, gcFrame.valueToStore);

    tuuvm_object_tuple_t **pointerObject = (tuuvm_object_tuple_t**)&gcFrame.pointerLikeValue;
    gcFrame.storage = (*pointerObject)->pointers[0];
    gcFrame.offset = tuuvm_tuple_intptr_encode(context, 0);
    if(slotCount >= 2)
        gcFrame.offset = (*pointerObject)->pointers[1];
    else
    {
        if(tuuvm_tuple_getType(context, gcFrame.storage) == context->roots.valueBoxType)
        {
            TUUVM_CAST_OOP_TO_OBJECT_TUPLE(gcFrame.storage)->pointers[0] = gcFrame.coercedValue;
            TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
            return gcFrame.pointerLikeValue;
        }
    }


    tuuvm_tuple_send3(context, context->roots.storeAtOffsetWithTypeSelector, gcFrame.storage, gcFrame.valueToStore, gcFrame.offset, gcFrame.pointerLikeType->baseType);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.pointerLikeValue;
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
    if(tuuvm_type_isReferenceType(gcFrame.valueType) && !tuuvm_type_isDirectSubtypeOf(gcFrame.valueType, gcFrame.type))
    {
        gcFrame.value = tuuvm_pointerLikeType_load(context, gcFrame.value);
        gcFrame.valueType = tuuvm_tuple_getType(context, gcFrame.value);
    }

    if(!gcFrame.value && tuuvm_type_isNullable(gcFrame.type))
    {
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return gcFrame.value;
    }

    if(tuuvm_type_isDirectSubtypeOf(gcFrame.valueType, gcFrame.type))
    {
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return gcFrame.value;
    }

    gcFrame.function = tuuvm_type_getCoerceValueFunction(context, tuuvm_tuple_getType(context, gcFrame.type));
    if(gcFrame.function)
    {
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return tuuvm_function_applyNoCheck2(context, gcFrame.function, gcFrame.type, gcFrame.value);
    }

    if(!gcFrame.valueType && tuuvm_tuple_isDummyValue(gcFrame.value))
    {
        tuuvm_tuple_t coercedDummyValue = (tuuvm_tuple_t)tuuvm_context_allocatePointerTuple(context, gcFrame.type, 0);
        tuuvm_tuple_markDummyValue(coercedDummyValue);
        TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return coercedDummyValue;
    }
    
    tuuvm_tuple_t typeString = tuuvm_tuple_printString(context, gcFrame.type);
    fprintf(stderr, "Cannot perform coercion of value into the required type: " TUUVM_STRING_PRINTF_FORMAT "\n", TUUVM_STRING_PRINTF_ARG(typeString));
    tuuvm_error("Cannot perform coercion of value into the required type.");
    return TUUVM_VOID_TUPLE;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_coerceValuePassingReferences(tuuvm_context_t *context, tuuvm_tuple_t type, tuuvm_tuple_t value)
{
    if(type)
        return value;
    return tuuvm_type_coerceValue(context, type, value);
}

TUUVM_API tuuvm_tuple_t tuuvm_type_decay(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    if(tuuvm_tuple_isKindOf(context, type, context->roots.referenceType))
        return ((tuuvm_referenceType_t*)type)->super.baseType;

    return type;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_getCanonicalPendingInstanceType(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    if(tuuvm_type_isDirectSubtypeOf(type, context->roots.referenceType))
        return tuuvm_type_createReferenceType(context, context->roots.anyValueType, TUUVM_NULL_TUPLE);
    if(tuuvm_type_isDirectSubtypeOf(type, context->roots.metatypeType))
    {
        tuuvm_tuple_t thisType = ((tuuvm_metatype_t*)type)->thisType;
        if(thisType)
            return thisType;
    }
    
    return context->roots.anyValueType;
}

TUUVM_API tuuvm_tuple_t tuuvm_type_getDefaultValue(tuuvm_context_t *context, tuuvm_tuple_t type)
{
    if(tuuvm_type_isNullable(type))
        return TUUVM_NULL_TUPLE;
    
    return tuuvm_tuple_send0(context, context->roots.defaultValueSelector, type);
}

static tuuvm_tuple_t tuuvm_type_primitive_flushLookupSelector(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    //tuuvm_tuple_t *type = &arguments[0];
    tuuvm_tuple_t *selector = &arguments[1];

    for(size_t i = 0; i < GLOBAL_LOOKUP_CACHE_ENTRY_COUNT; ++i)
    {
        tuuvm_globalLookupCacheEntry_t *cacheEntry = context->roots.globalMethodLookupCache + i;
        if(cacheEntry->selector == *selector)
            memset(cacheEntry, 0, sizeof(tuuvm_globalLookupCacheEntry_t));
    }
    memset(&context->roots.inlineCaches, 0, sizeof(context->roots.inlineCaches));

    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_type_primitive_flushMacroLookupSelector(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_type_primitive_flushFallbackLookupSelector(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_type_primitive_coerceValue(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return tuuvm_type_coerceValue(context, arguments[1], arguments[0]);
}

static tuuvm_tuple_t tuuvm_type_primitive_createSimpleFunctionType(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 3) tuuvm_error_argumentCountMismatch(3, argumentCount);

    return tuuvm_type_doCreateSimpleFunctionType(context, arguments[0], arguments[1], arguments[2]);
}

static tuuvm_tuple_t tuuvm_type_primitive_createPointerType(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return tuuvm_type_doCreatePointerType(context, arguments[0], arguments[1]);
}

static tuuvm_tuple_t tuuvm_type_primitive_createReferenceType(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return tuuvm_type_doCreateReferenceType(context, arguments[0], arguments[1]);
}

static tuuvm_tuple_t tuuvm_type_primitive_pointerLikeLoad(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_pointerLikeType_load(context, arguments[0]);
}

static tuuvm_tuple_t tuuvm_type_primitive_pointerLikeStore(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_pointerLikeType_store(context, arguments[0], arguments[1]);
}

static tuuvm_tuple_t tuuvm_void_primitive_anyValueToVoid(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)argumentCount;
    (void)arguments;
    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_type_primitive_coerceASTNodeWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) tuuvm_error_argumentCountMismatch(3, argumentCount);

    tuuvm_tuple_t *targetType = &arguments[0];
    tuuvm_tuple_t *astNode = &arguments[1];
    tuuvm_tuple_t *environment = &arguments[2];

    (void)targetType;
    (void)environment;

    tuuvm_tuple_t sourceType = tuuvm_astNode_getAnalyzedType(*astNode);
    if(sourceType && !tuuvm_type_isDirectSubtypeOf(sourceType, context->roots.controlFlowEscapeType))
    {
        if(tuuvm_type_isDynamic(sourceType))
        {
            // Add a downcast for the untyped case.
            return tuuvm_astDownCastNode_addOntoNodeWithTargetType(context, *astNode, *targetType);
        }
        else
        {
            tuuvm_error("Cannot perform the type coercion.");
        }
    }

    return *astNode;
}

static tuuvm_tuple_t tuuvm_void_primitive_coerceASTNodeWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) tuuvm_error_argumentCountMismatch(3, argumentCount);

    tuuvm_tuple_t *targetType = &arguments[0];
    tuuvm_tuple_t *astNode = &arguments[1];
    tuuvm_tuple_t *environment = &arguments[2];

    if(*targetType != context->roots.voidType)
    {
        if(tuuvm_type_isDirectSubtypeOf(*targetType, context->roots.controlFlowEscapeType))
            return *astNode;

        tuuvm_error("Invalid type conversion requested.");
    }

    tuuvm_tuple_t sourcePosition = tuuvm_astNode_getSourcePosition(*astNode);
    tuuvm_tuple_t functionLiteral = tuuvm_astLiteralNode_create(context, sourcePosition, context->roots.anyValueToVoidPrimitive);

    tuuvm_tuple_t applicationArguments = tuuvm_array_create(context, 1);
    tuuvm_array_atPut(applicationArguments, 0, *astNode);        

    tuuvm_astNode_t *functionApplication = (tuuvm_astNode_t*)tuuvm_astFunctionApplicationNode_create(context, sourcePosition, functionLiteral, applicationArguments);
    functionApplication->analyzedType = *targetType;
    functionApplication->analyzerToken = tuuvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, *environment);
    return (tuuvm_tuple_t)functionApplication;
}

void tuuvm_type_registerPrimitives(void)
{
    tuuvm_primitiveTable_registerFunction(tuuvm_type_primitive_createSimpleFunctionType, "SimpleFunctionTypeTemplate");
    tuuvm_primitiveTable_registerFunction(tuuvm_type_primitive_createPointerType, "PointerTypeTemplate");
    tuuvm_primitiveTable_registerFunction(tuuvm_type_primitive_createReferenceType, "ReferenceTypeTemplate");

    tuuvm_primitiveTable_registerFunction(tuuvm_type_primitive_pointerLikeLoad, "PointerLikeType::load");
    tuuvm_primitiveTable_registerFunction(tuuvm_type_primitive_pointerLikeStore, "PointerLikeType::store:");
    
    tuuvm_primitiveTable_registerFunction(tuuvm_void_primitive_anyValueToVoid, "Void::fromAnyValue");
    tuuvm_primitiveTable_registerFunction(tuuvm_void_primitive_coerceASTNodeWithEnvironment, "Void::coerceASTNode:withEnvironment:");

    tuuvm_primitiveTable_registerFunction(tuuvm_type_primitive_coerceValue, "Type::coerceValue:into:");
    tuuvm_primitiveTable_registerFunction(tuuvm_type_primitive_coerceASTNodeWithEnvironment, "Type::coerceASTNode:withEnvironment:");

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
    
    context->roots.anyValueToVoidPrimitive = tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "Void::fromAnyValue", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_NO_TYPECHECK_ARGUMENTS, NULL, tuuvm_void_primitive_anyValueToVoid);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Void::coerceASTNode:withEnvironment:", tuuvm_tuple_getType(context, context->roots.voidType), "coerceASTNode:withEnvironment:", 3, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_void_primitive_coerceASTNodeWithEnvironment);
    
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "Type::coerceValue:into:", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_type_primitive_coerceValue);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Type::coerceASTNode:withEnvironment:", context->roots.typeType, "coerceASTNode:withEnvironment:", 3, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_type_primitive_coerceASTNodeWithEnvironment);

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Type::flushLookupSelector:", context->roots.typeType, "flushLookupSelector:", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_type_primitive_flushLookupSelector);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Type::flushMacroLookupSelector:", context->roots.typeType, "flushMacroLookupSelector:", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_type_primitive_flushMacroLookupSelector);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Type::flushFallbackLookupSelector:", context->roots.typeType, "flushFallbackLookupSelector:", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_type_primitive_flushFallbackLookupSelector);

    // Export the type layout. This is used by the bootstraping algorithm for creating the accessors.
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "Type::Layout::name", tuuvm_tuple_integer_encodeSmall(TUUVM_SLOT_INDEX_FOR_STRUCTURE_MEMBER(tuuvm_type_tuple_t, name)));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "Type::Layout::owner", tuuvm_tuple_integer_encodeSmall(TUUVM_SLOT_INDEX_FOR_STRUCTURE_MEMBER(tuuvm_type_tuple_t, owner)));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "Type::Layout::supertype", tuuvm_tuple_integer_encodeSmall(TUUVM_SLOT_INDEX_FOR_STRUCTURE_MEMBER(tuuvm_type_tuple_t, supertype)));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "Type::Layout::slots", tuuvm_tuple_integer_encodeSmall(TUUVM_SLOT_INDEX_FOR_STRUCTURE_MEMBER(tuuvm_type_tuple_t, slots)));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "Type::Layout::totalSlotCount", tuuvm_tuple_integer_encodeSmall(TUUVM_SLOT_INDEX_FOR_STRUCTURE_MEMBER(tuuvm_type_tuple_t, totalSlotCount)));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "Type::Layout::flags", tuuvm_tuple_integer_encodeSmall(TUUVM_SLOT_INDEX_FOR_STRUCTURE_MEMBER(tuuvm_type_tuple_t, flags)));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "Type::Layout::macroMethodDictionary", tuuvm_tuple_integer_encodeSmall(TUUVM_SLOT_INDEX_FOR_STRUCTURE_MEMBER(tuuvm_type_tuple_t, macroMethodDictionary)));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "Type::Layout::methodDictionary", tuuvm_tuple_integer_encodeSmall(TUUVM_SLOT_INDEX_FOR_STRUCTURE_MEMBER(tuuvm_type_tuple_t, methodDictionary)));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "Type::Layout::fallbackMethodDictionary", tuuvm_tuple_integer_encodeSmall(TUUVM_SLOT_INDEX_FOR_STRUCTURE_MEMBER(tuuvm_type_tuple_t, fallbackMethodDictionary)));

    // Export the type slot layout. This is used by the bootstraping algorithm for creating the accessors.
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeSlot::Layout::name", tuuvm_tuple_integer_encodeSmall(TUUVM_SLOT_INDEX_FOR_STRUCTURE_MEMBER(tuuvm_typeSlot_t, name)));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeSlot::Layout::flags", tuuvm_tuple_integer_encodeSmall(TUUVM_SLOT_INDEX_FOR_STRUCTURE_MEMBER(tuuvm_typeSlot_t, flags)));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeSlot::Layout::type", tuuvm_tuple_integer_encodeSmall(TUUVM_SLOT_INDEX_FOR_STRUCTURE_MEMBER(tuuvm_typeSlot_t, type)));

    // Export the type flags.
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::None", tuuvm_tuple_bitflags_encode(TUUVM_TYPE_FLAGS_NONE));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::Nullable", tuuvm_tuple_bitflags_encode(TUUVM_TYPE_FLAGS_NULLABLE));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::Bytes", tuuvm_tuple_bitflags_encode(TUUVM_TYPE_FLAGS_BYTES));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::Immediate", tuuvm_tuple_bitflags_encode(TUUVM_TYPE_FLAGS_IMMEDIATE));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::Weak", tuuvm_tuple_bitflags_encode(TUUVM_TYPE_FLAGS_WEAK));

    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::Final", tuuvm_tuple_bitflags_encode(TUUVM_TYPE_FLAGS_FINAL));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::Abstract", tuuvm_tuple_bitflags_encode(TUUVM_TYPE_FLAGS_ABSTRACT));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::Dynamic", tuuvm_tuple_bitflags_encode(TUUVM_TYPE_FLAGS_DYNAMIC));

    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::PointerValue", tuuvm_tuple_bitflags_encode(TUUVM_TYPE_FLAGS_POINTER_VALUE));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::ReferenceValue", tuuvm_tuple_bitflags_encode(TUUVM_TYPE_FLAGS_REFERENCE_VALUE));

    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::PointerLikeValue", tuuvm_tuple_bitflags_encode(TUUVM_TYPE_FLAGS_POINTER_LIKE_VALUE));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::ClassDefaultFlags", tuuvm_tuple_bitflags_encode(TUUVM_TYPE_FLAGS_CLASS_DEFAULT_FLAGS));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::MetatypeRequiredFlags", tuuvm_tuple_bitflags_encode(TUUVM_TYPE_FLAGS_METATYPE_REQUIRED_FLAGS));

    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::PointerTypeFlags", tuuvm_tuple_bitflags_encode(TUUVM_TYPE_FLAGS_POINTER_TYPE_FLAGS));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::ReferenceTypeFlags", tuuvm_tuple_bitflags_encode(TUUVM_TYPE_FLAGS_REFERENCE_TYPE_FLAGS));

    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::PrimitiveValueTypeDefaultFlags", tuuvm_tuple_bitflags_encode(TUUVM_TYPE_FLAGS_PRIMITIVE_VALUE_TYPE_DEFAULT_FLAGS));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::PrimitiveValueMetatypeDefaultFlags", tuuvm_tuple_bitflags_encode(TUUVM_TYPE_FLAGS_PRIMITIVE_VALUE_TYPE_METATYPE_FLAGS));

    // Export the type slot flags.
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeSlotFlags::None", tuuvm_tuple_bitflags_encode(TUUVM_TYPE_SLOT_FLAG_NONE));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeSlotFlags::Public", tuuvm_tuple_bitflags_encode(TUUVM_TYPE_SLOT_FLAG_PUBLIC));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeSlotFlags::ReadOnly", tuuvm_tuple_bitflags_encode(TUUVM_TYPE_SLOT_FLAG_READONLY));
}
