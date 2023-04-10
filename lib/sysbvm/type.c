#include "sysbvm/type.h"
#include "sysbvm/array.h"
#include "sysbvm/assert.h"
#include "sysbvm/ast.h"
#include "sysbvm/dictionary.h"
#include "sysbvm/errors.h"
#include "sysbvm/environment.h"
#include "sysbvm/function.h"
#include "sysbvm/stackFrame.h"
#include "sysbvm/string.h"
#include "sysbvm/type.h"
#include "internal/context.h"
#include <string.h>

SYSBVM_API sysbvm_tuple_t sysbvm_typeSlot_create(sysbvm_context_t *context, sysbvm_tuple_t name, sysbvm_tuple_t flags, sysbvm_tuple_t type, size_t localIndex, size_t index)
{
    sysbvm_typeSlot_t* result = (sysbvm_typeSlot_t*)sysbvm_context_allocatePointerTuple(context, context->roots.typeSlotType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_typeSlot_t));
    result->flags = flags;
    result->name = name;
    result->type = type;
    result->localIndex = sysbvm_tuple_size_encode(context, localIndex);
    result->index = sysbvm_tuple_size_encode(context, index);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_typeSlot_getValidReferenceType(sysbvm_context_t *context, sysbvm_tuple_t typeSlot)
{
    if(!sysbvm_tuple_isNonNullPointer(typeSlot))
        return sysbvm_type_createFunctionLocalReferenceType(context, context->roots.anyValueType);

    sysbvm_typeSlot_t *typeSlotObject = (sysbvm_typeSlot_t*)typeSlot;
    if(!typeSlotObject->referenceType)
        typeSlotObject->referenceType = sysbvm_type_createFunctionLocalReferenceType(context, typeSlotObject->type);

    return typeSlotObject->referenceType;
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_createAnonymous(sysbvm_context_t *context)
{
    sysbvm_type_tuple_t* result = (sysbvm_type_tuple_t*)sysbvm_context_allocatePointerTuple(context, context->roots.typeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_type_tuple_t));
    result->supertype = context->roots.anyValueType;
    result->totalSlotCount = sysbvm_tuple_size_encode(context, 0);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_createAnonymousMetatype(sysbvm_context_t *context)
{
    sysbvm_metatype_t* result = (sysbvm_metatype_t*)sysbvm_context_allocatePointerTuple(context, context->roots.metatypeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_metatype_t));
    result->super.flags = sysbvm_tuple_bitflags_encode(SYSBVM_TYPE_FLAGS_METATYPE_REQUIRED_FLAGS);

    size_t slotCount = SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_type_tuple_t);
    
    result->super.totalSlotCount = sysbvm_tuple_size_encode(context, slotCount);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_createAnonymousAndMetatype(sysbvm_context_t *context)
{
    sysbvm_tuple_t metatype = sysbvm_type_createAnonymousMetatype(context);
    sysbvm_type_tuple_t* result = (sysbvm_type_tuple_t*)sysbvm_context_allocatePointerTuple(context, metatype, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_type_tuple_t));
    result->supertype = context->roots.anyValueType;
    result->totalSlotCount = sysbvm_tuple_size_encode(context, 0);
    ((sysbvm_metatype_t*)metatype)->thisType = (sysbvm_tuple_t)result;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_createAnonymousClass(sysbvm_context_t *context, sysbvm_tuple_t supertype, sysbvm_tuple_t metaclass)
{
    size_t classSlotCount = sysbvm_type_getTotalSlotCount(metaclass);
    SYSBVM_ASSERT(classSlotCount >= SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_class_t));

    sysbvm_class_t* result = (sysbvm_class_t*)sysbvm_context_allocatePointerTuple(context, metaclass, classSlotCount);
    result->super.supertype = supertype;
    result->super.totalSlotCount = sysbvm_tuple_size_encode(context, 0);
    result->super.flags = sysbvm_tuple_bitflags_encode(SYSBVM_TYPE_FLAGS_CLASS_DEFAULT_FLAGS);
    if(supertype)
        result->super.totalSlotCount = sysbvm_tuple_size_encode(context, sysbvm_type_getTotalSlotCount(supertype));
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_createAnonymousMetaclass(sysbvm_context_t *context, sysbvm_tuple_t supertype)
{
    sysbvm_metaclass_t* result = (sysbvm_metaclass_t*)sysbvm_context_allocatePointerTuple(context, context->roots.metaclassType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_metaclass_t));
    result->super.super.supertype = supertype;
    result->super.super.flags = sysbvm_tuple_bitflags_encode(SYSBVM_TYPE_FLAGS_METATYPE_REQUIRED_FLAGS);

    size_t slotCount = SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_class_t);
    if(supertype)
    {
        size_t superTypeSlotCount = sysbvm_type_getTotalSlotCount(supertype);
        if(superTypeSlotCount > slotCount)
            slotCount = superTypeSlotCount;
    }
    
    result->super.super.totalSlotCount = sysbvm_tuple_size_encode(context, slotCount);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_createAnonymousClassAndMetaclass(sysbvm_context_t *context, sysbvm_tuple_t supertype)
{
    sysbvm_tuple_t metaclassSupertype = context->roots.classType;
    sysbvm_tuple_t actualSuperType = supertype;
    if(!supertype)
        actualSuperType = context->roots.objectType;

    if(sysbvm_tuple_isKindOf(context, actualSuperType, context->roots.classType))
        metaclassSupertype = sysbvm_tuple_getType(context, actualSuperType);
    else if(sysbvm_type_isDirectSubtypeOf(actualSuperType, context->roots.typeType))
        metaclassSupertype = sysbvm_tuple_getType(context, sysbvm_type_getSupertype(actualSuperType));

    sysbvm_tuple_t metaclass = sysbvm_type_createAnonymousMetaclass(context, metaclassSupertype);
    sysbvm_tuple_t class = sysbvm_type_createAnonymousClass(context, actualSuperType, metaclass);

    // Link together the class with its metaclass.
    sysbvm_metaclass_t *metaclassObject = (sysbvm_metaclass_t*)metaclass;
    metaclassObject->super.thisType = class;
    return class;
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_createAnonymousPrimitiveValueType(sysbvm_context_t *context, sysbvm_tuple_t supertype, sysbvm_tuple_t metaclass)
{
    size_t typeSlotCount = sysbvm_type_getTotalSlotCount(metaclass);
    SYSBVM_ASSERT(typeSlotCount >= SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_primitiveValueType_t));

    sysbvm_primitiveValueType_t* result = (sysbvm_primitiveValueType_t*)sysbvm_context_allocatePointerTuple(context, metaclass, typeSlotCount);
    result->super.super.supertype = supertype;
    result->super.super.totalSlotCount = sysbvm_tuple_size_encode(context, 0);
    result->super.super.flags = sysbvm_tuple_bitflags_encode(SYSBVM_TYPE_FLAGS_PRIMITIVE_VALUE_TYPE_DEFAULT_FLAGS);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_createAnonymousValueMetatype(sysbvm_context_t *context, sysbvm_tuple_t supertype, size_t minimumSlotCount)
{
    sysbvm_valueMetatype_t* result = (sysbvm_valueMetatype_t*)sysbvm_context_allocatePointerTuple(context, context->roots.valueMetatypeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_valueMetatype_t));
    result->super.super.supertype = supertype;
    result->super.super.flags = sysbvm_tuple_bitflags_encode(SYSBVM_TYPE_FLAGS_METATYPE_REQUIRED_FLAGS);

    size_t slotCount = minimumSlotCount;
    if(supertype)
    {
        size_t superTypeSlotCount = sysbvm_type_getTotalSlotCount(supertype);
        if(superTypeSlotCount > slotCount)
            slotCount = superTypeSlotCount;
    }
    
    result->super.super.totalSlotCount = sysbvm_tuple_size_encode(context, slotCount);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_createAnonymousPrimitiveValueTypeAndValueMetatype(sysbvm_context_t *context, sysbvm_tuple_t supertype)
{
    sysbvm_tuple_t metatypeSupertype = context->roots.primitiveValueType;
    sysbvm_tuple_t actualSuperType = supertype;
    sysbvm_tuple_t supertypeMetatype = sysbvm_tuple_getType(context, supertype);
    if(sysbvm_type_isDirectSubtypeOf(supertypeMetatype, context->roots.primitiveValueType))
        metatypeSupertype = supertypeMetatype;

    sysbvm_tuple_t metatype = sysbvm_type_createAnonymousValueMetatype(context, metatypeSupertype, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_primitiveValueType_t));
    sysbvm_tuple_t primitiveValueType = sysbvm_type_createAnonymousPrimitiveValueType(context, actualSuperType, metatype);

    sysbvm_type_setFlags(context, metatype, SYSBVM_TYPE_FLAGS_PRIMITIVE_VALUE_TYPE_METATYPE_FLAGS);

    // Link together the type with its metatype.
    sysbvm_metatype_t *metatypeObject = (sysbvm_metatype_t*)metatype;
    metatypeObject->thisType = primitiveValueType;
    return primitiveValueType;
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_createWithName(sysbvm_context_t *context, sysbvm_tuple_t name)
{
    sysbvm_tuple_t result = sysbvm_type_createAnonymous(context);
    sysbvm_type_setName(result, name);
    return result;
}

static sysbvm_tuple_t sysbvm_type_doCreateSimpleFunctionType(sysbvm_context_t *context, sysbvm_tuple_t argumentTypes, sysbvm_tuple_t isVariadic, sysbvm_tuple_t resultType)
{
    sysbvm_type_tuple_t *supertype = (sysbvm_type_tuple_t*)context->roots.functionType;
    
    sysbvm_simpleFunctionType_t* result = (sysbvm_simpleFunctionType_t*)sysbvm_context_allocatePointerTuple(context, context->roots.simpleFunctionTypeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_simpleFunctionType_t));
    result->super.supertype = (sysbvm_tuple_t)supertype;
    result->super.flags = sysbvm_tuple_bitflags_encode(sysbvm_tuple_bitflags_decode(supertype->flags) | SYSBVM_TYPE_FLAGS_FUNCTION);
    result->super.totalSlotCount = supertype->totalSlotCount;
    result->argumentTypes = argumentTypes;
    result->isVariadic = isVariadic;
    result->resultType = resultType;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_createSimpleFunctionType(sysbvm_context_t *context, sysbvm_tuple_t argumentTypes, bool isVariadic, sysbvm_tuple_t resultType)
{
    return sysbvm_function_apply3(context, context->roots.simpleFunctionTypeTemplate, argumentTypes, sysbvm_tuple_boolean_encode(isVariadic), resultType);
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_createSimpleFunctionTypeWithArguments0(sysbvm_context_t *context, sysbvm_tuple_t resultType)
{
    struct {
        sysbvm_tuple_t arguments;
        sysbvm_tuple_t result;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.arguments = sysbvm_array_create(context, 0);
    gcFrame.result = sysbvm_type_createSimpleFunctionType(context, gcFrame.arguments, false, resultType);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_createSimpleFunctionTypeWithArguments1(sysbvm_context_t *context, sysbvm_tuple_t argument, sysbvm_tuple_t resultType)
{
    struct {
        sysbvm_tuple_t arguments;
        sysbvm_tuple_t result;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.arguments = sysbvm_array_create(context, 1);
    sysbvm_array_atPut(gcFrame.arguments, 0, argument);

    gcFrame.result = sysbvm_type_createSimpleFunctionType(context, gcFrame.arguments, false, resultType);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_createSimpleFunctionTypeWithArguments2(sysbvm_context_t *context, sysbvm_tuple_t argument0, sysbvm_tuple_t argument1, sysbvm_tuple_t resultType)
{
    struct {
        sysbvm_tuple_t arguments;
        sysbvm_tuple_t result;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.arguments = sysbvm_array_create(context, 2);
    sysbvm_array_atPut(gcFrame.arguments, 0, argument0);
    sysbvm_array_atPut(gcFrame.arguments, 1, argument1);

    gcFrame.result = sysbvm_type_createSimpleFunctionType(context, gcFrame.arguments, false, resultType);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_pointerLikeType_createPointerLikeLoadFunction(sysbvm_context_t *context, sysbvm_tuple_t pointerLikeType_, sysbvm_tuple_t baseType_, sysbvm_bitflags_t extraFlags)
{
    struct {
        sysbvm_tuple_t pointerLikeType;
        sysbvm_tuple_t baseType;
        sysbvm_tuple_t function;
        sysbvm_tuple_t functionType;
    } gcFrame = {
        .pointerLikeType = pointerLikeType_,
        .baseType = baseType_,
    };
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.function = sysbvm_context_shallowCopy(context, context->roots.pointerLikeLoadPrimitive);
    sysbvm_function_addFlags(context, gcFrame.function, extraFlags);

    gcFrame.functionType = sysbvm_type_createSimpleFunctionTypeWithArguments1(context, gcFrame.pointerLikeType, gcFrame.baseType);
    sysbvm_tuple_setType((sysbvm_object_tuple_t*)gcFrame.function, gcFrame.functionType);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.function;
}

static sysbvm_tuple_t sysbvm_pointerLikeType_createPointerLikeStoreFunction(sysbvm_context_t *context, sysbvm_tuple_t pointerLikeType_, sysbvm_tuple_t baseType_, sysbvm_bitflags_t extraFlags)
{
    struct {
        sysbvm_tuple_t pointerLikeType;
        sysbvm_tuple_t baseType;
        sysbvm_tuple_t function;
        sysbvm_tuple_t functionType;
    } gcFrame = {
        .pointerLikeType = pointerLikeType_,
        .baseType = baseType_,
    };
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.function = sysbvm_context_shallowCopy(context, context->roots.pointerLikeStorePrimitive);
    sysbvm_function_addFlags(context, gcFrame.function, extraFlags);

    gcFrame.functionType = sysbvm_type_createSimpleFunctionTypeWithArguments2(context, gcFrame.pointerLikeType, gcFrame.baseType, gcFrame.pointerLikeType);
    sysbvm_tuple_setType((sysbvm_object_tuple_t*)gcFrame.function, gcFrame.functionType);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.function;
}

static sysbvm_tuple_t sysbvm_type_doCreatePointerType(sysbvm_context_t *context, sysbvm_tuple_t baseType_, sysbvm_tuple_t addressSpace_)
{
    struct {
        sysbvm_tuple_t baseType;
        sysbvm_tuple_t addressSpace;
        sysbvm_pointerType_t* result;
        sysbvm_tuple_t storeFunction;
        sysbvm_tuple_t loadFunction;

    } gcFrame = {
        .baseType = baseType_,
        .addressSpace = addressSpace_,
    };
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.result = (sysbvm_pointerType_t*)sysbvm_context_allocatePointerTuple(context, context->roots.pointerType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_pointerType_t));
    gcFrame.result->super.super.super.flags = sysbvm_tuple_bitflags_encode(SYSBVM_TYPE_FLAGS_POINTER_TYPE_FLAGS);
    gcFrame.result->super.super.super.totalSlotCount = sysbvm_tuple_size_encode(context, 0);
    gcFrame.result->super.super.super.supertype = context->roots.anyPointerType;
    gcFrame.result->super.baseType = gcFrame.baseType;
    gcFrame.result->super.addressSpace = gcFrame.addressSpace;

    gcFrame.storeFunction = sysbvm_pointerLikeType_createPointerLikeStoreFunction(context, (sysbvm_tuple_t)gcFrame.result, gcFrame.baseType, 0);
    gcFrame.result->super.storeValueFunction = gcFrame.storeFunction;

    gcFrame.loadFunction = sysbvm_pointerLikeType_createPointerLikeLoadFunction(context, (sysbvm_tuple_t)gcFrame.result, gcFrame.baseType, 0);
    gcFrame.result->super.loadValueFunction = gcFrame.loadFunction;

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (sysbvm_tuple_t)gcFrame.result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_createPointerType(sysbvm_context_t *context, sysbvm_tuple_t baseType, sysbvm_tuple_t addressSpace)
{
    return sysbvm_function_apply2(context, context->roots.pointerTypeTemplate, baseType, addressSpace);
}

static sysbvm_tuple_t sysbvm_type_doCreateReferenceType(sysbvm_context_t *context, sysbvm_tuple_t baseType_, sysbvm_tuple_t addressSpace_)
{
    struct {
        sysbvm_tuple_t baseType;
        sysbvm_tuple_t addressSpace;
        sysbvm_referenceType_t* result;
        sysbvm_tuple_t storeFunction;
        sysbvm_tuple_t loadFunction;

    } gcFrame = {
        .baseType = baseType_,
        .addressSpace = addressSpace_,
    };
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.result = (sysbvm_referenceType_t*)sysbvm_context_allocatePointerTuple(context, context->roots.referenceType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_referenceType_t));
    gcFrame.result->super.super.super.flags = sysbvm_tuple_bitflags_encode(SYSBVM_TYPE_FLAGS_REFERENCE_TYPE_FLAGS);
    gcFrame.result->super.super.super.totalSlotCount = sysbvm_tuple_size_encode(context, 0);
    gcFrame.result->super.super.super.supertype = context->roots.anyReferenceType;
    gcFrame.result->super.baseType = gcFrame.baseType;
    gcFrame.result->super.addressSpace = gcFrame.addressSpace;

    gcFrame.storeFunction = sysbvm_pointerLikeType_createPointerLikeStoreFunction(context, (sysbvm_tuple_t)gcFrame.result, gcFrame.baseType, SYSBVM_FUNCTION_FLAGS_ALLOW_REFERENCE_IN_RECEIVER);
    sysbvm_type_setMethodWithSelector(context, (sysbvm_tuple_t)gcFrame.result, context->roots.assignmentSelector, gcFrame.storeFunction);
    gcFrame.result->super.storeValueFunction = gcFrame.storeFunction;

    gcFrame.loadFunction = sysbvm_pointerLikeType_createPointerLikeLoadFunction(context, (sysbvm_tuple_t)gcFrame.result, gcFrame.baseType, SYSBVM_FUNCTION_FLAGS_ALLOW_REFERENCE_IN_RECEIVER);
    gcFrame.result->super.loadValueFunction = gcFrame.loadFunction;

    return (sysbvm_tuple_t)gcFrame.result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_createReferenceType(sysbvm_context_t *context, sysbvm_tuple_t baseType, sysbvm_tuple_t addressSpace)
{
    return sysbvm_function_apply2(context, context->roots.referenceTypeTemplate, baseType, addressSpace);
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_createFunctionLocalReferenceType(sysbvm_context_t *context, sysbvm_tuple_t baseType)
{
    return sysbvm_type_createReferenceType(context, baseType, SYSBVM_NULL_TUPLE);
}

SYSBVM_API sysbvm_tuple_t sysbvm_valueBox_with(sysbvm_context_t *context, sysbvm_tuple_t boxedValue)
{
    sysbvm_object_tuple_t *box = sysbvm_context_allocatePointerTuple(context, context->roots.valueBoxType, 1);
    box->pointers[0] = boxedValue;
    return (sysbvm_tuple_t)box;
}

SYSBVM_API sysbvm_tuple_t sysbvm_pointerLikeType_withStorage(sysbvm_context_t *context, sysbvm_tuple_t pointerLikeType, sysbvm_tuple_t storage)
{
    sysbvm_object_tuple_t *reference = sysbvm_context_allocatePointerTuple(context, pointerLikeType, 1);
    reference->pointers[0] = storage;
    return (sysbvm_tuple_t)reference;
}

SYSBVM_API sysbvm_tuple_t sysbvm_pointerLikeType_withBoxForValue(sysbvm_context_t *context, sysbvm_tuple_t pointerLikeType, sysbvm_tuple_t boxedValue)
{
    return sysbvm_pointerLikeType_withStorage(context, pointerLikeType, sysbvm_valueBox_with(context, boxedValue));
}

SYSBVM_API sysbvm_tuple_t sysbvm_pointerLikeType_withEmptyBox(sysbvm_context_t *context, sysbvm_tuple_t pointerLikeType)
{
    return sysbvm_pointerLikeType_withBoxForValue(context, pointerLikeType, SYSBVM_NULL_TUPLE);
}

SYSBVM_API sysbvm_tuple_t sysbvm_referenceType_withStorage(sysbvm_context_t *context, sysbvm_tuple_t referenceType, sysbvm_tuple_t storage)
{
    if(!sysbvm_type_isReferenceType(referenceType))
        sysbvm_error("Expected a reference type");

    sysbvm_object_tuple_t *reference = sysbvm_context_allocatePointerTuple(context, referenceType, 1);
    reference->pointers[0] = storage;
    return (sysbvm_tuple_t)reference;
}

SYSBVM_API sysbvm_tuple_t sysbvm_referenceType_withBoxForValue(sysbvm_context_t *context, sysbvm_tuple_t referenceType, sysbvm_tuple_t boxedValue)
{
    return sysbvm_referenceType_withStorage(context, referenceType, sysbvm_valueBox_with(context, boxedValue));
}

SYSBVM_API sysbvm_tuple_t sysbvm_referenceType_withTupleAndTypeSlot(sysbvm_context_t *context, sysbvm_tuple_t referenceType, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot)
{
    if(!sysbvm_type_isReferenceType(referenceType))
        sysbvm_error("Expected a reference type");

    sysbvm_object_tuple_t *reference = sysbvm_context_allocatePointerTuple(context, referenceType, 2);
    reference->pointers[0] = typeSlot;
    reference->pointers[1] = tuple;
    return (sysbvm_tuple_t)reference;
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_createDependentFunctionType(sysbvm_context_t *context, sysbvm_tuple_t argumentNodes, bool isVariadic, sysbvm_tuple_t resultTypeNode,
    sysbvm_tuple_t environment, sysbvm_tuple_t captureBindings, sysbvm_tuple_t argumentBindings, sysbvm_tuple_t localBindings)
{
    sysbvm_type_tuple_t *supertype = (sysbvm_type_tuple_t*)context->roots.functionType;

    sysbvm_dependentFunctionType_t* result = (sysbvm_dependentFunctionType_t*)sysbvm_context_allocatePointerTuple(context, context->roots.dependentFunctionTypeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_dependentFunctionType_t));
    result->super.supertype = (sysbvm_tuple_t)supertype;
    result->super.flags = sysbvm_tuple_bitflags_encode(sysbvm_tuple_bitflags_decode(supertype->flags) | SYSBVM_TYPE_FLAGS_FUNCTION);
    result->super.totalSlotCount = supertype->totalSlotCount;
    result->argumentNodes = argumentNodes;
    result->isVariadic = sysbvm_tuple_boolean_encode(isVariadic);
    result->resultTypeNode = resultTypeNode;

    result->environment = environment;
    result->captureBindings = captureBindings;
    result->argumentBindings = argumentBindings;
    result->localBindings = localBindings;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API size_t sysbvm_type_getTotalSlotCount(sysbvm_tuple_t type)
{
    if(!sysbvm_tuple_isNonNullPointer(type)) return SYSBVM_NULL_TUPLE;
    return sysbvm_tuple_size_decode(((sysbvm_type_tuple_t*)type)->totalSlotCount);
}

SYSBVM_API void sysbvm_type_setTotalSlotCount(sysbvm_context_t *context, sysbvm_tuple_t type, size_t totalSlotCount)
{
    if(!sysbvm_tuple_isNonNullPointer(type)) return;
    ((sysbvm_type_tuple_t*)type)->totalSlotCount = sysbvm_tuple_size_encode(context, totalSlotCount);
}

SYSBVM_API void sysbvm_type_setFlags(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_bitflags_t flags)
{
    (void)context;
    if(!sysbvm_tuple_isNonNullPointer(type)) return;
    ((sysbvm_type_tuple_t*)type)->flags = sysbvm_tuple_bitflags_encode(flags);
}

SYSBVM_API void sysbvm_typeAndMetatype_setFlags(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_bitflags_t flags, sysbvm_bitflags_t metatypeFlags)
{
    sysbvm_type_setFlags(context, type, flags);
    sysbvm_type_setFlags(context, sysbvm_tuple_getType(context, type), SYSBVM_TYPE_FLAGS_METATYPE_REQUIRED_FLAGS | metatypeFlags);
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_lookupMacroSelector(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t selector)
{
    if(!sysbvm_tuple_isNonNullPointer(type)) return SYSBVM_NULL_TUPLE;
    sysbvm_tuple_t methodDictionary = sysbvm_type_getMacroMethodDictionary(type);
    if(methodDictionary)
    {
        sysbvm_tuple_t found = SYSBVM_NULL_TUPLE;
        if(sysbvm_methodDictionary_find(methodDictionary, selector, &found))
            return found;
    }

    return sysbvm_type_lookupMacroSelector(context, sysbvm_type_getSupertype(type), selector);
}

static inline size_t computeLookupCacheEntryIndexFor(sysbvm_tuple_t type, sysbvm_tuple_t selector)
{
    return sysbvm_hashConcatenate(sysbvm_tuple_identityHash(type), sysbvm_tuple_identityHash(selector)) % GLOBAL_LOOKUP_CACHE_ENTRY_COUNT;
}

static sysbvm_tuple_t sysbvm_type_lookupSelectorRecursively(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t selector)
{
    if(!sysbvm_tuple_isNonNullPointer(type)) return SYSBVM_NULL_TUPLE;
    if(sysbvm_tuple_isDummyValue(type)) return SYSBVM_NULL_TUPLE;
    sysbvm_tuple_t methodDictionary = sysbvm_type_getMethodDictionary(type);
    if(methodDictionary)
    {
        sysbvm_tuple_t found = SYSBVM_NULL_TUPLE;
        if(sysbvm_methodDictionary_find(methodDictionary, selector, &found))
            return found;
    }

    return sysbvm_type_lookupSelectorRecursively(context, sysbvm_type_getSupertype(type), selector);
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_lookupSlot(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t slotName)
{
    if(!sysbvm_tuple_isNonNullPointer(type)) return SYSBVM_NULL_TUPLE;
    if(sysbvm_tuple_isDummyValue(type)) return SYSBVM_NULL_TUPLE;

    sysbvm_tuple_t slotDictionary = sysbvm_type_getSlotDictionary(type);
    if(slotDictionary)
    {
        sysbvm_tuple_t found = SYSBVM_NULL_TUPLE;
        if(sysbvm_methodDictionary_find(slotDictionary, slotName, &found))
            return found;
    }

    return sysbvm_type_lookupSlot(context, sysbvm_type_getSupertype(type), slotName);
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_lookupSelector(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t selector)
{
    if(!sysbvm_tuple_isNonNullPointer(type)) return SYSBVM_NULL_TUPLE;
    if(sysbvm_tuple_isDummyValue(type)) return SYSBVM_NULL_TUPLE;

    // FIXME: Make this cache atomic and thread safe.
    size_t cacheEntryIndex = computeLookupCacheEntryIndexFor(type, selector);
    sysbvm_globalLookupCacheEntry_t *cacheEntry = context->roots.globalMethodLookupCache + cacheEntryIndex;
    if(cacheEntry->type != type || cacheEntry->selector != selector)
    {
        sysbvm_tuple_t method = sysbvm_type_lookupSelectorRecursively(context, type, selector);
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

SYSBVM_API sysbvm_tuple_t sysbvm_type_lookupSelectorWithInlineCache(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t selector, sysbvm_inlineLookupCacheEntry_t *inlineCache)
{
    // FIXME: Make this thread safe
    if(inlineCache->receiverType == type)
        return inlineCache->method;
    
    inlineCache->receiverType = type;
    return inlineCache->method = sysbvm_type_lookupSelector(context, type, selector);
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_lookupSelectorWithPIC(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t selector, sysbvm_polymorphicInlineLookupCache_t *pic)
{
    size_t expectedEntryIndex = sysbvm_tuple_identityHash(type) % PIC_ENTRY_COUNT;
    sysbvm_inlineLookupCacheEntry_t *entry = pic->entries + expectedEntryIndex;
    if(entry->receiverType == type)
        return entry->method;
    
    entry->receiverType = type;
    return entry->method = sysbvm_type_lookupSelector(context, type, selector);
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_lookupFallbackSelector(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t selector)
{
    if(!sysbvm_tuple_isNonNullPointer(type)) return SYSBVM_NULL_TUPLE;
    if(sysbvm_tuple_isDummyValue(type)) return SYSBVM_NULL_TUPLE;
    sysbvm_tuple_t methodDictionary = sysbvm_type_getFallbackMethodDictionary(type);
    if(methodDictionary)
    {
        sysbvm_tuple_t found = SYSBVM_NULL_TUPLE;
        if(sysbvm_methodDictionary_find(methodDictionary, selector, &found))
            return found;
    }

    return sysbvm_type_lookupFallbackSelector(context, sysbvm_type_getSupertype(type), selector);
}

static sysbvm_tuple_t sysbvm_type_lookupSelectorOrFallbackWith(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t selector, sysbvm_tuple_t fallbackMethod)
{
    sysbvm_tuple_t found = sysbvm_type_lookupSelector(context, type, selector);
    if(found)
        return found;
    else
        return fallbackMethod;
}

SYSBVM_API void sysbvm_type_setMethodWithSelector(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t selector, sysbvm_tuple_t method)
{
    if(!sysbvm_tuple_isNonNullPointer(type)) return;

    sysbvm_type_tuple_t* typeObject = (sysbvm_type_tuple_t*)type;
    if(!typeObject->methodDictionary)
        typeObject->methodDictionary = sysbvm_methodDictionary_create(context);
    sysbvm_methodDictionary_atPut(context, typeObject->methodDictionary, selector, method);

    if(sysbvm_tuple_isFunction(context, method))
    {
        sysbvm_function_t *functionObject = (sysbvm_function_t*)method;
        if(!functionObject->owner && !functionObject->name)
        {
            functionObject->owner = type;
            functionObject->name = selector;
        }
    }
}

SYSBVM_API void sysbvm_type_buildSlotDictionary(sysbvm_context_t *context, sysbvm_tuple_t type)
{
    if(!sysbvm_tuple_isNonNullPointer(type)) return;

    sysbvm_type_tuple_t* typeObject = (sysbvm_type_tuple_t*)type;
    size_t slotCount = sysbvm_array_getSize(typeObject->slots);
    if(slotCount == 0) return;

    if(!typeObject->slotDictionary)
        typeObject->slotDictionary = sysbvm_methodDictionary_createWithCapacity(context, slotCount);

    for(size_t i = 0; i < slotCount; ++i)
    {
        sysbvm_typeSlot_t *typeSlot = (sysbvm_typeSlot_t*)sysbvm_array_at(typeObject->slots, i);
        if(typeSlot->name)
            sysbvm_methodDictionary_atPut(context, typeObject->slotDictionary, typeSlot->name, (sysbvm_tuple_t)typeSlot);
    }
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_getEqualsFunction(sysbvm_context_t *context, sysbvm_tuple_t type)
{
    return sysbvm_type_lookupSelectorOrFallbackWith(context, type, context->roots.equalsSelector, context->roots.identityEqualsFunction);
}

SYSBVM_API void sysbvm_type_setEqualsFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t equalsFunction)
{
    sysbvm_type_setMethodWithSelector(context, type, context->roots.equalsSelector, equalsFunction);
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_getHashFunction(sysbvm_context_t *context, sysbvm_tuple_t type)
{
    return sysbvm_type_lookupSelectorOrFallbackWith(context, type, context->roots.hashSelector, context->roots.identityHashFunction);
}

SYSBVM_API void sysbvm_type_setHashFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t hashFunction)
{
    sysbvm_type_setMethodWithSelector(context, type, context->roots.hashSelector, hashFunction);
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_getAsStringFunction(sysbvm_context_t *context, sysbvm_tuple_t type)
{
    return sysbvm_type_lookupSelector(context, type, context->roots.asStringSelector);
}

SYSBVM_API void sysbvm_type_setAsStringFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t asStringFunction)
{
    sysbvm_type_setMethodWithSelector(context, type, context->roots.asStringSelector, asStringFunction);
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_getPrintStringFunction(sysbvm_context_t *context, sysbvm_tuple_t type)
{
    return sysbvm_type_lookupSelector(context, type, context->roots.printStringSelector);
}

SYSBVM_API void sysbvm_type_setPrintStringFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t printStringFunction)
{
    sysbvm_type_setMethodWithSelector(context, type, context->roots.printStringSelector, printStringFunction);
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_getAstNodeAnalysisFunction(sysbvm_context_t *context, sysbvm_tuple_t type)
{
    return sysbvm_type_lookupSelectorWithPIC(context, type, context->roots.astNodeAnalysisSelector, &context->roots.inlineCaches.analyzeASTWithEnvironment);
}

SYSBVM_API void sysbvm_type_setAstNodeAnalysisFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t astNodeAnalysisFunction)
{
    sysbvm_type_setMethodWithSelector(context, type, context->roots.astNodeAnalysisSelector, astNodeAnalysisFunction);
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_getAstNodeEvaluationFunction(sysbvm_context_t *context, sysbvm_tuple_t type)
{
    return sysbvm_type_lookupSelectorWithPIC(context, type, context->roots.astNodeEvaluationSelector, &context->roots.inlineCaches.evaluateASTWithEnvironment);
}

SYSBVM_API void sysbvm_type_setAstNodeEvaluationFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t astNodeEvaluationFunction)
{
    sysbvm_type_setMethodWithSelector(context, type, context->roots.astNodeEvaluationSelector, astNodeEvaluationFunction);
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_getAstNodeAnalysisAndEvaluationFunction(sysbvm_context_t *context, sysbvm_tuple_t type)
{
    return sysbvm_type_lookupSelectorWithPIC(context, type, context->roots.astNodeAnalysisAndEvaluationSelector, &context->roots.inlineCaches.evaluateAndAnalyzeASTWithEnvironment);
}

SYSBVM_API void sysbvm_type_setAstNodeAnalysisAndEvaluationFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t astNodeAnalysisAndEvaluationFunction)
{
    sysbvm_type_setMethodWithSelector(context, type, context->roots.astNodeAnalysisAndEvaluationSelector, astNodeAnalysisAndEvaluationFunction);
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_getAstNodeValidationWithAnalysisAndEvaluationFunction(sysbvm_context_t *context, sysbvm_tuple_t type)
{
    return sysbvm_type_lookupSelector(context, type, context->roots.astNodeValidateThenAnalyzeAndEvaluateWithEnvironmentSelector);
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_getAnalyzeAndEvaluateMessageSendNodeForReceiverWithEnvironmentFunction(sysbvm_context_t *context, sysbvm_tuple_t type)
{
    return sysbvm_type_lookupSelector(context, type, context->roots.analyzeAndEvaluateMessageSendNodeForReceiverWithEnvironmentSelector);
}

SYSBVM_API void sysbvm_type_setAnalyzeAndEvaluateMessageSendNodeForReceiverWithEnvironmentFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t function)
{
    sysbvm_type_setMethodWithSelector(context, type, context->roots.analyzeAndEvaluateMessageSendNodeForReceiverWithEnvironmentSelector, function);
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_getAnalyzeMessageSendNodeWithEnvironmentFunction(sysbvm_context_t *context, sysbvm_tuple_t type)
{
    return sysbvm_type_lookupSelector(context, type, context->roots.analyzeMessageSendNodeWithEnvironmentSelector);
}

SYSBVM_API void sysbvm_type_setAnalyzeMessageSendNodeWithEnvironmentFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t function)
{
    sysbvm_type_setMethodWithSelector(context, type, context->roots.analyzeMessageSendNodeWithEnvironmentSelector, function);
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_getAnalyzeAndEvaluateMessageChainNodeForReceiverWithEnvironmentFunction(sysbvm_context_t *context, sysbvm_tuple_t type)
{
    return sysbvm_type_lookupSelector(context, type, context->roots.analyzeAndEvaluateMessageChainNodeForReceiverWithEnvironmentSelector);
}

SYSBVM_API void sysbvm_type_setAnalyzeAndEvaluateMessageChainNodeForReceiverWithEnvironmentFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t function)
{
    sysbvm_type_setMethodWithSelector(context, type, context->roots.analyzeAndEvaluateMessageChainNodeForReceiverWithEnvironmentSelector, function);
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_getAnalyzeMessageChainNodeWithEnvironmentFunction(sysbvm_context_t *context, sysbvm_tuple_t type)
{
    return sysbvm_type_lookupSelector(context, type, context->roots.analyzeMessageChainNodeWithEnvironmentSelector);
}

SYSBVM_API void sysbvm_type_setAnalyzeMessageChainNodeWithEnvironmentFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t function)
{
    sysbvm_type_setMethodWithSelector(context, type, context->roots.analyzeMessageChainNodeWithEnvironmentSelector, function);
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_getAnalyzeAndEvaluateConcreteMetaValueWithEnvironmentFunction(sysbvm_context_t *context, sysbvm_tuple_t type)
{
    return sysbvm_type_lookupSelector(context, type, context->roots.analyzeAndEvaluateConcreteMetaValueWithEnvironmentSelector);
}

SYSBVM_API void sysbvm_type_setAnalyzeAndEvaluateConcreteMetaValueWithEnvironmentFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t function)
{
    sysbvm_type_setMethodWithSelector(context, type, context->roots.analyzeAndEvaluateConcreteMetaValueWithEnvironmentSelector, function);
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_getAnalyzeConcreteMetaValueWithEnvironmentFunction(sysbvm_context_t *context, sysbvm_tuple_t type)
{
    return sysbvm_type_lookupSelector(context, type, context->roots.analyzeConcreteMetaValueWithEnvironmentSelector);
}

SYSBVM_API void sysbvm_type_setAnalyzeConcreteMetaValueWithEnvironmentFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t function)
{
    sysbvm_type_setMethodWithSelector(context, type, context->roots.analyzeConcreteMetaValueWithEnvironmentSelector, function);
}
SYSBVM_API sysbvm_tuple_t sysbvm_type_getCoerceValueFunction(sysbvm_context_t *context, sysbvm_tuple_t type)
{
    return sysbvm_type_lookupSelector(context, type, context->roots.coerceValueSelector);
}

SYSBVM_API void sysbvm_type_setCoerceValueFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t coerceValueFunction)
{
    sysbvm_type_setMethodWithSelector(context, type, context->roots.coerceValueSelector, coerceValueFunction);
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_getAnalyzeAndTypeCheckFunctionApplicationNodeWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t type)
{
    return sysbvm_type_lookupSelector(context, type, context->roots.analyzeAndTypeCheckFunctionApplicationNodeWithEnvironmentSelector);
}

SYSBVM_API void sysbvm_type_setAnalyzeAndTypeCheckFunctionApplicationNodeWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t typeCheckFunction)
{
    sysbvm_type_setMethodWithSelector(context, type, context->roots.analyzeAndTypeCheckFunctionApplicationNodeWithEnvironmentSelector, typeCheckFunction);
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_getAnalyzeAndTypeCheckMessageSendNodeWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t type)
{
    return sysbvm_type_lookupSelector(context, type, context->roots.analyzeAndTypeCheckMessageSendNodeWithEnvironmentSelector);
}

SYSBVM_API void sysbvm_type_setAnalyzeAndTypeCheckMessageSendNodeWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t typeCheckFunction)
{
    sysbvm_type_setMethodWithSelector(context, type, context->roots.analyzeAndTypeCheckMessageSendNodeWithEnvironmentSelector, typeCheckFunction);
}

SYSBVM_API sysbvm_tuple_t sysbvm_pointerLikeType_load(sysbvm_context_t *context, sysbvm_tuple_t pointerLikeValue)
{
    if(!pointerLikeValue)
        sysbvm_error("Accessing null pointer-");
    if(!sysbvm_type_isPointerLikeType(sysbvm_tuple_getType(context, pointerLikeValue)))
        sysbvm_error("Expected a pointer like value.");

    size_t slotCount = sysbvm_tuple_getSizeInSlots(pointerLikeValue);
    if(slotCount == 0)
        sysbvm_error("Expected a pointer like value with at least single slot.");

    sysbvm_object_tuple_t *pointerObject = (sysbvm_object_tuple_t*)pointerLikeValue;
    sysbvm_tuple_t storage = pointerObject->pointers[0];
    sysbvm_tuple_t base = SYSBVM_NULL_TUPLE;
    intptr_t offset = sysbvm_tuple_intptr_encode(context, 0);
    if(slotCount >= 2)
    {
        base = pointerObject->pointers[1];
        if(slotCount >= 3)
            offset = pointerObject->pointers[2];
    }

    if(sysbvm_tuple_getType(context, storage) == context->roots.valueBoxType)
        return SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(storage)->pointers[0];

    sysbvm_pointerLikeType_t *pointerLikeType = (sysbvm_pointerLikeType_t*)sysbvm_tuple_getType(context, pointerLikeValue);
    return sysbvm_tuple_send3(context, context->roots.loadFromAtOffsetWithTypeSelector, storage, base, offset, pointerLikeType->baseType);
}

SYSBVM_API sysbvm_tuple_t sysbvm_pointerLikeType_store(sysbvm_context_t *context, sysbvm_tuple_t pointerLikeValue_, sysbvm_tuple_t valueToStore_)
{
    struct {
        sysbvm_tuple_t pointerLikeValue;
        sysbvm_tuple_t valueToStore;

        sysbvm_pointerLikeType_t *pointerLikeType;
        sysbvm_tuple_t coercedValue;
        sysbvm_tuple_t storage;
        sysbvm_tuple_t base;
        sysbvm_tuple_t offset;
    } gcFrame = {
        .pointerLikeValue = pointerLikeValue_,
        .valueToStore = valueToStore_
    };
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    if(!gcFrame.pointerLikeValue) sysbvm_error("Accessing null pointer.");
    if(!sysbvm_type_isPointerLikeType(sysbvm_tuple_getType(context, gcFrame.pointerLikeValue)))
        sysbvm_error("Expected a pointer like value.");

    size_t slotCount = sysbvm_tuple_getSizeInSlots(gcFrame.pointerLikeValue);
    if(slotCount == 0) sysbvm_error("Expected a pointer like value with at least single slot.");

    gcFrame.pointerLikeType = (sysbvm_pointerLikeType_t*)sysbvm_tuple_getType(context, gcFrame.pointerLikeValue);
    gcFrame.coercedValue = sysbvm_type_coerceValue(context, gcFrame.pointerLikeType->baseType, gcFrame.valueToStore);

    sysbvm_object_tuple_t **pointerObject = (sysbvm_object_tuple_t**)&gcFrame.pointerLikeValue;
    gcFrame.storage = (*pointerObject)->pointers[0];
    gcFrame.base = SYSBVM_NULL_TUPLE;
    gcFrame.offset = sysbvm_tuple_intptr_encode(context, 0);
    if(slotCount >= 2)
    {
        gcFrame.base = (*pointerObject)->pointers[1];
        if(slotCount >= 3)
            gcFrame.offset = (*pointerObject)->pointers[2];
    }
    else
    {
        if(sysbvm_tuple_getType(context, gcFrame.storage) == context->roots.valueBoxType)
        {
            SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(gcFrame.storage)->pointers[0] = gcFrame.coercedValue;
            SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
            return gcFrame.pointerLikeValue;
        }
    }

    sysbvm_tuple_send4(context, context->roots.storeInAtOffsetWithTypeSelector, gcFrame.storage, gcFrame.valueToStore, gcFrame.base, gcFrame.offset, gcFrame.pointerLikeType->baseType);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.pointerLikeValue;
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_coerceValue(sysbvm_context_t *context, sysbvm_tuple_t type_, sysbvm_tuple_t value_)
{
    struct {
        sysbvm_tuple_t type;
        sysbvm_tuple_t value;
        sysbvm_tuple_t valueType;
        sysbvm_tuple_t function;
    } gcFrame = {
        .type = type_,
        .value = value_
    };
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.valueType = sysbvm_tuple_getType(context, gcFrame.value);
    if(!gcFrame.type)
    {
        if(sysbvm_type_isReferenceType(gcFrame.valueType))
        {
            SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
            return sysbvm_pointerLikeType_load(context, gcFrame.value);
        }
        
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return gcFrame.value;
    }

    // Load references.
    if(sysbvm_type_isReferenceType(gcFrame.valueType) && !sysbvm_type_isDirectSubtypeOf(gcFrame.valueType, gcFrame.type))
    {
        gcFrame.value = sysbvm_pointerLikeType_load(context, gcFrame.value);
        gcFrame.valueType = sysbvm_tuple_getType(context, gcFrame.value);
    }

    if(!gcFrame.value && sysbvm_type_isNullable(gcFrame.type))
    {
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return gcFrame.value;
    }

    if(sysbvm_type_isDirectSubtypeOf(gcFrame.valueType, gcFrame.type))
    {
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return gcFrame.value;
    }

    gcFrame.function = sysbvm_type_getCoerceValueFunction(context, sysbvm_tuple_getType(context, gcFrame.type));
    if(gcFrame.function)
    {
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return sysbvm_function_applyNoCheck2(context, gcFrame.function, gcFrame.type, gcFrame.value);
    }

    if(!gcFrame.valueType && sysbvm_tuple_isDummyValue(gcFrame.value))
    {
        sysbvm_tuple_t coercedDummyValue = (sysbvm_tuple_t)sysbvm_context_allocatePointerTuple(context, gcFrame.type, 0);
        sysbvm_tuple_markDummyValue(coercedDummyValue);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return coercedDummyValue;
    }
    
    sysbvm_tuple_t typeString = sysbvm_tuple_printString(context, gcFrame.type);
    fprintf(stderr, "Cannot perform coercion of value into the required type: " SYSBVM_STRING_PRINTF_FORMAT "\n", SYSBVM_STRING_PRINTF_ARG(typeString));
    sysbvm_error("Cannot perform coercion of value into the required type.");
    return SYSBVM_VOID_TUPLE;
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_coerceValuePassingReferences(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t value)
{
    if(type)
        return value;
    return sysbvm_type_coerceValue(context, type, value);
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_decay(sysbvm_context_t *context, sysbvm_tuple_t type)
{
    if(sysbvm_tuple_isKindOf(context, type, context->roots.referenceType))
        return ((sysbvm_referenceType_t*)type)->super.baseType;

    return type;
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_getCanonicalPendingInstanceType(sysbvm_context_t *context, sysbvm_tuple_t type)
{
    if(sysbvm_type_isDirectSubtypeOf(type, context->roots.referenceType))
        return sysbvm_type_createReferenceType(context, context->roots.anyValueType, SYSBVM_NULL_TUPLE);
    if(sysbvm_type_isDirectSubtypeOf(type, context->roots.metatypeType))
    {
        sysbvm_tuple_t thisType = ((sysbvm_metatype_t*)type)->thisType;
        if(thisType)
            return thisType;
    }
    
    return context->roots.anyValueType;
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_canonicalizeDependentResultType(sysbvm_context_t *context, sysbvm_tuple_t type)
{
    if(!sysbvm_tuple_isDummyValue(type)) return type;

    return context->roots.anyValueType;
}

SYSBVM_API sysbvm_tuple_t sysbvm_type_getDefaultValue(sysbvm_context_t *context, sysbvm_tuple_t type)
{
    if(sysbvm_type_isNullable(type))
        return SYSBVM_NULL_TUPLE;
    
    return sysbvm_tuple_send0(context, context->roots.defaultValueSelector, type);
}

static sysbvm_tuple_t sysbvm_type_primitive_flushLookupSelector(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    //sysbvm_tuple_t *type = &arguments[0];
    sysbvm_tuple_t *selector = &arguments[1];

    for(size_t i = 0; i < GLOBAL_LOOKUP_CACHE_ENTRY_COUNT; ++i)
    {
        sysbvm_globalLookupCacheEntry_t *cacheEntry = context->roots.globalMethodLookupCache + i;
        if(cacheEntry->selector == *selector)
            memset(cacheEntry, 0, sizeof(sysbvm_globalLookupCacheEntry_t));
    }
    memset(&context->roots.inlineCaches, 0, sizeof(context->roots.inlineCaches));

    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_type_primitive_flushMacroLookupSelector(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_type_primitive_flushFallbackLookupSelector(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_type_primitive_coerceValue(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    return sysbvm_type_coerceValue(context, arguments[1], arguments[0]);
}

static sysbvm_tuple_t sysbvm_type_primitive_createSimpleFunctionType(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    return sysbvm_type_doCreateSimpleFunctionType(context, arguments[0], arguments[1], arguments[2]);
}

static sysbvm_tuple_t sysbvm_type_primitive_createPointerType(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    return sysbvm_type_doCreatePointerType(context, arguments[0], arguments[1]);
}

static sysbvm_tuple_t sysbvm_type_primitive_createReferenceType(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    return sysbvm_type_doCreateReferenceType(context, arguments[0], arguments[1]);
}

static sysbvm_tuple_t sysbvm_type_primitive_pointerLikeLoad(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_pointerLikeType_load(context, arguments[0]);
}

static sysbvm_tuple_t sysbvm_type_primitive_pointerLikeStore(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_pointerLikeType_store(context, arguments[0], arguments[1]);
}

static sysbvm_tuple_t sysbvm_void_primitive_anyValueToVoid(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)argumentCount;
    (void)arguments;
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_type_primitive_coerceASTNodeWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_tuple_t *targetType = &arguments[0];
    sysbvm_tuple_t *astNode = &arguments[1];
    sysbvm_tuple_t *environment = &arguments[2];

    (void)targetType;
    (void)environment;

    sysbvm_tuple_t sourceType = sysbvm_astNode_getAnalyzedType(*astNode);
    if(sourceType && !sysbvm_type_isDirectSubtypeOf(sourceType, context->roots.controlFlowEscapeType))
    {
        if(sysbvm_type_isDynamic(sourceType))
        {
            // Add a downcast for the untyped case.
            return sysbvm_astDownCastNode_addOntoNodeWithTargetType(context, *astNode, *targetType);
        }
        else
        {
            sysbvm_error("Cannot perform the type coercion.");
        }
    }

    return *astNode;
}

static sysbvm_tuple_t sysbvm_void_primitive_coerceASTNodeWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_tuple_t *targetType = &arguments[0];
    sysbvm_tuple_t *astNode = &arguments[1];
    sysbvm_tuple_t *environment = &arguments[2];

    if(*targetType != context->roots.voidType)
    {
        if(sysbvm_type_isDirectSubtypeOf(*targetType, context->roots.controlFlowEscapeType))
            return *astNode;

        sysbvm_error("Invalid type conversion requested.");
    }

    sysbvm_tuple_t sourcePosition = sysbvm_astNode_getSourcePosition(*astNode);
    sysbvm_tuple_t functionLiteral = sysbvm_astLiteralNode_create(context, sourcePosition, context->roots.anyValueToVoidPrimitive);

    sysbvm_tuple_t applicationArguments = sysbvm_array_create(context, 1);
    sysbvm_array_atPut(applicationArguments, 0, *astNode);        

    sysbvm_astNode_t *functionApplication = (sysbvm_astNode_t*)sysbvm_astFunctionApplicationNode_create(context, sourcePosition, functionLiteral, applicationArguments);
    functionApplication->analyzedType = *targetType;
    functionApplication->analyzerToken = sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, *environment);
    return (sysbvm_tuple_t)functionApplication;
}

void sysbvm_type_registerPrimitives(void)
{
    sysbvm_primitiveTable_registerFunction(sysbvm_type_primitive_createSimpleFunctionType, "SimpleFunctionTypeTemplate");
    sysbvm_primitiveTable_registerFunction(sysbvm_type_primitive_createPointerType, "PointerTypeTemplate");
    sysbvm_primitiveTable_registerFunction(sysbvm_type_primitive_createReferenceType, "ReferenceTypeTemplate");

    sysbvm_primitiveTable_registerFunction(sysbvm_type_primitive_pointerLikeLoad, "PointerLikeType::load");
    sysbvm_primitiveTable_registerFunction(sysbvm_type_primitive_pointerLikeStore, "PointerLikeType::store:");
    
    sysbvm_primitiveTable_registerFunction(sysbvm_void_primitive_anyValueToVoid, "Void::fromAnyValue");
    sysbvm_primitiveTable_registerFunction(sysbvm_void_primitive_coerceASTNodeWithEnvironment, "Void::coerceASTNode:withEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_type_primitive_coerceValue, "Type::coerceValue:into:");
    sysbvm_primitiveTable_registerFunction(sysbvm_type_primitive_coerceASTNodeWithEnvironment, "Type::coerceASTNode:withEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_type_primitive_flushLookupSelector, "Type::flushLookupSelector:");
    sysbvm_primitiveTable_registerFunction(sysbvm_type_primitive_flushMacroLookupSelector, "Type::flushMacroLookupSelector:");
    sysbvm_primitiveTable_registerFunction(sysbvm_type_primitive_flushFallbackLookupSelector, "Type::flushFallbackLookupSelector:");
}

void sysbvm_type_setupPrimitives(sysbvm_context_t *context)
{
    context->roots.simpleFunctionTypeTemplate = sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "SimpleFunctionTypeTemplate", 3, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_MEMOIZED | SYSBVM_FUNCTION_FLAGS_TEMPLATE, NULL, sysbvm_type_primitive_createSimpleFunctionType);
    context->roots.pointerTypeTemplate = sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "PointerTypeTemplate", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_MEMOIZED | SYSBVM_FUNCTION_FLAGS_TEMPLATE, NULL, sysbvm_type_primitive_createPointerType);
    context->roots.referenceTypeTemplate = sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "ReferenceTypeTemplate", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_MEMOIZED | SYSBVM_FUNCTION_FLAGS_TEMPLATE, NULL, sysbvm_type_primitive_createReferenceType);
    context->roots.pointerLikeLoadPrimitive = sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "PointerLikeType::load", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_type_primitive_pointerLikeLoad);
    context->roots.pointerLikeStorePrimitive = sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "PointerLikeType::store:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_type_primitive_pointerLikeStore);
    
    context->roots.anyValueToVoidPrimitive = sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "Void::fromAnyValue", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_NO_TYPECHECK_ARGUMENTS, NULL, sysbvm_void_primitive_anyValueToVoid);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Void::coerceASTNode:withEnvironment:", sysbvm_tuple_getType(context, context->roots.voidType), "coerceASTNode:withEnvironment:", 3, SYSBVM_FUNCTION_FLAGS_NONE, NULL, sysbvm_void_primitive_coerceASTNodeWithEnvironment);
    
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "Type::coerceValue:into:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_type_primitive_coerceValue);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Type::coerceASTNode:withEnvironment:", context->roots.typeType, "coerceASTNode:withEnvironment:", 3, SYSBVM_FUNCTION_FLAGS_NONE, NULL, sysbvm_type_primitive_coerceASTNodeWithEnvironment);

    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Type::flushLookupSelector:", context->roots.typeType, "flushLookupSelector:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_type_primitive_flushLookupSelector);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Type::flushMacroLookupSelector:", context->roots.typeType, "flushMacroLookupSelector:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_type_primitive_flushMacroLookupSelector);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "Type::flushFallbackLookupSelector:", context->roots.typeType, "flushFallbackLookupSelector:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_type_primitive_flushFallbackLookupSelector);

    // Export the type layout. This is used by the bootstraping algorithm for creating the accessors.
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "Type::Layout::name", sysbvm_tuple_integer_encodeSmall(SYSBVM_SLOT_INDEX_FOR_STRUCTURE_MEMBER(sysbvm_type_tuple_t, name)));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "Type::Layout::owner", sysbvm_tuple_integer_encodeSmall(SYSBVM_SLOT_INDEX_FOR_STRUCTURE_MEMBER(sysbvm_type_tuple_t, owner)));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "Type::Layout::supertype", sysbvm_tuple_integer_encodeSmall(SYSBVM_SLOT_INDEX_FOR_STRUCTURE_MEMBER(sysbvm_type_tuple_t, supertype)));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "Type::Layout::slots", sysbvm_tuple_integer_encodeSmall(SYSBVM_SLOT_INDEX_FOR_STRUCTURE_MEMBER(sysbvm_type_tuple_t, slots)));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "Type::Layout::totalSlotCount", sysbvm_tuple_integer_encodeSmall(SYSBVM_SLOT_INDEX_FOR_STRUCTURE_MEMBER(sysbvm_type_tuple_t, totalSlotCount)));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "Type::Layout::flags", sysbvm_tuple_integer_encodeSmall(SYSBVM_SLOT_INDEX_FOR_STRUCTURE_MEMBER(sysbvm_type_tuple_t, flags)));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "Type::Layout::macroMethodDictionary", sysbvm_tuple_integer_encodeSmall(SYSBVM_SLOT_INDEX_FOR_STRUCTURE_MEMBER(sysbvm_type_tuple_t, macroMethodDictionary)));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "Type::Layout::methodDictionary", sysbvm_tuple_integer_encodeSmall(SYSBVM_SLOT_INDEX_FOR_STRUCTURE_MEMBER(sysbvm_type_tuple_t, methodDictionary)));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "Type::Layout::fallbackMethodDictionary", sysbvm_tuple_integer_encodeSmall(SYSBVM_SLOT_INDEX_FOR_STRUCTURE_MEMBER(sysbvm_type_tuple_t, fallbackMethodDictionary)));

    // Export the type slot layout. This is used by the bootstraping algorithm for creating the accessors.
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeSlot::Layout::name", sysbvm_tuple_integer_encodeSmall(SYSBVM_SLOT_INDEX_FOR_STRUCTURE_MEMBER(sysbvm_typeSlot_t, name)));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeSlot::Layout::flags", sysbvm_tuple_integer_encodeSmall(SYSBVM_SLOT_INDEX_FOR_STRUCTURE_MEMBER(sysbvm_typeSlot_t, flags)));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeSlot::Layout::type", sysbvm_tuple_integer_encodeSmall(SYSBVM_SLOT_INDEX_FOR_STRUCTURE_MEMBER(sysbvm_typeSlot_t, type)));

    // Export the type flags.
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::None", sysbvm_tuple_bitflags_encode(SYSBVM_TYPE_FLAGS_NONE));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::Nullable", sysbvm_tuple_bitflags_encode(SYSBVM_TYPE_FLAGS_NULLABLE));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::Bytes", sysbvm_tuple_bitflags_encode(SYSBVM_TYPE_FLAGS_BYTES));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::Immediate", sysbvm_tuple_bitflags_encode(SYSBVM_TYPE_FLAGS_IMMEDIATE));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::Weak", sysbvm_tuple_bitflags_encode(SYSBVM_TYPE_FLAGS_WEAK));

    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::Final", sysbvm_tuple_bitflags_encode(SYSBVM_TYPE_FLAGS_FINAL));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::Abstract", sysbvm_tuple_bitflags_encode(SYSBVM_TYPE_FLAGS_ABSTRACT));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::Dynamic", sysbvm_tuple_bitflags_encode(SYSBVM_TYPE_FLAGS_DYNAMIC));

    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::PointerValue", sysbvm_tuple_bitflags_encode(SYSBVM_TYPE_FLAGS_POINTER_VALUE));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::ReferenceValue", sysbvm_tuple_bitflags_encode(SYSBVM_TYPE_FLAGS_REFERENCE_VALUE));

    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::PointerLikeValue", sysbvm_tuple_bitflags_encode(SYSBVM_TYPE_FLAGS_POINTER_LIKE_VALUE));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::ClassDefaultFlags", sysbvm_tuple_bitflags_encode(SYSBVM_TYPE_FLAGS_CLASS_DEFAULT_FLAGS));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::MetatypeRequiredFlags", sysbvm_tuple_bitflags_encode(SYSBVM_TYPE_FLAGS_METATYPE_REQUIRED_FLAGS));

    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::PointerTypeFlags", sysbvm_tuple_bitflags_encode(SYSBVM_TYPE_FLAGS_POINTER_TYPE_FLAGS));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::ReferenceTypeFlags", sysbvm_tuple_bitflags_encode(SYSBVM_TYPE_FLAGS_REFERENCE_TYPE_FLAGS));

    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::PrimitiveValueTypeDefaultFlags", sysbvm_tuple_bitflags_encode(SYSBVM_TYPE_FLAGS_PRIMITIVE_VALUE_TYPE_DEFAULT_FLAGS));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeFlags::PrimitiveValueMetatypeDefaultFlags", sysbvm_tuple_bitflags_encode(SYSBVM_TYPE_FLAGS_PRIMITIVE_VALUE_TYPE_METATYPE_FLAGS));

    // Export the type slot flags.
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeSlotFlags::None", sysbvm_tuple_bitflags_encode(SYSBVM_TYPE_SLOT_FLAG_NONE));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeSlotFlags::Public", sysbvm_tuple_bitflags_encode(SYSBVM_TYPE_SLOT_FLAG_PUBLIC));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "TypeSlotFlags::ReadOnly", sysbvm_tuple_bitflags_encode(SYSBVM_TYPE_SLOT_FLAG_READONLY));
}