#include "internal/context.h"
#include "sysbvm/type.h"
#include "sysbvm/array.h"
#include "sysbvm/orderedCollection.h"
#include "sysbvm/environment.h"
#include "sysbvm/gc.h"
#include "sysbvm/stackFrame.h"
#include "sysbvm/string.h"
#include "sysbvm/set.h"
#include "sysbvm/function.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static bool sysbvm_context_default_jitEnabled = true;

extern void sysbvm_array_registerPrimitives(void);
extern void sysbvm_orderedCollection_registerPrimitives(void);
extern void sysbvm_astInterpreter_registerPrimitives(void);
extern void sysbvm_boolean_registerPrimitives(void);
extern void sysbvm_bytecode_registerPrimitives(void);
extern void sysbvm_bytecodeCompiler_registerPrimitives();
extern void sysbvm_dictionary_registerPrimitives(void);
extern void sysbvm_errors_registerPrimitives(void);
extern void sysbvm_environment_registerPrimitives(void);
extern void sysbvm_filesystem_registerPrimitives(void);
extern void sysbvm_float_registerPrimitives(void);
extern void sysbvm_function_registerPrimitives(void);
extern void sysbvm_integer_registerPrimitives(void);
extern void sysbvm_io_registerPrimitives(void);
extern void sysbvm_primitiveInteger_registerPrimitives(void);
extern void sysbvm_string_registerPrimitives(void);
extern void sysbvm_stringStream_registerPrimitives(void);
extern void sysbvm_time_registerPrimitives(void);
extern void sysbvm_tuple_registerPrimitives(void);
extern void sysbvm_type_registerPrimitives(void);

extern void sysbvm_array_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_orderedCollection_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_astInterpreter_setupASTInterpreter(sysbvm_context_t *context);
extern void sysbvm_boolean_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_bytecode_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_bytecodeCompiler_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_dictionary_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_errors_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_environment_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_filesystem_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_float_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_function_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_integer_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_io_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_primitiveInteger_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_string_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_stringStream_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_time_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_tuple_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_type_setupPrimitives(sysbvm_context_t *context);

void sysbvm_context_registerPrimitives(void)
{
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_identityEquals, "==");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_identityNotEquals, "~~");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_identityHash, "identityHash");
    sysbvm_primitiveTable_registerFunction(sysbvm_string_primitive_equals, "String::=");
    sysbvm_primitiveTable_registerFunction(sysbvm_string_primitive_hash, "String::hash");

    sysbvm_array_registerPrimitives();
    sysbvm_orderedCollection_registerPrimitives();
    sysbvm_astInterpreter_registerPrimitives();
    sysbvm_boolean_registerPrimitives();
    sysbvm_bytecode_registerPrimitives();
    sysbvm_bytecodeCompiler_registerPrimitives();
    sysbvm_dictionary_registerPrimitives();
    sysbvm_errors_registerPrimitives();
    sysbvm_environment_registerPrimitives();
    sysbvm_filesystem_registerPrimitives();
    sysbvm_float_registerPrimitives();
    sysbvm_function_registerPrimitives();
    sysbvm_integer_registerPrimitives();
    sysbvm_io_registerPrimitives();
    sysbvm_primitiveInteger_registerPrimitives();
    sysbvm_string_registerPrimitives();
    sysbvm_stringStream_registerPrimitives();
    sysbvm_time_registerPrimitives();
    sysbvm_tuple_registerPrimitives();
    sysbvm_type_registerPrimitives();
}

SYSBVM_API sysbvm_tuple_t sysbvm_context_createIntrinsicClass(sysbvm_context_t *context, const char *name, sysbvm_tuple_t supertype, ...)
{
    sysbvm_tuple_t nameSymbol = sysbvm_symbol_internWithCString(context, name);
    sysbvm_tuple_t type = sysbvm_type_createAnonymousClassAndMetaclass(context, supertype);
    sysbvm_type_setName(type, nameSymbol);
    sysbvm_environment_setNewSymbolBindingWithValue(context, context->roots.globalNamespace, nameSymbol, type);
    sysbvm_orderedCollection_add(context, context->roots.intrinsicTypes, type);

    // First pass: count the arguments.
    size_t slotNameCount = 0;
    va_list valist;
    va_start(valist, supertype);
    while(va_arg(valist, const char *))
    {
        va_arg(valist, sysbvm_typeSlotFlags_t);
        va_arg(valist, int);
        ++slotNameCount;
    }
    va_end(valist);

    // Second pass: make the argument list.
    size_t supertypeTotalSlotCount = 0;
    sysbvm_tuple_t actualSupertype = sysbvm_type_getSupertype(type);
    if(sysbvm_tuple_isNonNullPointer(actualSupertype))
        supertypeTotalSlotCount = sysbvm_type_getTotalSlotCount(actualSupertype);

    sysbvm_tuple_t slots = sysbvm_array_create(context, slotNameCount);
    va_start(valist, supertype);
    for(size_t i = 0; i < slotNameCount; ++i)
    {
        sysbvm_tuple_t name = sysbvm_symbol_internWithCString(context, va_arg(valist, const char *));
        sysbvm_tuple_t flags = sysbvm_tuple_integer_encodeSmall(va_arg(valist, int));
        sysbvm_tuple_t type = va_arg(valist, sysbvm_tuple_t);
        if(!type)
            type = context->roots.anyValueType;
        sysbvm_array_atPut(slots, i, sysbvm_typeSlot_create(context, name, flags, type, i, supertypeTotalSlotCount + i));
    }

    va_end(valist);
    sysbvm_type_setSlots(type, slots);
    sysbvm_type_setTotalSlotCount(context, type, supertypeTotalSlotCount + slotNameCount);
    sysbvm_type_buildSlotDictionary(context, type);

    return type;
}

SYSBVM_API sysbvm_tuple_t sysbvm_context_createIntrinsicPrimitiveValueType(sysbvm_context_t *context, const char *name, sysbvm_tuple_t supertype, size_t instanceSize, size_t instanceAlignment)
{
    sysbvm_tuple_t nameSymbol = sysbvm_symbol_internWithCString(context, name);
    sysbvm_tuple_t type = sysbvm_type_createAnonymousPrimitiveValueTypeAndValueMetatype(context, supertype);
    sysbvm_type_setName(type, nameSymbol);
    sysbvm_environment_setNewSymbolBindingWithValue(context, context->roots.globalNamespace, nameSymbol, type);
    sysbvm_orderedCollection_add(context, context->roots.intrinsicTypes, type);

    sysbvm_type_setSlots(type, sysbvm_array_create(context, 0));
    sysbvm_type_setTotalSlotCount(context, type, 0);
    sysbvm_type_setInstanceSizeAndAlignment(context, type, instanceSize, instanceAlignment);
    sysbvm_type_buildSlotDictionary(context, type);
    return type;
}

SYSBVM_API sysbvm_tuple_t sysbvm_context_createIntrinsicType(sysbvm_context_t *context, const char *name, sysbvm_tuple_t supertype, ...)
{
    sysbvm_tuple_t nameSymbol = sysbvm_symbol_internWithCString(context, name);
    sysbvm_tuple_t type = sysbvm_type_createWithName(context, nameSymbol);
    if(supertype)
        sysbvm_type_setSupertype(type, supertype);
    sysbvm_environment_setNewSymbolBindingWithValue(context, context->roots.globalNamespace, nameSymbol, type);
    sysbvm_orderedCollection_add(context, context->roots.intrinsicTypes, type);

    // First pass: count the arguments.
    size_t slotNameCount = 0;
    va_list valist;
    va_start(valist, supertype);
    while(va_arg(valist, const char *))
    {
        va_arg(valist, sysbvm_typeSlotFlags_t);
        va_arg(valist, int);
        ++slotNameCount;
    }
    va_end(valist);

    // Second pass: make the argument list.
    size_t supertypeTotalSlotCount = 0;
    sysbvm_tuple_t actualSupertype = sysbvm_type_getSupertype(type);
    if(sysbvm_tuple_isNonNullPointer(actualSupertype))
        supertypeTotalSlotCount = sysbvm_type_getTotalSlotCount(actualSupertype);

    sysbvm_tuple_t slots = sysbvm_array_create(context, slotNameCount);
    va_start(valist, supertype);
    for(size_t i = 0; i < slotNameCount; ++i)
    {
        sysbvm_tuple_t name = sysbvm_symbol_internWithCString(context, va_arg(valist, const char *));
        sysbvm_tuple_t flags = sysbvm_tuple_integer_encodeSmall(va_arg(valist, int));
        sysbvm_tuple_t type = va_arg(valist, sysbvm_tuple_t);
        if(!type)
            type = context->roots.anyValueType;
        sysbvm_array_atPut(slots, i, sysbvm_typeSlot_create(context, name, flags, type, i, supertypeTotalSlotCount + i));
    }

    va_end(valist);
    sysbvm_type_setSlots(type, slots);
    sysbvm_type_setTotalSlotCount(context, type, supertypeTotalSlotCount + slotNameCount);
    sysbvm_type_buildSlotDictionary(context, type);

    return type;
}

static void sysbvm_context_setIntrinsicTypeMetadata(sysbvm_context_t *context, sysbvm_tuple_t type, const char *name, sysbvm_tuple_t supertype, ...)
{
    sysbvm_tuple_t nameSymbol = sysbvm_symbol_internWithCString(context, name);
    sysbvm_type_setName(type, nameSymbol);
    if(supertype)
        sysbvm_type_setSupertype(type, supertype);
    sysbvm_environment_setNewSymbolBindingWithValue(context, context->roots.globalNamespace, nameSymbol, type);
    sysbvm_orderedCollection_add(context, context->roots.intrinsicTypes, type);

    // First pass: count the arguments.
    size_t slotNameCount = 0;
    va_list valist;
    va_start(valist, supertype);
    while(va_arg(valist, const char *))
    {
        va_arg(valist, sysbvm_typeSlotFlags_t);
        va_arg(valist, int);
        ++slotNameCount;
    }
    va_end(valist);

    // Second pass: make the argument list.
    size_t supertypeTotalSlotCount = 0;
    sysbvm_tuple_t actualSupertype = sysbvm_type_getSupertype(type);
    if(sysbvm_tuple_isNonNullPointer(actualSupertype))
        supertypeTotalSlotCount = sysbvm_type_getTotalSlotCount(actualSupertype);

    sysbvm_tuple_t slots = sysbvm_array_create(context, slotNameCount);
    va_start(valist, supertype);
    for(size_t i = 0; i < slotNameCount; ++i)
    {
        sysbvm_tuple_t name = sysbvm_symbol_internWithCString(context, va_arg(valist, const char *));
        sysbvm_tuple_t flags = sysbvm_tuple_integer_encodeSmall(va_arg(valist, int));
        sysbvm_tuple_t type = va_arg(valist, sysbvm_tuple_t);
        if(!type)
            type = context->roots.anyValueType;
        sysbvm_array_atPut(slots, i, sysbvm_typeSlot_create(context, name, flags, type, i, supertypeTotalSlotCount + i));
    }

    va_end(valist);
    sysbvm_type_setSlots(type, slots);
    sysbvm_type_setTotalSlotCount(context, type, supertypeTotalSlotCount + slotNameCount);
    sysbvm_type_buildSlotDictionary(context, type);
}

SYSBVM_API void sysbvm_context_setIntrinsicSymbolBindingValue(sysbvm_context_t *context, sysbvm_tuple_t symbol, sysbvm_tuple_t value)
{
    sysbvm_environment_setNewSymbolBindingWithValue(context, context->roots.globalNamespace, symbol, value);

    if(sysbvm_tuple_isFunction(context, value))
    {
        sysbvm_function_t *functionObject = (sysbvm_function_t*)value;
        if(!functionObject->owner && !functionObject->name)
            functionObject->name = symbol;
    }
}

SYSBVM_API void sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(sysbvm_context_t *context, const char *symbolName, sysbvm_tuple_t binding)
{
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, symbolName), binding);
}

SYSBVM_API sysbvm_tuple_t sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(sysbvm_context_t *context, const char *symbolString, size_t argumentCount, sysbvm_bitflags_t flags, void *userdata, sysbvm_functionEntryPoint_t entryPoint)
{
    struct {
        sysbvm_tuple_t symbol;
        sysbvm_tuple_t primitiveFunction;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.symbol = sysbvm_symbol_internWithCString(context, symbolString);
    gcFrame.primitiveFunction = sysbvm_function_createPrimitive(context, argumentCount, flags, userdata, entryPoint);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, gcFrame.symbol, gcFrame.primitiveFunction);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.primitiveFunction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(sysbvm_context_t *context, const char *symbolString, sysbvm_tuple_t ownerClass, const char *selectorString, size_t argumentCount, sysbvm_bitflags_t flags, void *userdata, sysbvm_functionEntryPoint_t entryPoint)
{
    struct {
        sysbvm_tuple_t symbol;
        sysbvm_tuple_t selector;
        sysbvm_tuple_t primitiveFunction;
        sysbvm_tuple_t ownerClass;
    } gcFrame = {
        .ownerClass = ownerClass
    };

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.primitiveFunction = sysbvm_function_createPrimitive(context, argumentCount, flags, userdata, entryPoint);
    if(selectorString)
    {
        gcFrame.selector = sysbvm_symbol_internWithCString(context, selectorString);
        sysbvm_type_setMethodWithSelector(context, gcFrame.ownerClass, gcFrame.selector, gcFrame.primitiveFunction);
    }

    if(symbolString)
    {
        gcFrame.symbol = sysbvm_symbol_internWithCString(context, symbolString);
        sysbvm_context_setIntrinsicSymbolBindingValue(context, gcFrame.symbol, gcFrame.primitiveFunction);
    }

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.primitiveFunction;
}

static void sysbvm_context_createBasicTypes(sysbvm_context_t *context)
{
    // Make a circular base type.
    context->roots.untypedType = sysbvm_type_createAnonymousAndMetatype(context);
    context->roots.anyValueType = sysbvm_type_createAnonymousAndMetatype(context);
    context->roots.typeType = sysbvm_type_createAnonymous(context);
    sysbvm_tuple_setType((sysbvm_object_tuple_t*)context->roots.typeType, context->roots.typeType);
    sysbvm_type_setSupertype(context->roots.anyValueType, context->roots.untypedType);

    sysbvm_type_setFlags(context, context->roots.anyValueType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_DYNAMIC);
    sysbvm_type_setFlags(context, context->roots.untypedType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_DYNAMIC);
    sysbvm_type_setFlags(context, context->roots.typeType, SYSBVM_TYPE_FLAGS_NULLABLE);

    context->roots.objectType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.anyValueType);
    context->roots.programEntityType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    sysbvm_type_setSupertype(context->roots.typeType, context->roots.programEntityType);
    sysbvm_type_setSupertype(sysbvm_tuple_getType(context, context->roots.programEntityType), sysbvm_tuple_getType(context, context->roots.objectType));

    context->roots.metatypeType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.typeType);

    sysbvm_tuple_setType((sysbvm_object_tuple_t*)sysbvm_tuple_getType(context, context->roots.untypedType), context->roots.metatypeType);
    sysbvm_type_setSupertype(sysbvm_tuple_getType(context, context->roots.untypedType), context->roots.typeType);

    sysbvm_tuple_setType((sysbvm_object_tuple_t*)sysbvm_tuple_getType(context, context->roots.anyValueType), context->roots.metatypeType);
    sysbvm_type_setSupertype(sysbvm_tuple_getType(context, context->roots.anyValueType), context->roots.typeType);

    context->roots.classType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.typeType);
    context->roots.metaclassType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.metatypeType);

    sysbvm_tuple_setType((sysbvm_object_tuple_t*)sysbvm_tuple_getType(context, context->roots.objectType), context->roots.metaclassType);
    sysbvm_type_setSupertype(sysbvm_tuple_getType(context, context->roots.objectType), context->roots.classType);

    sysbvm_tuple_setType((sysbvm_object_tuple_t*)sysbvm_tuple_getType(context, context->roots.programEntityType), context->roots.metaclassType);

    sysbvm_tuple_setType((sysbvm_object_tuple_t*)sysbvm_tuple_getType(context, context->roots.metatypeType), context->roots.metaclassType);

    sysbvm_tuple_setType((sysbvm_object_tuple_t*)sysbvm_tuple_getType(context, context->roots.classType), context->roots.metaclassType);

    sysbvm_tuple_setType((sysbvm_object_tuple_t*)sysbvm_tuple_getType(context, context->roots.metaclassType), context->roots.metaclassType);

    // Create the type slot class.
    context->roots.typeSlotType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.typeSlotType);

    // Create the function class.
    context->roots.functionType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    sysbvm_type_setFlags(context, context->roots.functionType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_FUNCTION);
    context->roots.functionDefinitionType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    context->roots.functionBytecodeType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);

    // Create the function type classes.
    context->roots.functionTypeType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.typeType);
    context->roots.dependentFunctionTypeType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.functionTypeType);
    context->roots.simpleFunctionTypeType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.functionTypeType);

    context->roots.symbolBindingType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    context->roots.symbolAnalysisBindingType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.symbolBindingType);
    context->roots.symbolArgumentBindingType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.symbolAnalysisBindingType);
    context->roots.symbolCaptureBindingType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.symbolAnalysisBindingType);
    context->roots.symbolLocalBindingType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.symbolAnalysisBindingType);
    context->roots.symbolMacroValueBindingType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.symbolBindingType);
    context->roots.symbolTupleSlotBindingType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.symbolAnalysisBindingType);
    context->roots.symbolValueBindingType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.symbolBindingType);
    context->roots.environmentType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.programEntityType);
    context->roots.namespaceType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.environmentType);
    context->roots.astNodeType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);

    // Collection base hierarchy
    context->roots.collectionType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    context->roots.hashedCollectionType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.collectionType);
    context->roots.sequenceableCollectionType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.collectionType);
    context->roots.arrayedCollectionType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.sequenceableCollectionType);

    // Create the basic hash functions.
    context->roots.identityEqualsFunction = sysbvm_function_createPrimitive(context, 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_tuple_primitive_identityEquals);
    context->roots.identityNotEqualsFunction = sysbvm_function_createPrimitive(context, 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_tuple_primitive_identityNotEquals);
    context->roots.identityHashFunction = sysbvm_function_createPrimitive(context, 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_tuple_primitive_identityHash);
    context->roots.stringEqualsFunction = sysbvm_function_createPrimitive(context, 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_string_primitive_equals);
    context->roots.stringHashFunction = sysbvm_function_createPrimitive(context, 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_string_primitive_hash);

    context->roots.symbolType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.arrayedCollectionType);
    context->roots.stringSymbolType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.symbolType);
    sysbvm_type_setFlags(context, context->roots.stringSymbolType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_BYTES);

    context->roots.dictionaryType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.hashedCollectionType);
    context->roots.weakKeyDictionaryType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.dictionaryType);
    context->roots.weakValueDictionaryType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.dictionaryType);
    context->roots.identityDictionaryType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.dictionaryType);
    context->roots.methodDictionaryType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.hashedCollectionType);

    context->roots.setType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.hashedCollectionType);
    context->roots.identitySetType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.setType);
    context->roots.weakSetType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.setType);
    context->roots.weakIdentitySetType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.weakSetType);

    context->roots.arrayType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.arrayedCollectionType);
    context->roots.orderedCollectionType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.sequenceableCollectionType);
    context->roots.weakArrayType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.arrayType);
    context->roots.weakOrderedCollectionType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.orderedCollectionType);

    context->roots.internedSymbolSet = sysbvm_identitySet_create(context);
    context->roots.sessionToken = sysbvm_tuple_systemHandle_encode(context, 1);

    // Create the intrinsic built-in environment
    context->roots.globalNamespace = sysbvm_environment_create(context, SYSBVM_NULL_TUPLE);
    context->roots.intrinsicTypes = sysbvm_orderedCollection_create(context);

    context->roots.equalsSelector = sysbvm_symbol_internWithCString(context, "=");
    context->roots.hashSelector = sysbvm_symbol_internWithCString(context, "hash");
    context->roots.asStringSelector = sysbvm_symbol_internWithCString(context, "asString");
    context->roots.printStringSelector = sysbvm_symbol_internWithCString(context, "printString");
    context->roots.doesNotUnderstandSelector = sysbvm_symbol_internWithCString(context, "doesNotUnderstand:");

    context->roots.loadFromAtOffsetWithTypeSelector = sysbvm_symbol_internWithCString(context, "loadFrom:atOffset:withType:");
    context->roots.storeInAtOffsetWithTypeSelector = sysbvm_symbol_internWithCString(context, "store:in:atOffset:withType:");
    context->roots.assignmentSelector = sysbvm_symbol_internWithCString(context, ":=");
    context->roots.underscoreSelector = sysbvm_symbol_internWithCString(context, "_");

    context->roots.astNodeAnalysisSelector = sysbvm_symbol_internWithCString(context, "analyzeWithEnvironment:");
    context->roots.astNodeEvaluationSelector = sysbvm_symbol_internWithCString(context, "evaluateWithEnvironment:");
    context->roots.astNodeAnalysisAndEvaluationSelector = sysbvm_symbol_internWithCString(context, "analyzeAndEvaluateWithEnvironment:");
    context->roots.astNodeValidateThenAnalyzeAndEvaluateWithEnvironmentSelector = sysbvm_symbol_internWithCString(context, "validateThenAnalyzeAndEvaluateWithEnvironment:");
    context->roots.astNodeCompileIntoBytecodeSelector = sysbvm_symbol_internWithCString(context, "compileIntoBytecodeWith:");
    context->roots.ensureAnalysisSelector = sysbvm_symbol_internWithCString(context, "ensureAnalysis");
    
    context->roots.analyzeAndEvaluateMessageSendNodeForReceiverWithEnvironmentSelector = sysbvm_symbol_internWithCString(context, "analyzeAndEvaluateMessageSendNode:forReceiver:withEnvironment:");
    context->roots.analyzeMessageSendNodeWithEnvironmentSelector = sysbvm_symbol_internWithCString(context, "analyzeMessageSendNode:withEnvironment:");
    context->roots.analyzeAndEvaluateMessageChainNodeForReceiverWithEnvironmentSelector = sysbvm_symbol_internWithCString(context, "analyzeAndEvaluateMessageChainNode:forReceiver:withEnvironment:");
    context->roots.analyzeMessageChainNodeWithEnvironmentSelector = sysbvm_symbol_internWithCString(context, "analyzeMessageChainNode:withEnvironment:");
    context->roots.analyzeAndEvaluateConcreteMetaValueWithEnvironmentSelector = sysbvm_symbol_internWithCString(context, "analyzeAndEvaluateConcreteMetaValue:withEnvironment:");
    context->roots.analyzeConcreteMetaValueWithEnvironmentSelector = sysbvm_symbol_internWithCString(context, "analyzeConcreteMetaValue:withEnvironment:");

    context->roots.coerceASTNodeWithEnvironmentSelector = sysbvm_symbol_internWithCString(context, "coerceASTNode:withEnvironment:");
    context->roots.analyzeAndTypeCheckFunctionApplicationNodeWithEnvironmentSelector = sysbvm_symbol_internWithCString(context, "analyzeAndTypeCheckFunctionApplicationNode:withEnvironment:");
    context->roots.analyzeAndTypeCheckMessageSendNodeWithEnvironmentSelector = sysbvm_symbol_internWithCString(context, "analyzeAndTypeCheckMessageSendNode:withEnvironment:");
    context->roots.getOrCreateDependentApplicationValueForNodeSelector = sysbvm_symbol_internWithCString(context, "getOrCreateDependentApplicationValueForNode:");

    context->roots.applyWithoutArgumentsSelector = sysbvm_symbol_internWithCString(context, "()");
    context->roots.applyWithArgumentsSelector = sysbvm_symbol_internWithCString(context, "():");

    context->roots.primitiveNamedSelector = sysbvm_symbol_internWithCString(context, "primitive:");

    context->roots.coerceValueSelector = sysbvm_symbol_internWithCString(context, "coerceValue:");
    context->roots.defaultValueSelector = sysbvm_symbol_internWithCString(context, "defaultValue");

    context->roots.anyValueToVoidPrimitiveName = sysbvm_symbol_internWithCString(context, "Void::fromAnyValue");
    context->roots.pointerLikeLoadPrimitiveName = sysbvm_symbol_internWithCString(context, "PointerLikeType::load");
    context->roots.pointerLikeStorePrimitiveName = sysbvm_symbol_internWithCString(context, "PointerLikeType::store:");

    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "nil"), SYSBVM_NULL_TUPLE);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "false"), SYSBVM_FALSE_TUPLE);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "true"), SYSBVM_TRUE_TUPLE);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "void"), SYSBVM_VOID_TUPLE);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "__hashtableEmptyElement__"), SYSBVM_HASHTABLE_EMPTY_ELEMENT_TUPLE);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "__tombstone__"), SYSBVM_TOMBSTONE_TUPLE);

    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "__Global__"), context->roots.globalNamespace);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "BootstrapEnv::IntrinsicTypes"), context->roots.intrinsicTypes);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "BootstrapEnv::InternedSymbolSet"), context->roots.internedSymbolSet);

    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "identityHash"), context->roots.identityHashFunction);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "=="), context->roots.identityEqualsFunction);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "~~"), context->roots.identityNotEqualsFunction);

    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "String::hash"), context->roots.stringHashFunction);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "String::equals:"), context->roots.stringEqualsFunction);

    // Some basic method
    sysbvm_type_setHashFunction(context, context->roots.anyValueType, context->roots.identityHashFunction);
    sysbvm_type_setEqualsFunction(context, context->roots.anyValueType, context->roots.identityEqualsFunction);
    
    sysbvm_type_setMethodWithSelector(context, context->roots.anyValueType, sysbvm_symbol_internWithCString(context, "identityHash"), context->roots.identityHashFunction);
    sysbvm_type_setMethodWithSelector(context, context->roots.anyValueType, sysbvm_symbol_internWithCString(context, "=="), context->roots.identityEqualsFunction);
    sysbvm_type_setMethodWithSelector(context, context->roots.anyValueType, sysbvm_symbol_internWithCString(context, "~~"), context->roots.identityNotEqualsFunction);


    // Create the value type classes.
    context->roots.valueType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.typeType);
    context->roots.valueMetatypeType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.metatypeType);
    context->roots.primitiveValueType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.valueType);
    
    context->roots.anyPointerType = sysbvm_type_createAnonymous(context);
    context->roots.anyReferenceType = sysbvm_type_createAnonymous(context);
    sysbvm_type_setSupertype(context->roots.anyReferenceType, context->roots.untypedType);

    context->roots.pointerLikeType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.valueType);
    context->roots.pointerType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.pointerLikeType);
    context->roots.referenceType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.pointerLikeType);

    // Some basic types
    context->roots.voidType = sysbvm_context_createIntrinsicPrimitiveValueType(context, "Void", context->roots.anyValueType, 0, 1);

    context->roots.primitiveNumberType = sysbvm_context_createIntrinsicPrimitiveValueType(context, "PrimitiveNumber", context->roots.anyValueType, 0, 1);
    context->roots.primitiveIntegerType = sysbvm_context_createIntrinsicPrimitiveValueType(context, "PrimitiveInteger", context->roots.primitiveNumberType, 0, 1);
    context->roots.primitiveCharacterType = sysbvm_context_createIntrinsicPrimitiveValueType(context, "PrimitiveCharacter", context->roots.primitiveIntegerType, 0, 1);
    context->roots.primitiveUnsignedIntegerType = sysbvm_context_createIntrinsicPrimitiveValueType(context, "PrimitiveUnsignedInteger", context->roots.primitiveIntegerType, 0, 1);
    context->roots.primitiveSignedIntegerType = sysbvm_context_createIntrinsicPrimitiveValueType(context, "PrimitiveSignedInteger", context->roots.primitiveIntegerType, 0, 1);
    context->roots.primitiveFloatType = sysbvm_context_createIntrinsicPrimitiveValueType(context, "PrimitiveFloat", context->roots.primitiveNumberType, 0, 1);

    context->roots.char8Type = sysbvm_context_createIntrinsicPrimitiveValueType(context, "Char8", context->roots.primitiveCharacterType, 1, 1);
    context->roots.uint8Type = sysbvm_context_createIntrinsicPrimitiveValueType(context, "UInt8", context->roots.primitiveUnsignedIntegerType, 1, 1);
    context->roots.int8Type = sysbvm_context_createIntrinsicPrimitiveValueType(context, "Int8", context->roots.primitiveSignedIntegerType, 1, 1);

    context->roots.char16Type = sysbvm_context_createIntrinsicPrimitiveValueType(context, "Char16", context->roots.primitiveCharacterType, 2, 2);
    context->roots.uint16Type = sysbvm_context_createIntrinsicPrimitiveValueType(context, "UInt16", context->roots.primitiveUnsignedIntegerType, 2, 2);
    context->roots.int16Type = sysbvm_context_createIntrinsicPrimitiveValueType(context, "Int16", context->roots.primitiveSignedIntegerType, 2, 2);

    context->roots.char32Type = sysbvm_context_createIntrinsicPrimitiveValueType(context, "Char32", context->roots.primitiveCharacterType, 4, 4);
    context->roots.uint32Type = sysbvm_context_createIntrinsicPrimitiveValueType(context, "UInt32", context->roots.primitiveUnsignedIntegerType, 4, 4);
    context->roots.int32Type = sysbvm_context_createIntrinsicPrimitiveValueType(context, "Int32", context->roots.primitiveSignedIntegerType, 4, 4);

    context->roots.uint64Type = sysbvm_context_createIntrinsicPrimitiveValueType(context, "UInt64", context->roots.primitiveUnsignedIntegerType, 8, 8);
    context->roots.int64Type = sysbvm_context_createIntrinsicPrimitiveValueType(context, "Int64", context->roots.primitiveSignedIntegerType, 8, 8);

    context->roots.float32Type = sysbvm_context_createIntrinsicPrimitiveValueType(context, "Float32", context->roots.primitiveFloatType, 4, 4);
    context->roots.float64Type = sysbvm_context_createIntrinsicPrimitiveValueType(context, "Float64", context->roots.primitiveFloatType, 8, 8);
    
    context->roots.bitflagsType = sizeof(sysbvm_bitflags_t) == 4 ? context->roots.uint32Type : context->roots.uint64Type;
    context->roots.systemHandleType = sizeof(sysbvm_systemHandle_t) == 4 ? context->roots.int32Type : context->roots.int64Type;

    context->roots.sizeType = context->targetWordSize == 4 ? context->roots.uint32Type : context->roots.uint64Type;
    context->roots.uintptrType = context->targetWordSize == 4 ? context->roots.uint32Type : context->roots.uint64Type;
    context->roots.intptrType = context->targetWordSize == 4 ? context->roots.int32Type : context->roots.int64Type;

    context->roots.booleanType = sysbvm_context_createIntrinsicPrimitiveValueType(context, "Boolean", context->roots.anyValueType, 1, 1);
    context->roots.trueType = sysbvm_context_createIntrinsicPrimitiveValueType(context, "True", context->roots.booleanType, 1, 1);
    context->roots.falseType = sysbvm_context_createIntrinsicPrimitiveValueType(context, "False", context->roots.booleanType, 1, 1);

    context->roots.integerType = sysbvm_context_createIntrinsicClass(context, "Integer", SYSBVM_NULL_TUPLE, NULL);
    context->roots.positiveIntegerType = sysbvm_context_createIntrinsicClass(context, "PositiveInteger", context->roots.integerType, NULL);
    context->roots.negativeIntegerType = sysbvm_context_createIntrinsicClass(context, "NegativeInteger", context->roots.integerType, NULL);

    context->roots.undefinedObjectType = sysbvm_context_createIntrinsicClass(context, "UndefinedObject", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_typeAndMetatype_setFlags(context, context->roots.undefinedObjectType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_IMMEDIATE | SYSBVM_TYPE_FLAGS_FINAL, SYSBVM_TYPE_FLAGS_FINAL);

    context->roots.pendingMemoizationValueType = sysbvm_context_createIntrinsicClass(context, "PendingMemoizationValue", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_typeAndMetatype_setFlags(context, context->roots.pendingMemoizationValueType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_IMMEDIATE | SYSBVM_TYPE_FLAGS_FINAL, SYSBVM_TYPE_FLAGS_FINAL);

    context->roots.tombstoneType = sysbvm_context_createIntrinsicClass(context, "ObjectTombstone", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_typeAndMetatype_setFlags(context, context->roots.tombstoneType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_IMMEDIATE | SYSBVM_TYPE_FLAGS_FINAL, SYSBVM_TYPE_FLAGS_FINAL);

    context->roots.stringType = sysbvm_context_createIntrinsicClass(context, "String", context->roots.arrayedCollectionType, NULL);
    sysbvm_typeAndMetatype_setFlags(context, context->roots.stringType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_BYTES | SYSBVM_TYPE_FLAGS_FINAL, SYSBVM_TYPE_FLAGS_FINAL);

    // Special types used during semantic analysis.
    context->roots.controlFlowEscapeType = sysbvm_context_createIntrinsicPrimitiveValueType(context, "ControlFlowEscapeType", context->roots.voidType, 0, 1);
    context->roots.controlFlowBreakType = sysbvm_context_createIntrinsicPrimitiveValueType(context, "ControlFlowBreakType", context->roots.controlFlowEscapeType, 0, 1);
    context->roots.controlFlowContinueType = sysbvm_context_createIntrinsicPrimitiveValueType(context, "ControlFlowContinueType", context->roots.controlFlowEscapeType, 0, 1);
    context->roots.controlFlowReturnType = sysbvm_context_createIntrinsicPrimitiveValueType(context, "ControlFlowReturnType", context->roots.controlFlowEscapeType, 0, 1);
    context->roots.noReturnType = sysbvm_context_createIntrinsicPrimitiveValueType(context, "NoReturn", context->roots.controlFlowEscapeType, 0, 1);
    context->roots.unwindsType = sysbvm_context_createIntrinsicPrimitiveValueType(context, "Unwinds", context->roots.controlFlowEscapeType, 0, 1);

    context->roots.decayedTypeInferenceType = sysbvm_context_createIntrinsicPrimitiveValueType(context, "DecayedTypeInferenceType", SYSBVM_NULL_TUPLE, 0, 1);
    context->roots.directTypeInferenceType = sysbvm_context_createIntrinsicPrimitiveValueType(context, "DirectTypeInferenceType", SYSBVM_NULL_TUPLE, 0, 1);

    // Set the name of the root basic type.
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.untypedType, "Untyped", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.anyValueType, "AnyValue", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.objectType, "Object", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.programEntityType, "ProgramEntity", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.typeType, "Type", SYSBVM_NULL_TUPLE,
        "name", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolType,
        "owner", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        "supertype", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeType,
        "slots", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "slotsWithBasicInitialization", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "totalSlotCount", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.anyValueType,
        "instanceSize", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "instanceAlignment", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "flags", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.bitflagsType,

        "slotDictionary", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.methodDictionaryType,

        "macroMethodDictionary", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.methodDictionaryType,
        "methodDictionary", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.methodDictionaryType,
        "fallbackMethodDictionary", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.methodDictionaryType,

        "pendingSlots", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        "subtypes", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.classType, "Class", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.metatypeType, "Metatype", SYSBVM_NULL_TUPLE,
        "thisType", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.metaclassType, "Metaclass", SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.typeSlotType, "TypeSlot", SYSBVM_NULL_TUPLE,
        "name", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolType,
        "flags", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.bitflagsType,
        "type", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeType,
        "referenceType", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeType,
        "localIndex", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "index", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "offset", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "initialValueBlock", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.anyValueType,
        NULL);

    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.valueType, "ValueType", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.valueMetatypeType, "ValueMetatype", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.primitiveValueType, "PrimitiveValueType", SYSBVM_NULL_TUPLE, NULL);
    
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.anyPointerType, "AnyPointer", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.anyReferenceType, "AnyReference", SYSBVM_NULL_TUPLE, NULL);

    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.pointerLikeType, "PointerLikeType", SYSBVM_NULL_TUPLE,
        "baseType", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeType,
        "addressSpace", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        "loadValueFunction", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionType,
        "storeValueFunction", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionType,
        SYSBVM_NULL_TUPLE);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.pointerType, "PointerType", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.referenceType, "ReferenceType", SYSBVM_NULL_TUPLE, NULL);

    context->roots.analysisQueueEntryType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.valueType);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.analysisQueueEntryType, "AnalysisQueueEntry", context->roots.objectType,
        "programEntity", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.programEntityType,
        "nextEntry", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.analysisQueueEntryType,
        NULL);
    context->roots.analysisQueueType = sysbvm_context_createIntrinsicClass(context, "AnalysisQueue", context->roots.objectType,
        "firstEntry", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.analysisQueueEntryType,
        "lastEntry", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.analysisQueueEntryType,
        NULL);

    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.environmentType, "Environment", SYSBVM_NULL_TUPLE,
        "parent", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.environmentType,
        "symbolTable", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.namespaceType, "Namespace", SYSBVM_NULL_TUPLE,
        NULL);
    context->roots.analysisAndEvaluationEnvironmentType = sysbvm_context_createIntrinsicClass(context, "AnalysisAndEvaluationEnvironment", context->roots.environmentType,
        "usedTuplesWithNamedSlots", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.orderedCollectionType,
        "analyzerToken", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.objectType,
        "expectedType", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeType,
        "returnTarget", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        "breakTarget", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        "continueTarget", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        NULL);
    context->roots.analysisEnvironmentType = sysbvm_context_createIntrinsicClass(context, "AnalysisEnvironment", context->roots.analysisAndEvaluationEnvironmentType,
        NULL);
    context->roots.functionActivationEnvironmentType = sysbvm_context_createIntrinsicClass(context, "FunctionActivationEnvironment", context->roots.analysisAndEvaluationEnvironmentType,
        "function", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionType,
        "functionDefinition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionDefinitionType,
        "dependentFunctionType", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionTypeType,
        "argumentVectorSize", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "captureVector", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "valueVector", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.functionAnalysisEnvironmentType = sysbvm_context_createIntrinsicClass(context, "FunctionAnalysisEnvironment", context->roots.analysisEnvironmentType,
        "functionDefinition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionDefinitionType,
        "captureBindingTable", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.identityDictionaryType,
        "captureBindingList", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.orderedCollectionType,
        "argumentBindingList", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.orderedCollectionType,
        "localBindingList", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.orderedCollectionType,
        "innerFunctionList", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.orderedCollectionType,
        "pragmaList", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.orderedCollectionType,
        "hasBreakTarget", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "hasContinueTarget", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        NULL);
    context->roots.localAnalysisEnvironmentType = sysbvm_context_createIntrinsicClass(context, "LocalAnalysisEnvironment", context->roots.analysisEnvironmentType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolBindingType, "SymbolBinding", SYSBVM_NULL_TUPLE,
        "name", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        "sourcePosition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        "type", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolAnalysisBindingType, "SymbolAnalysisBinding", SYSBVM_NULL_TUPLE,
        "ownerFunction", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionDefinitionType,
        "vectorIndex", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolArgumentBindingType, "SymbolArgumentBinding", SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolCaptureBindingType, "SymbolCaptureBinding", SYSBVM_NULL_TUPLE,
        "sourceBinding", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolAnalysisBindingType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolTupleSlotBindingType, "SymbolTupleSlotBinding", SYSBVM_NULL_TUPLE,
        "tupleBinding", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolAnalysisBindingType,
        "typeSlot", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeSlotType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolLocalBindingType, "SymbolLocalBinding", SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolMacroValueBindingType, "SymbolMacroValueBinding", SYSBVM_NULL_TUPLE,
        "expansion", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolValueBindingType, "SymbolValueBinding", SYSBVM_NULL_TUPLE,
        "value", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.functionType, "Function", SYSBVM_NULL_TUPLE,
        "name", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolType,
        "owner", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.programEntityType,
        "flags", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.bitflagsType,
        "argumentCount", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "captureVector", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "captureEnvironment", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.environmentType,
        "definition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionDefinitionType,
        "primitiveTableIndex", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.uint32Type,
        "primitiveName", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolType,
        "nativeUserdata", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.systemHandleType,
        "nativeEntryPoint", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.systemHandleType,
        "memoizationTable", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.weakValueDictionaryType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.functionDefinitionType, "FunctionDefinition", SYSBVM_NULL_TUPLE,
        "name", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolType,
        "owner", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.programEntityType,
        "flags", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.bitflagsType,
        "argumentCount", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "sourcePosition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,

        "definitionEnvironment", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.environmentType,
        "definitionArgumentNodes", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "definitionResultTypeNode", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "definitionBodyNode", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,

        "analyzedType", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionTypeType,

        "analysisEnvironment", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.environmentType,
        "analyzedCaptures", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "analyzedArguments", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "analyzedLocals", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "analyzedPragmas", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "analyzedInnerFunctions", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "analyzedPrimitiveName", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,

        "analyzedArgumentNodes", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "analyzedResultTypeNode", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "analyzedBodyNode", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,

        "bytecode", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionBytecodeType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.functionBytecodeType, "FunctionBytecode", SYSBVM_NULL_TUPLE,
        "argumentCount", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "captureVectorSize", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "localVectorSize", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "literalVector", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "instructions", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.byteArrayType,

        "definition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionDefinitionType,
        "pcToDebugListTable", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "debugSourceASTNodes", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "debugSourcePositions", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "debugSourceEnvironments", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        
        "jittedCode", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.systemHandleType,
        "jittedCodeSessionToken", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.systemHandleType,

        "jittedCodeTrampoline", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.systemHandleType,
        "jittedCodeTrampolineSessionToken", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.systemHandleType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.functionTypeType, "FunctionType", SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.dependentFunctionTypeType, "DependentFunctionType", SYSBVM_NULL_TUPLE,
        "sourcePosition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        "argumentNodes", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "isVariadic", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "resultTypeNode", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,

        "environment", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.environmentType,
        "captureBindings", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "argumentBindings", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "localBindings", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.simpleFunctionTypeType, "SimpleFunctionType", SYSBVM_NULL_TUPLE,
        "argumentTypes", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "isVariadic", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "resultType", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeType,
        NULL);

    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolType, "Symbol", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.stringSymbolType, "StringSymbol", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.setType, "Set", SYSBVM_NULL_TUPLE,
        "size", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "storage", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.identitySetType, "IdentitySet", SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.weakSetType, "WeakSet", SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.weakIdentitySetType, "WeakIdentitySet", SYSBVM_NULL_TUPLE,
        NULL);

    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.collectionType, "Collection", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.hashedCollectionType, "HashedCollection", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.sequenceableCollectionType, "SequenceableCollection", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.arrayedCollectionType, "ArrayedCollection", SYSBVM_NULL_TUPLE, NULL);

    // 
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.arrayType, "Array", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.orderedCollectionType, "OrderedCollection", SYSBVM_NULL_TUPLE,
        "size", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "storage", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.weakArrayType, "WeakArray", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.weakOrderedCollectionType, "WeakOrderedCollection", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_typeAndMetatype_setFlags(context, context->roots.arrayType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_FINAL, SYSBVM_TYPE_FLAGS_FINAL);
    sysbvm_typeAndMetatype_setFlags(context, context->roots.orderedCollectionType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_FINAL, SYSBVM_TYPE_FLAGS_NONE);
    sysbvm_typeAndMetatype_setFlags(context, context->roots.weakArrayType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_FINAL | SYSBVM_TYPE_FLAGS_WEAK, SYSBVM_TYPE_FLAGS_FINAL);
    sysbvm_typeAndMetatype_setFlags(context, context->roots.weakOrderedCollectionType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_FINAL, SYSBVM_TYPE_FLAGS_FINAL);

    // Create other root basic types.
    context->roots.arraySliceType = sysbvm_context_createIntrinsicClass(context, "ArraySlice", context->roots.sequenceableCollectionType,
        "elements", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        "offset", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "size", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        NULL);
    sysbvm_typeAndMetatype_setFlags(context, context->roots.arraySliceType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_FINAL, SYSBVM_TYPE_FLAGS_FINAL);
    context->roots.associationType = sysbvm_context_createIntrinsicClass(context, "Association", SYSBVM_NULL_TUPLE,
        "key", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        "value", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        NULL);
    context->roots.weakValueAssociationType = sysbvm_context_createIntrinsicClass(context, "WeakValueAssociation", SYSBVM_NULL_TUPLE,
        "key", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        //"value", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_WEAK, SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_typeAndMetatype_setFlags(context, context->roots.associationType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_FINAL, SYSBVM_TYPE_FLAGS_FINAL);
    context->roots.byteArrayType = sysbvm_context_createIntrinsicClass(context, "ByteArray", context->roots.arrayedCollectionType, NULL);
    sysbvm_typeAndMetatype_setFlags(context, context->roots.byteArrayType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_BYTES | SYSBVM_TYPE_FLAGS_FINAL, SYSBVM_TYPE_FLAGS_FINAL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.dictionaryType, "Dictionary", SYSBVM_NULL_TUPLE,
        "size", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "storage", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.identityDictionaryType, "IdentityDictionary", SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.weakKeyDictionaryType, "WeakKeyDictionary", SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.weakValueDictionaryType, "WeakValueDictionary", SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.methodDictionaryType, "MethodDictionary", SYSBVM_NULL_TUPLE,
        "size", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "storage", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.generatedSymbolType = sysbvm_context_createIntrinsicClass(context, "GeneratedSymbol", context->roots.symbolType,
        "value", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolType,
        "sourcePosition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        NULL);
    context->roots.hashtableEmptyType = sysbvm_context_createIntrinsicClass(context, "HashtableEmpty", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_typeAndMetatype_setFlags(context, context->roots.hashtableEmptyType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_IMMEDIATE | SYSBVM_TYPE_FLAGS_FINAL, SYSBVM_TYPE_FLAGS_FINAL);

    context->roots.macroContextType = sysbvm_context_createIntrinsicClass(context, "MacroContext", SYSBVM_NULL_TUPLE,
        "sourceNode", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "sourcePosition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        NULL);
    context->roots.messageType = sysbvm_context_createIntrinsicClass(context, "Message", SYSBVM_NULL_TUPLE,
        "selector", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        "arguments", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        NULL);
    context->roots.pragmaType = sysbvm_context_createIntrinsicClass(context, "Pragma", SYSBVM_NULL_TUPLE,
        "selector", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        "arguments", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        NULL);
    context->roots.streamType = sysbvm_context_createIntrinsicClass(context, "Stream", SYSBVM_NULL_TUPLE,
        NULL);
    context->roots.stringStreamType = sysbvm_context_createIntrinsicClass(context, "StringStream", context->roots.streamType,
        "size", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "storage", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.stringType,
        NULL);
    context->roots.valueBoxType = sysbvm_context_createIntrinsicClass(context, "ValueBox", SYSBVM_NULL_TUPLE,
        "value", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.untypedType,
        NULL);
    sysbvm_typeAndMetatype_setFlags(context, context->roots.valueBoxType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_FINAL, SYSBVM_TYPE_FLAGS_FINAL);

    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "Bitflags"), context->roots.bitflagsType);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "SystemHandle"), context->roots.systemHandleType);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "Size"), context->roots.sizeType);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "UIntPointer"), context->roots.uintptrType);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "IntPointer"), context->roots.intptrType);

    context->roots.sourceCodeType = sysbvm_context_createIntrinsicClass(context, "SourceCode", SYSBVM_NULL_TUPLE,
        "text", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.stringType,
        "directory", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.stringType,
        "name", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.stringType,
        "language", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.stringSymbolType,
        "lineStartIndexTable", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        NULL);
    context->roots.sourcePositionType = sysbvm_context_createIntrinsicClass(context, "SourcePosition", SYSBVM_NULL_TUPLE,
        "sourceCode", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sourceCodeType,
        "startIndex", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "startLine", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "startColumn", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "endIndex", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "endLine", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "endColumn", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        NULL);
    context->roots.tokenType = sysbvm_context_createIntrinsicClass(context, "Token", SYSBVM_NULL_TUPLE,
        "kind", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.uint8Type,
        "sourcePosition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sourcePositionType,
        "value", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        NULL);

    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.astNodeType, "ASTNode", SYSBVM_NULL_TUPLE,
        "sourcePosition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sourcePositionType,
        "analyzerToken", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        "analyzedType", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeType,
        NULL);
    context->roots.astArgumentNodeType = sysbvm_context_createIntrinsicClass(context, "ASTArgumentNode", context->roots.astNodeType,
        "isForAll", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "name", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "type", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "binding", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        NULL);
    context->roots.astBinaryExpressionSequenceNodeType = sysbvm_context_createIntrinsicClass(context, "ASTBinaryExpressionSequenceNode", context->roots.astNodeType,
        "operands", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "operators", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.astBreakNodeType = sysbvm_context_createIntrinsicClass(context, "ASTBreakNode", context->roots.astNodeType,
        NULL);
    context->roots.astCoerceValueNodeType = sysbvm_context_createIntrinsicClass(context, "ASTCoerceValueNode", context->roots.astNodeType,
        "typeExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "valueExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);
    context->roots.astContinueNodeType = sysbvm_context_createIntrinsicClass(context, "ASTContinueNode", context->roots.astNodeType,
        NULL);
    context->roots.astDoWhileContinueWithNodeType = sysbvm_context_createIntrinsicClass(context, "ASTDoWhileContinueWithNode", context->roots.astNodeType,
        "bodyExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "conditionExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "continueExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);
    context->roots.astDownCastNodeType = sysbvm_context_createIntrinsicClass(context, "ASTDownCastNode", context->roots.astNodeType,
        "typeExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "valueExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);
    context->roots.astErrorNodeType = sysbvm_context_createIntrinsicClass(context, "ASTErrorNode", context->roots.astNodeType,
        "errorMessage", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.stringType,
        NULL);
    context->roots.astFunctionApplicationNodeType = sysbvm_context_createIntrinsicClass(context, "ASTFunctionApplicationNode", context->roots.astNodeType,
        "functionExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "arguments", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "applicationFlags", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.bitflagsType,
        NULL);
    context->roots.astLambdaNodeType = sysbvm_context_createIntrinsicClass(context, "ASTLambdaNode", context->roots.astNodeType,
        "flags", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        "name", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "arguments", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "resultType", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "body", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "hasLazyAnalysis", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.anyValueType,
        "functionDefinition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionDefinitionType,
        "binding", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        NULL);
    context->roots.astLexicalBlockNodeType = sysbvm_context_createIntrinsicClass(context, "ASTLexicalBlockNode", context->roots.astNodeType,
        "body", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "bodyEnvironment", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.environmentType,
        NULL);
    context->roots.astLiteralNodeType = sysbvm_context_createIntrinsicClass(context, "ASTLiteralNode", context->roots.astNodeType,
        "value", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        NULL);
    context->roots.astLocalDefinitionNodeType = sysbvm_context_createIntrinsicClass(context, "ASTLocalDefinitionNode", context->roots.astNodeType,
        "nameExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "typeExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "valueExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "binding", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        "isMacroSymbol", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "isMutable", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "analyzedValueType", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeType,
        NULL);
    context->roots.astIdentifierReferenceNodeType = sysbvm_context_createIntrinsicClass(context, "ASTIdentifierReferenceNode", context->roots.astNodeType,
        "value", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        "binding", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        NULL);
    context->roots.astIfNodeType = sysbvm_context_createIntrinsicClass(context, "ASTIfNode", context->roots.astNodeType,
        "conditionExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "trueExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "falseExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);

    context->roots.astMakeAssociationNodeType = sysbvm_context_createIntrinsicClass(context, "ASTMakeAssociationNode", context->roots.astNodeType,
        "key", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "value", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);
    context->roots.astMakeByteArrayNodeType = sysbvm_context_createIntrinsicClass(context, "ASTMakeByteArrayNode", context->roots.astNodeType,
        "elements", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.astMakeDictionaryNodeType = sysbvm_context_createIntrinsicClass(context, "ASTMakeDictionaryNode", context->roots.astNodeType,
        "elements", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.astMakeArrayNodeType = sysbvm_context_createIntrinsicClass(context, "ASTMakeArrayNode", context->roots.astNodeType,
        "elements", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);

    context->roots.astMessageSendNodeType = sysbvm_context_createIntrinsicClass(context, "ASTMessageSendNode", context->roots.astNodeType,
        "receiver", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "receiverLookupType", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "selector", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "arguments", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "isDynamic", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "boundMethod", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        NULL);
    context->roots.astMessageChainNodeType = sysbvm_context_createIntrinsicClass(context, "ASTMessageChainNode", context->roots.astNodeType,
        "receiver", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "receiverLookupType", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "messages", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.astMessageChainMessageNodeType = sysbvm_context_createIntrinsicClass(context, "ASTMessageChainMessageNode", context->roots.astNodeType,
        "selector", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "arguments", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.astPragmaNodeType = sysbvm_context_createIntrinsicClass(context, "ASTPragmaNode", context->roots.astNodeType,
        "selector", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "arguments", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.astReturnNodeType = sysbvm_context_createIntrinsicClass(context, "ASTReturnNode", context->roots.astNodeType,
        "expression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);
    context->roots.astSequenceNodeType = sysbvm_context_createIntrinsicClass(context, "ASTSequenceNode", context->roots.astNodeType,
        "pragmas", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "expressions", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.astTupleSlotNamedAtNodeType = sysbvm_context_createIntrinsicClass(context, "ASTTupleSlotNamedAtNode", context->roots.astNodeType,
        "tupleExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "nameExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "boundSlot", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeSlotType,
        NULL);
    context->roots.astTupleSlotNamedAtPutNodeType = sysbvm_context_createIntrinsicClass(context, "ASTTupleSlotNamedAtPutNode", context->roots.astNodeType,
        "tupleExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "nameExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "valueExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "boundSlot", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeSlotType,
        NULL);
    context->roots.astTupleSlotNamedReferenceAtNodeType = sysbvm_context_createIntrinsicClass(context, "ASTTupleSlotNamedReferenceAtNode", context->roots.astNodeType,
        "tupleExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "nameExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "boundSlot", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeSlotType,
        NULL);
    context->roots.astTupleWithLookupStartingFromNodeType = sysbvm_context_createIntrinsicClass(context, "ASTTupleWithLookupStartingFromNode", context->roots.astNodeType,
        "tupleExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "typeExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);
    context->roots.astUnexpandedApplicationNodeType = sysbvm_context_createIntrinsicClass(context, "ASTUnexpandedApplicationNode", context->roots.astNodeType,
        "functionOrMacroExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "arguments", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.astUnexpandedSExpressionNodeType = sysbvm_context_createIntrinsicClass(context, "ASTUnexpandedSExpressionNode", context->roots.astNodeType,
        "elements", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.astUseNamedSlotsOfNodeType = sysbvm_context_createIntrinsicClass(context, "ASTUseNamedSlotsOfNode", context->roots.astNodeType,
        "tupleExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "binding", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolLocalBindingType,
        NULL);
    context->roots.astWhileContinueWithNodeType = sysbvm_context_createIntrinsicClass(context, "ASTWhileContinueWithNode", context->roots.astNodeType,
        "conditionExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "bodyExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "continueExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);

    context->roots.astQuoteNodeType = sysbvm_context_createIntrinsicClass(context, "ASTQuoteNode", context->roots.astNodeType,
        "node", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);
    context->roots.astQuasiQuoteNodeType = sysbvm_context_createIntrinsicClass(context, "ASTQuasiQuoteNode", context->roots.astNodeType,
        "node", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);
    context->roots.astQuasiUnquoteNodeType = sysbvm_context_createIntrinsicClass(context, "ASTQuasiUnquoteNode", context->roots.astNodeType,
        "expression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "astTemplateParameterIndex", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        NULL);
    context->roots.astSpliceNodeType = sysbvm_context_createIntrinsicClass(context, "ASTSpliceNode", context->roots.astNodeType,
        "expression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "astTemplateParameterIndex", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        NULL);

    context->roots.bytecodeCompilerInstructionOperandType = sysbvm_context_createIntrinsicClass(context, "BootstrapBytecodeCompilerInstructionOperand", context->roots.objectType,
        NULL);
    context->roots.bytecodeCompilerInstructionType = sysbvm_context_createIntrinsicClass(context, "BootstrapBytecodeCompilerInstruction", context->roots.objectType,
        "pc", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "endPC", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "opcode", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, NULL,
        "operands", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,

        "sourcePosition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sourcePositionType,
        "sourceEnvironment", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.environmentType,
        "sourceASTNode", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,

        NULL);
    context->roots.bytecodeCompilerInstructionVectorOperandType = sysbvm_context_createIntrinsicClass(context, "BootstrapBytecodeCompilerInstructionVectorOperand", context->roots.bytecodeCompilerInstructionOperandType,
        "index", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.int16Type,
        "vectorType", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.int16Type,

        "hasAllocaDestination", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "hasNonAllocaDestination", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "hasSlotReferenceAtDestination", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "hasNonSlotReferenceAtDestination", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "hasLoadStoreUsage", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "hasNonLoadStoreUsage", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "optimizationTupleOperand", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.anyValueType,
        "optimizationTypeSlotOperand", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.anyValueType,
        NULL);
    context->roots.bytecodeCompilerType = sysbvm_context_createIntrinsicClass(context, "BootstrapBytecodeCompiler", context->roots.objectType,
        "arguments", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "captures", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "literals", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.orderedCollectionType,
        "literalDictionary", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.identityDictionaryType,
        "temporaries", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.orderedCollectionType,
        "usedTemporaryCount", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,

        "firstInstruction", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.bytecodeCompilerInstructionType,
        "lastInstruction", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.bytecodeCompilerInstructionType,

        "breakLabel", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.bytecodeCompilerInstructionOperandType,
        "continueLabel", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.bytecodeCompilerInstructionOperandType,

        "sourcePosition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sourcePositionType,
        "sourceEnvironment", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.environmentType,
        "sourceASTNode", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,

        "bindingDictionary", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.dictionaryType,
        NULL);
    
    // Fill the immediate type table.
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_NIL] = context->roots.undefinedObjectType;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_INTEGER] = context->roots.integerType;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_CHAR8] = context->roots.char8Type;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_UINT8] = context->roots.uint8Type;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_INT8] = context->roots.int8Type;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_CHAR16] = context->roots.char16Type;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_UINT16] = context->roots.uint16Type;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_INT16] = context->roots.int16Type;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_CHAR32] = context->roots.char32Type;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_UINT32] = context->roots.uint32Type;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_INT32] = context->roots.int32Type;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_UINT64] = context->roots.uint64Type;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_INT64] = context->roots.int64Type;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_FLOAT32] = context->roots.float32Type;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_FLOAT64] = context->roots.float64Type;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_TRIVIAL] = context->roots.undefinedObjectType;

    // Fill the immediate trivial type table.
    context->roots.immediateTrivialTypeTable[SYSBVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_FALSE] = context->roots.falseType;
    context->roots.immediateTrivialTypeTable[SYSBVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_TRUE] = context->roots.trueType;
    context->roots.immediateTrivialTypeTable[SYSBVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_VOID] = context->roots.voidType;
    context->roots.immediateTrivialTypeTable[SYSBVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_HASHTABLE_EMPTY_ELEMENT] = context->roots.hashtableEmptyType;
    context->roots.immediateTrivialTypeTable[SYSBVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_TOMBSTONE] = context->roots.tombstoneType;
    context->roots.immediateTrivialTypeTable[SYSBVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_PENDING_MEMOIZATION_VALUE] = context->roots.pendingMemoizationValueType;
}

SYSBVM_API sysbvm_context_t *sysbvm_context_create(void)
{
    sysbvm_context_t *context = (sysbvm_context_t*)calloc(1, sizeof(sysbvm_context_t));
    context->targetWordSize = sizeof(void*);
    context->identityHashSeed = 1;
    context->jitEnabled = sysbvm_context_default_jitEnabled;
    sysbvm_heap_initialize(&context->heap);
    sysbvm_gc_lock(context);

    sysbvm_context_createBasicTypes(context);
    
    sysbvm_array_setupPrimitives(context);
    sysbvm_orderedCollection_setupPrimitives(context);
    sysbvm_astInterpreter_setupASTInterpreter(context);
    sysbvm_boolean_setupPrimitives(context);
    sysbvm_bytecode_setupPrimitives(context);
    sysbvm_bytecodeCompiler_setupPrimitives(context);
    sysbvm_dictionary_setupPrimitives(context);
    sysbvm_errors_setupPrimitives(context);
    sysbvm_environment_setupPrimitives(context);
    sysbvm_filesystem_setupPrimitives(context);
    sysbvm_float_setupPrimitives(context);
    sysbvm_function_setupPrimitives(context);
    sysbvm_integer_setupPrimitives(context);
    sysbvm_io_setupPrimitives(context);
    sysbvm_primitiveInteger_setupPrimitives(context);
    sysbvm_string_setupPrimitives(context);
    sysbvm_stringStream_setupPrimitives(context);
    sysbvm_time_setupPrimitives(context);
    sysbvm_tuple_setupPrimitives(context);
    sysbvm_type_setupPrimitives(context);
    
    sysbvm_gc_unlock(context);

    return context;
}

SYSBVM_API void sysbvm_context_destroy(sysbvm_context_t *context)
{
    if(!context) return;

    // Destroy the context heap.
    sysbvm_heap_destroy(&context->heap);
    free(context);
}

SYSBVM_API sysbvm_context_t *sysbvm_context_loadImageFromFileNamed(const char *filename)
{
    FILE *inputFile = fopen(filename, "rb");
    if(!inputFile)
        return NULL;

    char magic[4] = {0};
    if(fread(magic, 4, 1, inputFile) != 1 || memcmp(magic, "TVIM", 4))
    {
        fclose(inputFile);
        return NULL;
    }

    sysbvm_context_t *context = (sysbvm_context_t*)calloc(1, sizeof(sysbvm_context_t));
    if(fread(&context->targetWordSize, sizeof(context->targetWordSize), 1, inputFile) != 1 ||
        fread(&context->identityHashSeed, sizeof(context->identityHashSeed), 1, inputFile) != 1 ||
        fread(&context->roots, sizeof(context->roots), 1, inputFile) != 1 ||
        !sysbvm_heap_loadFromFile(&context->heap, inputFile, sizeof(context->roots) / sizeof(sysbvm_tuple_t), (sysbvm_tuple_t*)&context->roots))
    {
        fclose(inputFile);
        free(context);
        return NULL;
    }
    
    context->jitEnabled = sysbvm_context_default_jitEnabled;

    fclose(inputFile);

    context->roots.sessionToken = sysbvm_tuple_systemHandle_encode(context, sysbvm_tuple_systemHandle_decode(context->roots.sessionToken) +  1);
    return context;
}

SYSBVM_API void sysbvm_context_saveImageToFileNamed(sysbvm_context_t *context, const char *filename)
{
    sysbvm_gc_collect(context);
    FILE *outputFile = fopen(filename, "wb");
    fwrite("TVIM", 4, 1, outputFile);
    fwrite(&context->targetWordSize, sizeof(context->targetWordSize), 1, outputFile);
    fwrite(&context->identityHashSeed, sizeof(context->identityHashSeed), 1, outputFile);
    fwrite(&context->roots, sizeof(context->roots), 1, outputFile);
    sysbvm_heap_dumpToFile(&context->heap, outputFile);
    fclose(outputFile);
}

sysbvm_heap_t *sysbvm_context_getHeap(sysbvm_context_t *context)
{
    if(!context) return 0;

    return &context->heap;
}

static size_t sysbvm_context_generateIdentityHash(sysbvm_context_t *context)
{
    context->identityHashSeed = sysbvm_hashMultiply(context->identityHashSeed) + 12345;
    return context->identityHashSeed & SYSBVM_HASH_BIT_MASK;
}

sysbvm_object_tuple_t *sysbvm_context_allocateByteTuple(sysbvm_context_t *context, sysbvm_tuple_t type, size_t byteSize)
{
    if(!context) return 0;

    sysbvm_object_tuple_t *result = sysbvm_heap_allocateByteTuple(&context->heap, byteSize);
    sysbvm_tuple_setIdentityHash(result, sysbvm_context_generateIdentityHash(context));
    if(result)
        sysbvm_tuple_setType(result, type);
    return result;
}

sysbvm_object_tuple_t *sysbvm_context_allocatePointerTuple(sysbvm_context_t *context, sysbvm_tuple_t type, size_t slotCount)
{
    if(!context) return 0;

    sysbvm_object_tuple_t *result = sysbvm_heap_allocatePointerTuple(&context->heap, slotCount);
    sysbvm_tuple_setIdentityHash(result, sysbvm_context_generateIdentityHash(context));
    if(result)
        sysbvm_tuple_setType(result, type);
    return result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_context_shallowCopy(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    if(!sysbvm_tuple_isNonNullPointer(tuple))
        return tuple;

    sysbvm_object_tuple_t *result = sysbvm_heap_shallowCopyTuple(&context->heap, (sysbvm_object_tuple_t*)tuple);
    sysbvm_tuple_setIdentityHash(result, sysbvm_context_generateIdentityHash(context));
    return (sysbvm_tuple_t)result;    
}
