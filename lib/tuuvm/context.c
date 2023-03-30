#include "internal/context.h"
#include "tuuvm/type.h"
#include "tuuvm/array.h"
#include "tuuvm/arrayList.h"
#include "tuuvm/environment.h"
#include "tuuvm/gc.h"
#include "tuuvm/stackFrame.h"
#include "tuuvm/string.h"
#include "tuuvm/set.h"
#include "tuuvm/function.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static bool tuuvm_context_default_jitEnabled = true;

extern void tuuvm_array_registerPrimitives(void);
extern void tuuvm_arrayList_registerPrimitives(void);
extern void tuuvm_astInterpreter_registerPrimitives(void);
extern void tuuvm_boolean_registerPrimitives(void);
extern void tuuvm_bytecode_registerPrimitives(void);
extern void tuuvm_bytecodeCompiler_registerPrimitives();
extern void tuuvm_dictionary_registerPrimitives(void);
extern void tuuvm_errors_registerPrimitives(void);
extern void tuuvm_environment_registerPrimitives(void);
extern void tuuvm_filesystem_registerPrimitives(void);
extern void tuuvm_float_registerPrimitives(void);
extern void tuuvm_function_registerPrimitives(void);
extern void tuuvm_integer_registerPrimitives(void);
extern void tuuvm_io_registerPrimitives(void);
extern void tuuvm_primitiveInteger_registerPrimitives(void);
extern void tuuvm_string_registerPrimitives(void);
extern void tuuvm_stringStream_registerPrimitives(void);
extern void tuuvm_time_registerPrimitives(void);
extern void tuuvm_tuple_registerPrimitives(void);
extern void tuuvm_type_registerPrimitives(void);

extern void tuuvm_array_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_arrayList_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_astInterpreter_setupASTInterpreter(tuuvm_context_t *context);
extern void tuuvm_boolean_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_bytecode_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_bytecodeCompiler_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_dictionary_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_errors_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_environment_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_filesystem_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_float_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_function_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_integer_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_io_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_primitiveInteger_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_string_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_stringStream_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_time_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_tuple_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_type_setupPrimitives(tuuvm_context_t *context);

void tuuvm_context_registerPrimitives(void)
{
    tuuvm_primitiveTable_registerFunction(tuuvm_tuple_primitive_identityEquals, "==");
    tuuvm_primitiveTable_registerFunction(tuuvm_tuple_primitive_identityNotEquals, "~~");
    tuuvm_primitiveTable_registerFunction(tuuvm_tuple_primitive_identityHash, "identityHash");
    tuuvm_primitiveTable_registerFunction(tuuvm_string_primitive_equals, "String::=");
    tuuvm_primitiveTable_registerFunction(tuuvm_string_primitive_hash, "String::hash");

    tuuvm_array_registerPrimitives();
    tuuvm_arrayList_registerPrimitives();
    tuuvm_astInterpreter_registerPrimitives();
    tuuvm_boolean_registerPrimitives();
    tuuvm_bytecode_registerPrimitives();
    tuuvm_bytecodeCompiler_registerPrimitives();
    tuuvm_dictionary_registerPrimitives();
    tuuvm_errors_registerPrimitives();
    tuuvm_environment_registerPrimitives();
    tuuvm_filesystem_registerPrimitives();
    tuuvm_float_registerPrimitives();
    tuuvm_function_registerPrimitives();
    tuuvm_integer_registerPrimitives();
    tuuvm_io_registerPrimitives();
    tuuvm_primitiveInteger_registerPrimitives();
    tuuvm_string_registerPrimitives();
    tuuvm_stringStream_registerPrimitives();
    tuuvm_time_registerPrimitives();
    tuuvm_tuple_registerPrimitives();
    tuuvm_type_registerPrimitives();
}

TUUVM_API tuuvm_tuple_t tuuvm_context_createIntrinsicClass(tuuvm_context_t *context, const char *name, tuuvm_tuple_t supertype, ...)
{
    tuuvm_tuple_t nameSymbol = tuuvm_symbol_internWithCString(context, name);
    tuuvm_tuple_t type = tuuvm_type_createAnonymousClassAndMetaclass(context, supertype);
    tuuvm_type_setName(type, nameSymbol);
    tuuvm_environment_setNewSymbolBindingWithValue(context, context->roots.globalNamespace, nameSymbol, type);
    tuuvm_arrayList_add(context, context->roots.intrinsicTypes, type);

    // First pass: count the arguments.
    size_t slotNameCount = 0;
    va_list valist;
    va_start(valist, supertype);
    while(va_arg(valist, const char *))
    {
        va_arg(valist, tuuvm_typeSlotFlags_t);
        va_arg(valist, int);
        ++slotNameCount;
    }
    va_end(valist);

    // Second pass: make the argument list.
    tuuvm_tuple_t slots = tuuvm_array_create(context, slotNameCount);
    va_start(valist, supertype);
    for(size_t i = 0; i < slotNameCount; ++i)
    {
        tuuvm_tuple_t name = tuuvm_symbol_internWithCString(context, va_arg(valist, const char *));
        tuuvm_tuple_t flags = tuuvm_tuple_integer_encodeSmall(va_arg(valist, int));
        tuuvm_tuple_t type = va_arg(valist, tuuvm_tuple_t);
        if(!type)
            type = context->roots.anyValueType;
        tuuvm_array_atPut(slots, i, tuuvm_typeSlot_create(context, name, flags, type));
    }

    va_end(valist);
    tuuvm_type_setSlots(type, slots);

    // Set the total slot count.
    size_t totalSlotCount = slotNameCount;
    tuuvm_tuple_t actualSupertype = tuuvm_type_getSupertype(type);
    if(tuuvm_tuple_isNonNullPointer(actualSupertype))
        totalSlotCount += tuuvm_type_getTotalSlotCount(actualSupertype);
    tuuvm_type_setTotalSlotCount(context, type, totalSlotCount);

    return type;
}

TUUVM_API tuuvm_tuple_t tuuvm_context_createIntrinsicPrimitiveValueType(tuuvm_context_t *context, const char *name, tuuvm_tuple_t supertype)
{
    tuuvm_tuple_t nameSymbol = tuuvm_symbol_internWithCString(context, name);
    tuuvm_tuple_t type = tuuvm_type_createAnonymousPrimitiveValueTypeAndValueMetatype(context, supertype);
    tuuvm_type_setName(type, nameSymbol);
    tuuvm_environment_setNewSymbolBindingWithValue(context, context->roots.globalNamespace, nameSymbol, type);
    tuuvm_arrayList_add(context, context->roots.intrinsicTypes, type);

    tuuvm_type_setSlots(type, tuuvm_array_create(context, 0));
    tuuvm_type_setTotalSlotCount(context, type, 0);
    return type;
}

TUUVM_API tuuvm_tuple_t tuuvm_context_createIntrinsicType(tuuvm_context_t *context, const char *name, tuuvm_tuple_t supertype, ...)
{
    tuuvm_tuple_t nameSymbol = tuuvm_symbol_internWithCString(context, name);
    tuuvm_tuple_t type = tuuvm_type_createWithName(context, nameSymbol);
    if(supertype)
        tuuvm_type_setSupertype(type, supertype);
    tuuvm_environment_setNewSymbolBindingWithValue(context, context->roots.globalNamespace, nameSymbol, type);
    tuuvm_arrayList_add(context, context->roots.intrinsicTypes, type);

    // First pass: count the arguments.
    size_t slotNameCount = 0;
    va_list valist;
    va_start(valist, supertype);
    while(va_arg(valist, const char *))
    {
        va_arg(valist, tuuvm_typeSlotFlags_t);
        va_arg(valist, int);
        ++slotNameCount;
    }
    va_end(valist);

    // Second pass: make the argument list.
    tuuvm_tuple_t slots = tuuvm_array_create(context, slotNameCount);
    va_start(valist, supertype);
    for(size_t i = 0; i < slotNameCount; ++i)
    {
        tuuvm_tuple_t name = tuuvm_symbol_internWithCString(context, va_arg(valist, const char *));
        tuuvm_tuple_t flags = tuuvm_tuple_integer_encodeSmall(va_arg(valist, int));
        tuuvm_tuple_t type = va_arg(valist, tuuvm_tuple_t);
        if(!type)
            type = context->roots.anyValueType;
        tuuvm_array_atPut(slots, i, tuuvm_typeSlot_create(context, name, flags, type));
    }

    va_end(valist);
    tuuvm_type_setSlots(type, slots);

    // Set the total slot count.
    size_t totalSlotCount = slotNameCount;
    tuuvm_tuple_t actualSupertype = tuuvm_type_getSupertype(type);
    if(tuuvm_tuple_isNonNullPointer(actualSupertype))
        totalSlotCount += tuuvm_type_getTotalSlotCount(actualSupertype);
    tuuvm_type_setTotalSlotCount(context, type, totalSlotCount);

    return type;
}

static void tuuvm_context_setIntrinsicTypeMetadata(tuuvm_context_t *context, tuuvm_tuple_t type, const char *name, tuuvm_tuple_t supertype, ...)
{
    tuuvm_tuple_t nameSymbol = tuuvm_symbol_internWithCString(context, name);
    tuuvm_type_setName(type, nameSymbol);
    if(supertype)
        tuuvm_type_setSupertype(type, supertype);
    tuuvm_environment_setNewSymbolBindingWithValue(context, context->roots.globalNamespace, nameSymbol, type);
    tuuvm_arrayList_add(context, context->roots.intrinsicTypes, type);

    // First pass: count the arguments.
    size_t slotNameCount = 0;
    va_list valist;
    va_start(valist, supertype);
    while(va_arg(valist, const char *))
    {
        va_arg(valist, tuuvm_typeSlotFlags_t);
        va_arg(valist, int);
        ++slotNameCount;
    }
    va_end(valist);

    // Second pass: make the argument list.
    tuuvm_tuple_t slots = tuuvm_array_create(context, slotNameCount);
    va_start(valist, supertype);
    for(size_t i = 0; i < slotNameCount; ++i)
    {
        tuuvm_tuple_t name = tuuvm_symbol_internWithCString(context, va_arg(valist, const char *));
        tuuvm_tuple_t flags = tuuvm_tuple_integer_encodeSmall(va_arg(valist, int));
        tuuvm_tuple_t type = va_arg(valist, tuuvm_tuple_t);
        if(!type)
            type = context->roots.anyValueType;
        tuuvm_array_atPut(slots, i, tuuvm_typeSlot_create(context, name, flags, type));
    }

    va_end(valist);
    tuuvm_type_setSlots(type, slots);

    // Set the total slot count.
    size_t totalSlotCount = slotNameCount;
    tuuvm_tuple_t actualSupertype = tuuvm_type_getSupertype(type);
    if(tuuvm_tuple_isNonNullPointer(actualSupertype))
        totalSlotCount += tuuvm_type_getTotalSlotCount(actualSupertype);
    tuuvm_type_setTotalSlotCount(context, type, totalSlotCount);
}

TUUVM_API void tuuvm_context_setIntrinsicSymbolBindingValue(tuuvm_context_t *context, tuuvm_tuple_t symbol, tuuvm_tuple_t value)
{
    tuuvm_environment_setNewSymbolBindingWithValue(context, context->roots.globalNamespace, symbol, value);

    if(tuuvm_tuple_isFunction(context, value))
    {
        tuuvm_function_t *functionObject = (tuuvm_function_t*)value;
        if(!functionObject->owner && !functionObject->name)
            functionObject->name = symbol;
    }
}

TUUVM_API void tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(tuuvm_context_t *context, const char *symbolName, tuuvm_tuple_t binding)
{
    tuuvm_context_setIntrinsicSymbolBindingValue(context, tuuvm_symbol_internWithCString(context, symbolName), binding);
}

TUUVM_API tuuvm_tuple_t tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(tuuvm_context_t *context, const char *symbolString, size_t argumentCount, tuuvm_bitflags_t flags, void *userdata, tuuvm_functionEntryPoint_t entryPoint)
{
    struct {
        tuuvm_tuple_t symbol;
        tuuvm_tuple_t primitiveFunction;
    } gcFrame = {0};

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.symbol = tuuvm_symbol_internWithCString(context, symbolString);
    gcFrame.primitiveFunction = tuuvm_function_createPrimitive(context, argumentCount, flags, userdata, entryPoint);
    tuuvm_context_setIntrinsicSymbolBindingValue(context, gcFrame.symbol, gcFrame.primitiveFunction);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.primitiveFunction;
}

TUUVM_API tuuvm_tuple_t tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(tuuvm_context_t *context, const char *symbolString, tuuvm_tuple_t ownerClass, const char *selectorString, size_t argumentCount, tuuvm_bitflags_t flags, void *userdata, tuuvm_functionEntryPoint_t entryPoint)
{
    struct {
        tuuvm_tuple_t symbol;
        tuuvm_tuple_t selector;
        tuuvm_tuple_t primitiveFunction;
        tuuvm_tuple_t ownerClass;
    } gcFrame = {
        .ownerClass = ownerClass
    };

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.primitiveFunction = tuuvm_function_createPrimitive(context, argumentCount, flags, userdata, entryPoint);
    if(selectorString)
    {
        gcFrame.selector = tuuvm_symbol_internWithCString(context, selectorString);
        tuuvm_type_setMethodWithSelector(context, gcFrame.ownerClass, gcFrame.selector, gcFrame.primitiveFunction);
    }

    if(symbolString)
    {
        gcFrame.symbol = tuuvm_symbol_internWithCString(context, symbolString);
        tuuvm_context_setIntrinsicSymbolBindingValue(context, gcFrame.symbol, gcFrame.primitiveFunction);
    }

    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.primitiveFunction;
}

static void tuuvm_context_createBasicTypes(tuuvm_context_t *context)
{
    // Make a circular base type.
    context->roots.untypedType = tuuvm_type_createAnonymousAndMetatype(context);
    context->roots.anyValueType = tuuvm_type_createAnonymousAndMetatype(context);
    context->roots.typeType = tuuvm_type_createAnonymous(context);
    tuuvm_tuple_setType((tuuvm_object_tuple_t*)context->roots.typeType, context->roots.typeType);
    tuuvm_type_setSupertype(context->roots.anyValueType, context->roots.untypedType);

    tuuvm_type_setFlags(context, context->roots.anyValueType, TUUVM_TYPE_FLAGS_NULLABLE | TUUVM_TYPE_FLAGS_DYNAMIC);
    tuuvm_type_setFlags(context, context->roots.untypedType, TUUVM_TYPE_FLAGS_NULLABLE | TUUVM_TYPE_FLAGS_DYNAMIC);
    tuuvm_type_setFlags(context, context->roots.typeType, TUUVM_TYPE_FLAGS_NULLABLE);

    context->roots.objectType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.anyValueType);
    context->roots.programEntityType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    tuuvm_type_setSupertype(context->roots.typeType, context->roots.programEntityType);
    tuuvm_type_setSupertype(tuuvm_tuple_getType(context, context->roots.programEntityType), tuuvm_tuple_getType(context, context->roots.objectType));

    context->roots.metatypeType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.typeType);

    tuuvm_tuple_setType((tuuvm_object_tuple_t*)tuuvm_tuple_getType(context, context->roots.untypedType), context->roots.metatypeType);
    tuuvm_type_setSupertype(tuuvm_tuple_getType(context, context->roots.untypedType), context->roots.typeType);

    tuuvm_tuple_setType((tuuvm_object_tuple_t*)tuuvm_tuple_getType(context, context->roots.anyValueType), context->roots.metatypeType);
    tuuvm_type_setSupertype(tuuvm_tuple_getType(context, context->roots.anyValueType), context->roots.typeType);

    context->roots.classType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.typeType);
    context->roots.metaclassType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.metatypeType);

    tuuvm_tuple_setType((tuuvm_object_tuple_t*)tuuvm_tuple_getType(context, context->roots.objectType), context->roots.metaclassType);
    tuuvm_type_setSupertype(tuuvm_tuple_getType(context, context->roots.objectType), context->roots.classType);

    tuuvm_tuple_setType((tuuvm_object_tuple_t*)tuuvm_tuple_getType(context, context->roots.programEntityType), context->roots.metaclassType);

    tuuvm_tuple_setType((tuuvm_object_tuple_t*)tuuvm_tuple_getType(context, context->roots.metatypeType), context->roots.metaclassType);

    tuuvm_tuple_setType((tuuvm_object_tuple_t*)tuuvm_tuple_getType(context, context->roots.classType), context->roots.metaclassType);

    tuuvm_tuple_setType((tuuvm_object_tuple_t*)tuuvm_tuple_getType(context, context->roots.metaclassType), context->roots.metaclassType);

    // Create the type slot class.
    context->roots.typeSlotType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.typeSlotType);

    // Create the function class.
    context->roots.functionType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    tuuvm_type_setFlags(context, context->roots.functionType, TUUVM_TYPE_FLAGS_NULLABLE | TUUVM_TYPE_FLAGS_FUNCTION);
    context->roots.functionDefinitionType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    context->roots.functionBytecodeType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);

    // Create the function type classes.
    context->roots.functionTypeType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.typeType);
    context->roots.dependentFunctionTypeType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.functionTypeType);
    context->roots.simpleFunctionTypeType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.functionTypeType);

    context->roots.symbolBindingType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    context->roots.symbolAnalysisBindingType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.symbolBindingType);
    context->roots.symbolArgumentBindingType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.symbolAnalysisBindingType);
    context->roots.symbolCaptureBindingType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.symbolAnalysisBindingType);
    context->roots.symbolLocalBindingType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.symbolAnalysisBindingType);
    context->roots.symbolMacroValueBindingType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.symbolBindingType);
    context->roots.symbolValueBindingType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.symbolBindingType);
    context->roots.environmentType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.programEntityType);
    context->roots.namespaceType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.environmentType);
    context->roots.astNodeType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);

    // Collection base hierarchy
    context->roots.collectionType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    context->roots.hashedCollectionType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.collectionType);
    context->roots.sequenceableCollectionType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.collectionType);
    context->roots.arrayedCollectionType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.sequenceableCollectionType);

    // Create the basic hash functions.
    context->roots.identityEqualsFunction = tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_tuple_primitive_identityEquals);
    context->roots.identityNotEqualsFunction = tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_tuple_primitive_identityNotEquals);
    context->roots.identityHashFunction = tuuvm_function_createPrimitive(context, 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_tuple_primitive_identityHash);
    context->roots.stringEqualsFunction = tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_string_primitive_equals);
    context->roots.stringHashFunction = tuuvm_function_createPrimitive(context, 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE | TUUVM_FUNCTION_FLAGS_PURE | TUUVM_FUNCTION_FLAGS_FINAL, NULL, tuuvm_string_primitive_hash);

    context->roots.symbolType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.arrayedCollectionType);
    context->roots.stringSymbolType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.symbolType);
    tuuvm_type_setFlags(context, context->roots.stringSymbolType, TUUVM_TYPE_FLAGS_NULLABLE | TUUVM_TYPE_FLAGS_BYTES);

    context->roots.dictionaryType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.hashedCollectionType);
    context->roots.weakKeyDictionaryType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.dictionaryType);
    context->roots.weakValueDictionaryType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.dictionaryType);
    context->roots.identityDictionaryType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.dictionaryType);
    context->roots.methodDictionaryType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.hashedCollectionType);

    context->roots.setType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.hashedCollectionType);
    context->roots.identitySetType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.setType);
    context->roots.weakSetType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.hashedCollectionType);
    context->roots.weakIdentitySetType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.weakIdentitySetType);

    context->roots.arrayType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.arrayedCollectionType);
    context->roots.arrayListType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.sequenceableCollectionType);
    context->roots.weakArrayType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.arrayType);
    context->roots.weakArrayListType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.arrayListType);

    context->roots.internedSymbolSet = tuuvm_identitySet_create(context);
    context->roots.sessionToken = tuuvm_tuple_systemHandle_encode(context, 1);

    // Create the intrinsic built-in environment
    context->roots.globalNamespace = tuuvm_environment_create(context, TUUVM_NULL_TUPLE);
    context->roots.intrinsicTypes = tuuvm_arrayList_create(context);

    context->roots.equalsSelector = tuuvm_symbol_internWithCString(context, "=");
    context->roots.hashSelector = tuuvm_symbol_internWithCString(context, "hash");
    context->roots.asStringSelector = tuuvm_symbol_internWithCString(context, "asString");
    context->roots.printStringSelector = tuuvm_symbol_internWithCString(context, "printString");
    context->roots.doesNotUnderstandSelector = tuuvm_symbol_internWithCString(context, "doesNotUnderstand:");

    context->roots.loadAtOffsetWithTypeSelector = tuuvm_symbol_internWithCString(context, "loadAtOffset:withType:");
    context->roots.storeAtOffsetWithTypeSelector = tuuvm_symbol_internWithCString(context, "store:atOffset:withType:");
    context->roots.assignmentSelector = tuuvm_symbol_internWithCString(context, ":=");
    context->roots.underscoreSelector = tuuvm_symbol_internWithCString(context, "_");

    context->roots.astNodeAnalysisSelector = tuuvm_symbol_internWithCString(context, "analyzeWithEnvironment:");
    context->roots.astNodeEvaluationSelector = tuuvm_symbol_internWithCString(context, "evaluateWithEnvironment:");
    context->roots.astNodeAnalysisAndEvaluationSelector = tuuvm_symbol_internWithCString(context, "analyzeAndEvaluateWithEnvironment:");
    context->roots.astNodeCompileIntoBytecodeSelector = tuuvm_symbol_internWithCString(context, "compileIntoBytecodeWith:");
    
    context->roots.analyzeAndEvaluateMessageSendNodeForReceiverWithEnvironmentSelector = tuuvm_symbol_internWithCString(context, "analyzeAndEvaluateMessageSendNode:forReceiver:withEnvironment:");
    context->roots.analyzeMessageSendNodeWithEnvironmentSelector = tuuvm_symbol_internWithCString(context, "analyzeMessageSendNode:withEnvironment:");
    context->roots.analyzeAndEvaluateMessageChainNodeForReceiverWithEnvironmentSelector = tuuvm_symbol_internWithCString(context, "analyzeAndEvaluateMessageChainNode:forReceiver:withEnvironment:");
    context->roots.analyzeMessageChainNodeWithEnvironmentSelector = tuuvm_symbol_internWithCString(context, "analyzeMessageChainNode:withEnvironment:");
    context->roots.analyzeAndEvaluateConcreteMetaValueWithEnvironmentSelector = tuuvm_symbol_internWithCString(context, "analyzeAndEvaluateConcreteMetaValue:withEnvironment:");
    context->roots.analyzeConcreteMetaValueWithEnvironmentSelector = tuuvm_symbol_internWithCString(context, "analyzeConcreteMetaValue:withEnvironment:");

    context->roots.coerceASTNodeWithEnvironmentSelector = tuuvm_symbol_internWithCString(context, "coerceASTNode:withEnvironment:");
    context->roots.analyzeAndTypeCheckFunctionApplicationNodeWithEnvironmentSelector = tuuvm_symbol_internWithCString(context, "analyzeAndTypeCheckFunctionApplicationNode:withEnvironment:");
    context->roots.analyzeAndTypeCheckMessageSendNodeWithEnvironmentSelector = tuuvm_symbol_internWithCString(context, "analyzeAndTypeCheckMessageSendNode:withEnvironment:");
    context->roots.getOrCreateDependentApplicationValueForNodeSelector = tuuvm_symbol_internWithCString(context, "getOrCreateDependentApplicationValueForNode:");

    context->roots.applyWithoutArgumentsSelector = tuuvm_symbol_internWithCString(context, "()");
    context->roots.applyWithArgumentsSelector = tuuvm_symbol_internWithCString(context, "():");

    context->roots.primitiveNamedSelector = tuuvm_symbol_internWithCString(context, "primitive:");

    context->roots.coerceValueSelector = tuuvm_symbol_internWithCString(context, "coerceValue:");
    context->roots.defaultValueSelector = tuuvm_symbol_internWithCString(context, "defaultValue");

    context->roots.anyValueToVoidPrimitiveName = tuuvm_symbol_internWithCString(context, "Void::fromAnyValue");
    context->roots.pointerLikeLoadPrimitiveName = tuuvm_symbol_internWithCString(context, "PointerLikeType::load");
    context->roots.pointerLikeStorePrimitiveName = tuuvm_symbol_internWithCString(context, "PointerLikeType::store:");

    tuuvm_context_setIntrinsicSymbolBindingValue(context, tuuvm_symbol_internWithCString(context, "nil"), TUUVM_NULL_TUPLE);
    tuuvm_context_setIntrinsicSymbolBindingValue(context, tuuvm_symbol_internWithCString(context, "false"), TUUVM_FALSE_TUPLE);
    tuuvm_context_setIntrinsicSymbolBindingValue(context, tuuvm_symbol_internWithCString(context, "true"), TUUVM_TRUE_TUPLE);
    tuuvm_context_setIntrinsicSymbolBindingValue(context, tuuvm_symbol_internWithCString(context, "void"), TUUVM_VOID_TUPLE);
    tuuvm_context_setIntrinsicSymbolBindingValue(context, tuuvm_symbol_internWithCString(context, "__hashtableEmptyElement__"), TUUVM_HASHTABLE_EMPTY_ELEMENT_TUPLE);
    tuuvm_context_setIntrinsicSymbolBindingValue(context, tuuvm_symbol_internWithCString(context, "__tombstone__"), TUUVM_TOMBSTONE_TUPLE);

    tuuvm_context_setIntrinsicSymbolBindingValue(context, tuuvm_symbol_internWithCString(context, "__Global__"), context->roots.globalNamespace);
    tuuvm_context_setIntrinsicSymbolBindingValue(context, tuuvm_symbol_internWithCString(context, "BootstrapEnv::IntrinsicTypes"), context->roots.intrinsicTypes);
    tuuvm_context_setIntrinsicSymbolBindingValue(context, tuuvm_symbol_internWithCString(context, "BootstrapEnv::InternedSymbolSet"), context->roots.internedSymbolSet);

    tuuvm_context_setIntrinsicSymbolBindingValue(context, tuuvm_symbol_internWithCString(context, "identityHash"), context->roots.identityHashFunction);
    tuuvm_context_setIntrinsicSymbolBindingValue(context, tuuvm_symbol_internWithCString(context, "=="), context->roots.identityEqualsFunction);
    tuuvm_context_setIntrinsicSymbolBindingValue(context, tuuvm_symbol_internWithCString(context, "~~"), context->roots.identityNotEqualsFunction);

    tuuvm_context_setIntrinsicSymbolBindingValue(context, tuuvm_symbol_internWithCString(context, "String::hash"), context->roots.stringHashFunction);
    tuuvm_context_setIntrinsicSymbolBindingValue(context, tuuvm_symbol_internWithCString(context, "String::equals:"), context->roots.stringEqualsFunction);

    // Some basic method
    tuuvm_type_setHashFunction(context, context->roots.anyValueType, context->roots.identityHashFunction);
    tuuvm_type_setEqualsFunction(context, context->roots.anyValueType, context->roots.identityEqualsFunction);
    
    tuuvm_type_setMethodWithSelector(context, context->roots.anyValueType, tuuvm_symbol_internWithCString(context, "identityHash"), context->roots.identityHashFunction);
    tuuvm_type_setMethodWithSelector(context, context->roots.anyValueType, tuuvm_symbol_internWithCString(context, "=="), context->roots.identityEqualsFunction);
    tuuvm_type_setMethodWithSelector(context, context->roots.anyValueType, tuuvm_symbol_internWithCString(context, "~~"), context->roots.identityNotEqualsFunction);


    // Create the value type classes.
    context->roots.valueType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.typeType);
    context->roots.valueMetatypeType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.metatypeType);
    context->roots.primitiveValueType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.typeType);
    
    context->roots.anyPointerType = tuuvm_type_createAnonymous(context);
    context->roots.anyReferenceType = tuuvm_type_createAnonymous(context);
    tuuvm_type_setSupertype(context->roots.anyReferenceType, TUUVM_NULL_TUPLE);

    context->roots.pointerLikeType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.valueType);
    context->roots.pointerType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.pointerLikeType);
    context->roots.referenceType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.pointerLikeType);

    context->roots.structureType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.valueType);

    // Some basic types
    context->roots.voidType = tuuvm_context_createIntrinsicPrimitiveValueType(context, "Void", context->roots.anyValueType);

    context->roots.primitiveNumberType = tuuvm_context_createIntrinsicPrimitiveValueType(context, "PrimitiveNumber", context->roots.anyValueType);
    context->roots.primitiveIntegerType = tuuvm_context_createIntrinsicPrimitiveValueType(context, "PrimitiveInteger", context->roots.primitiveNumberType);
    context->roots.primitiveCharacterType = tuuvm_context_createIntrinsicPrimitiveValueType(context, "PrimitiveCharacter", context->roots.primitiveIntegerType);
    context->roots.primitiveUnsignedIntegerType = tuuvm_context_createIntrinsicPrimitiveValueType(context, "PrimitiveUnsignedInteger", context->roots.primitiveIntegerType);
    context->roots.primitiveSignedIntegerType = tuuvm_context_createIntrinsicPrimitiveValueType(context, "PrimitiveSignedInteger", context->roots.primitiveIntegerType);
    context->roots.primitiveFloatType = tuuvm_context_createIntrinsicPrimitiveValueType(context, "PrimitiveFloat", context->roots.primitiveNumberType);

    context->roots.char8Type = tuuvm_context_createIntrinsicPrimitiveValueType(context, "Char8", context->roots.primitiveCharacterType);
    context->roots.uint8Type = tuuvm_context_createIntrinsicPrimitiveValueType(context, "UInt8", context->roots.primitiveUnsignedIntegerType);
    context->roots.int8Type = tuuvm_context_createIntrinsicPrimitiveValueType(context, "Int8", context->roots.primitiveSignedIntegerType);

    context->roots.char16Type = tuuvm_context_createIntrinsicPrimitiveValueType(context, "Char16", context->roots.primitiveCharacterType);
    context->roots.uint16Type = tuuvm_context_createIntrinsicPrimitiveValueType(context, "UInt16", context->roots.primitiveUnsignedIntegerType);
    context->roots.int16Type = tuuvm_context_createIntrinsicPrimitiveValueType(context, "Int16", context->roots.primitiveSignedIntegerType);

    context->roots.char32Type = tuuvm_context_createIntrinsicPrimitiveValueType(context, "Char32", context->roots.primitiveCharacterType);
    context->roots.uint32Type = tuuvm_context_createIntrinsicPrimitiveValueType(context, "UInt32", context->roots.primitiveUnsignedIntegerType);
    context->roots.int32Type = tuuvm_context_createIntrinsicPrimitiveValueType(context, "Int32", context->roots.primitiveSignedIntegerType);

    context->roots.uint64Type = tuuvm_context_createIntrinsicPrimitiveValueType(context, "UInt64", context->roots.primitiveUnsignedIntegerType);
    context->roots.int64Type = tuuvm_context_createIntrinsicPrimitiveValueType(context, "Int64", context->roots.primitiveSignedIntegerType);

    context->roots.float32Type = tuuvm_context_createIntrinsicPrimitiveValueType(context, "Float32", context->roots.primitiveFloatType);
    context->roots.float64Type = tuuvm_context_createIntrinsicPrimitiveValueType(context, "Float64", context->roots.primitiveFloatType);
    
    context->roots.bitflagsType = sizeof(tuuvm_bitflags_t) == 4 ? context->roots.uint32Type : context->roots.uint64Type;
    context->roots.systemHandleType = sizeof(tuuvm_systemHandle_t) == 4 ? context->roots.int32Type : context->roots.int64Type;

    context->roots.sizeType = context->targetWordSize == 4 ? context->roots.uint32Type : context->roots.uint64Type;
    context->roots.uintptrType = context->targetWordSize == 4 ? context->roots.uint32Type : context->roots.uint64Type;
    context->roots.intptrType = context->targetWordSize == 4 ? context->roots.int32Type : context->roots.int64Type;

    context->roots.booleanType = tuuvm_context_createIntrinsicPrimitiveValueType(context, "Boolean", context->roots.anyValueType);
    context->roots.trueType = tuuvm_context_createIntrinsicPrimitiveValueType(context, "True", context->roots.booleanType);
    context->roots.falseType = tuuvm_context_createIntrinsicPrimitiveValueType(context, "False", context->roots.booleanType);

    context->roots.integerType = tuuvm_context_createIntrinsicClass(context, "Integer", TUUVM_NULL_TUPLE, NULL);
    context->roots.positiveIntegerType = tuuvm_context_createIntrinsicClass(context, "PositiveInteger", context->roots.integerType, NULL);
    context->roots.negativeIntegerType = tuuvm_context_createIntrinsicClass(context, "NegativeInteger", context->roots.integerType, NULL);

    context->roots.undefinedObjectType = tuuvm_context_createIntrinsicClass(context, "UndefinedObject", TUUVM_NULL_TUPLE, NULL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.undefinedObjectType, TUUVM_TYPE_FLAGS_NULLABLE | TUUVM_TYPE_FLAGS_IMMEDIATE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);

    context->roots.pendingMemoizationValueType = tuuvm_context_createIntrinsicClass(context, "PendingMemoizationValue", TUUVM_NULL_TUPLE, NULL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.pendingMemoizationValueType, TUUVM_TYPE_FLAGS_NULLABLE | TUUVM_TYPE_FLAGS_IMMEDIATE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);

    context->roots.tombstoneType = tuuvm_context_createIntrinsicClass(context, "ObjectTombstone", TUUVM_NULL_TUPLE, NULL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.tombstoneType, TUUVM_TYPE_FLAGS_NULLABLE | TUUVM_TYPE_FLAGS_IMMEDIATE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);

    context->roots.stringType = tuuvm_context_createIntrinsicClass(context, "String", context->roots.arrayedCollectionType, NULL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.stringType, TUUVM_TYPE_FLAGS_NULLABLE | TUUVM_TYPE_FLAGS_BYTES | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);

    // Special types used during semantic analysis.
    context->roots.controlFlowEscapeType = tuuvm_context_createIntrinsicPrimitiveValueType(context, "ControlFlowEscapeType", context->roots.voidType);
    context->roots.controlFlowBreakType = tuuvm_context_createIntrinsicPrimitiveValueType(context, "ControlFlowBreakType", context->roots.controlFlowEscapeType);
    context->roots.controlFlowContinueType = tuuvm_context_createIntrinsicPrimitiveValueType(context, "ControlFlowContinueType", context->roots.controlFlowEscapeType);
    context->roots.controlFlowReturnType = tuuvm_context_createIntrinsicPrimitiveValueType(context, "ControlFlowReturnType", context->roots.controlFlowEscapeType);
    context->roots.noReturnType = tuuvm_context_createIntrinsicPrimitiveValueType(context, "NoReturn", context->roots.controlFlowEscapeType);
    context->roots.unwindsType = tuuvm_context_createIntrinsicPrimitiveValueType(context, "Unwinds", context->roots.controlFlowEscapeType);

    context->roots.decayedTypeInferenceType = tuuvm_context_createIntrinsicPrimitiveValueType(context, "DecayedTypeInferenceType", TUUVM_NULL_TUPLE);
    context->roots.directTypeInferenceType = tuuvm_context_createIntrinsicPrimitiveValueType(context, "DirectTypeInferenceType", TUUVM_NULL_TUPLE);

    // Set the name of the root basic type.
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.untypedType, "Untyped", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.anyValueType, "AnyValue", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.objectType, "Object", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.programEntityType, "ProgramEntity", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.typeType, "Type", TUUVM_NULL_TUPLE,
        "name", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolType,
        "owner", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "supertype", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeType,
        "slots", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "totalSlotCount", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "flags", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.bitflagsType,
        
        "macroMethodDictionary", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "methodDictionary", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "fallbackMethodDictionary", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,

        "pendingSlots", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "subtypes", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.classType, "Class", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.metatypeType, "Metatype", TUUVM_NULL_TUPLE,
        "thisType", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.metaclassType, "Metaclass", TUUVM_NULL_TUPLE,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.typeSlotType, "TypeSlot", TUUVM_NULL_TUPLE,
        "name", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "flags", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "type", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        NULL);

    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.valueType, "ValueType", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.valueMetatypeType, "ValueMetatype", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.primitiveValueType, "PrimitiveValueType", TUUVM_NULL_TUPLE, NULL);
    
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.anyPointerType, "AnyPointer", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.anyReferenceType, "AnyReference", TUUVM_NULL_TUPLE, NULL);

    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.pointerLikeType, "PointerLikeType", TUUVM_NULL_TUPLE,
        "baseType", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeType,
        "addressSpace", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "loadValueFunction", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionType,
        "storeValueFunction", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionType,
        TUUVM_NULL_TUPLE);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.pointerType, "PointerType", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.referenceType, "ReferenceType", TUUVM_NULL_TUPLE, NULL);

    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.structureType, "Structure", TUUVM_NULL_TUPLE, NULL);

    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.environmentType, "Environment", TUUVM_NULL_TUPLE,
        "parent", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.environmentType,
        "symbolTable", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.namespaceType, "Namespace", TUUVM_NULL_TUPLE,
        NULL);
    context->roots.analysisAndEvaluationEnvironmentType = tuuvm_context_createIntrinsicClass(context, "AnalysisAndEvaluationEnvironment", context->roots.environmentType,
        "analyzerToken", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.objectType,
        "expectedType", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeType,
        "returnTarget", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "breakTarget", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "continueTarget", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        NULL);
    context->roots.analysisEnvironmentType = tuuvm_context_createIntrinsicClass(context, "AnalysisEnvironment", context->roots.analysisAndEvaluationEnvironmentType,
        NULL);
    context->roots.functionActivationEnvironmentType = tuuvm_context_createIntrinsicClass(context, "FunctionActivationEnvironment", context->roots.analysisAndEvaluationEnvironmentType,
        "function", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionType,
        "functionDefinition", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionDefinitionType,
        "dependentFunctionType", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionTypeType,
        "argumentVectorSize", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "captureVector", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "valueVector", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.functionAnalysisEnvironmentType = tuuvm_context_createIntrinsicClass(context, "FunctionAnalysisEnvironment", context->roots.analysisEnvironmentType,
        "functionDefinition", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionDefinitionType,
        "captureBindingTable", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.identityDictionaryType,
        "captureBindingList", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayListType,
        "argumentBindingList", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayListType,
        "localBindingList", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayListType,
        "innerFunctionList", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayListType,
        "pragmaList", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayListType,
        "hasBreakTarget", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "hasContinueTarget", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        NULL);
    context->roots.localAnalysisEnvironmentType = tuuvm_context_createIntrinsicClass(context, "LocalAnalysisEnvironment", context->roots.analysisEnvironmentType,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolBindingType, "SymbolBinding", TUUVM_NULL_TUPLE,
        "name", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "sourcePosition", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "type", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolAnalysisBindingType, "SymbolAnalysisBinding", TUUVM_NULL_TUPLE,
        "ownerFunction", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionDefinitionType,
        "vectorIndex", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolArgumentBindingType, "SymbolArgumentBinding", TUUVM_NULL_TUPLE,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolCaptureBindingType, "SymbolCaptureBinding", TUUVM_NULL_TUPLE,
        "sourceBinding", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolAnalysisBindingType,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolLocalBindingType, "SymbolLocalBinding", TUUVM_NULL_TUPLE,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolMacroValueBindingType, "SymbolMacroValueBinding", TUUVM_NULL_TUPLE,
        "expansion", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolValueBindingType, "SymbolValueBinding", TUUVM_NULL_TUPLE,
        "value", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.functionType, "Function", TUUVM_NULL_TUPLE,
        "name", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "owner", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "flags", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.bitflagsType,
        "argumentCount", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "captureVector", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "definition", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionDefinitionType,
        "primitiveTableIndex", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.uint32Type,
        "primitiveName", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolType,
        "nativeUserdata", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.systemHandleType,
        "nativeEntryPoint", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.systemHandleType,
        "memoizationTable", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.weakValueDictionaryType,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.functionDefinitionType, "FunctionDefinition", TUUVM_NULL_TUPLE,
        "flags", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.bitflagsType,
        "argumentCount", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "sourcePosition", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,

        "definitionEnvironment", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.environmentType,
        "definitionArgumentNodes", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "definitionResultTypeNode", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "definitionBodyNode", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,

        "analyzedType", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionTypeType,

        "analysisEnvironment", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.environmentType,
        "analyzedCaptures", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "analyzedArguments", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "analyzedLocals", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "analyzedPragmas", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "analyzedInnerFunctions", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "analyzedPrimitiveName", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,

        "analyzedArgumentNodes", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "analyzedResultTypeNode", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "analyzedBodyNode", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,

        "bytecode", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionBytecodeType,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.functionBytecodeType, "FunctionBytecode", TUUVM_NULL_TUPLE,
        "argumentCount", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "captureVectorSize", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "localVectorSize", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "literalVector", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "instructions", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.byteArrayType,

        "definition", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionDefinitionType,
        "pcToDebugListTable", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "debugSourceASTNodes", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "debugSourcePositions", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "debugSourceEnvironments", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        
        "jittedCode", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.systemHandleType,
        "jittedCodeSessionToken", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.systemHandleType,

        "jittedCodeTrampoline", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.systemHandleType,
        "jittedCodeTrampolineSessionToken", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.systemHandleType,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.functionTypeType, "FunctionType", TUUVM_NULL_TUPLE,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.dependentFunctionTypeType, "DependentFunctionType", TUUVM_NULL_TUPLE,
        "sourcePosition", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "argumentNodes", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "isVariadic", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "resultTypeNode", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,

        "environment", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.environmentType,
        "captureBindings", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "argumentBindings", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "localBindings", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.simpleFunctionTypeType, "SimpleFunctionType", TUUVM_NULL_TUPLE,
        "argumentTypes", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "isVariadic", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "resultType", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeType,
        NULL);

    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolType, "Symbol", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.stringSymbolType, "StringSymbol", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.setType, "Set", TUUVM_NULL_TUPLE,
        "size", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "storage", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.identitySetType, "IdentitySet", TUUVM_NULL_TUPLE,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.weakSetType, "WeakSet", TUUVM_NULL_TUPLE,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.weakIdentitySetType, "WeakIdentitySet", TUUVM_NULL_TUPLE,
        NULL);

    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.collectionType, "Collection", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.hashedCollectionType, "HashedCollection", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.sequenceableCollectionType, "SequenceableCollection", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.arrayedCollectionType, "ArrayedCollection", TUUVM_NULL_TUPLE, NULL);

    // 
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.arrayType, "Array", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.arrayListType, "ArrayList", TUUVM_NULL_TUPLE,
        "size", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "storage", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.weakArrayType, "WeakArray", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.weakArrayListType, "WeakArrayList", TUUVM_NULL_TUPLE, NULL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.arrayType, TUUVM_TYPE_FLAGS_NULLABLE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.arrayListType, TUUVM_TYPE_FLAGS_NULLABLE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_NONE);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.weakArrayType, TUUVM_TYPE_FLAGS_NULLABLE | TUUVM_TYPE_FLAGS_FINAL | TUUVM_TYPE_FLAGS_WEAK, TUUVM_TYPE_FLAGS_FINAL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.weakArrayListType, TUUVM_TYPE_FLAGS_NULLABLE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);

    // Create other root basic types.
    context->roots.arraySliceType = tuuvm_context_createIntrinsicClass(context, "ArraySlice", context->roots.sequenceableCollectionType,
        "elements", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "offset", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "size", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        NULL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.arraySliceType, TUUVM_TYPE_FLAGS_NULLABLE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);
    context->roots.associationType = tuuvm_context_createIntrinsicClass(context, "Association", TUUVM_NULL_TUPLE,
        "key", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "value", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        NULL);
    context->roots.weakValueAssociationType = tuuvm_context_createIntrinsicClass(context, "WeakValueAssociation", TUUVM_NULL_TUPLE,
        "key", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        //"value", TUUVM_TYPE_SLOT_FLAG_PUBLIC | TUUVM_TYPE_SLOT_FLAG_WEAK, TUUVM_NULL_TUPLE,
        NULL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.associationType, TUUVM_TYPE_FLAGS_NULLABLE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);
    context->roots.byteArrayType = tuuvm_context_createIntrinsicClass(context, "ByteArray", context->roots.arrayedCollectionType, NULL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.byteArrayType, TUUVM_TYPE_FLAGS_NULLABLE | TUUVM_TYPE_FLAGS_BYTES | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.dictionaryType, "Dictionary", TUUVM_NULL_TUPLE,
        "size", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "storage", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.identityDictionaryType, "IdentityDictionary", TUUVM_NULL_TUPLE,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.weakKeyDictionaryType, "WeakKeyDictionary", TUUVM_NULL_TUPLE,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.weakValueDictionaryType, "WeakValueDictionary", TUUVM_NULL_TUPLE,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.methodDictionaryType, "MethodDictionary", TUUVM_NULL_TUPLE,
        "size", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "storage", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.generatedSymbolType = tuuvm_context_createIntrinsicClass(context, "GeneratedSymbol", context->roots.symbolType,
        "value", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolType,
        "sourcePosition", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        NULL);
    context->roots.hashtableEmptyType = tuuvm_context_createIntrinsicClass(context, "HashtableEmpty", TUUVM_NULL_TUPLE, NULL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.hashtableEmptyType, TUUVM_TYPE_FLAGS_NULLABLE | TUUVM_TYPE_FLAGS_IMMEDIATE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);

    context->roots.macroContextType = tuuvm_context_createIntrinsicClass(context, "MacroContext", TUUVM_NULL_TUPLE,
        "sourceNode", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "sourcePosition", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        NULL);
    context->roots.messageType = tuuvm_context_createIntrinsicClass(context, "Message", TUUVM_NULL_TUPLE,
        "selector", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "arguments", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        NULL);
    context->roots.pragmaType = tuuvm_context_createIntrinsicClass(context, "Pragma", TUUVM_NULL_TUPLE,
        "selector", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "arguments", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        NULL);
    context->roots.streamType = tuuvm_context_createIntrinsicClass(context, "Stream", TUUVM_NULL_TUPLE,
        NULL);
    context->roots.stringStreamType = tuuvm_context_createIntrinsicClass(context, "StringStream", context->roots.streamType,
        "size", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "storage", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.stringType,
        NULL);
    context->roots.valueBoxType = tuuvm_context_createIntrinsicClass(context, "ValueBox", TUUVM_NULL_TUPLE,
        "value", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.untypedType,
        NULL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.valueBoxType, TUUVM_TYPE_FLAGS_NULLABLE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);

    tuuvm_context_setIntrinsicSymbolBindingValue(context, tuuvm_symbol_internWithCString(context, "Bitflags"), context->roots.bitflagsType);
    tuuvm_context_setIntrinsicSymbolBindingValue(context, tuuvm_symbol_internWithCString(context, "SystemHandle"), context->roots.systemHandleType);
    tuuvm_context_setIntrinsicSymbolBindingValue(context, tuuvm_symbol_internWithCString(context, "Size"), context->roots.sizeType);
    tuuvm_context_setIntrinsicSymbolBindingValue(context, tuuvm_symbol_internWithCString(context, "UIntPointer"), context->roots.uintptrType);
    tuuvm_context_setIntrinsicSymbolBindingValue(context, tuuvm_symbol_internWithCString(context, "IntPointer"), context->roots.intptrType);

    context->roots.sourceCodeType = tuuvm_context_createIntrinsicClass(context, "SourceCode", TUUVM_NULL_TUPLE,
        "text", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.stringType,
        "directory", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.stringType,
        "name", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.stringType,
        "language", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.stringSymbolType,
        "lineStartIndexTable", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        NULL);
    context->roots.sourcePositionType = tuuvm_context_createIntrinsicClass(context, "SourcePosition", TUUVM_NULL_TUPLE,
        "sourceCode", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sourceCodeType,
        "startIndex", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "startLine", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "startColumn", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "endIndex", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "endLine", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "endColumn", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        NULL);
    context->roots.tokenType = tuuvm_context_createIntrinsicClass(context, "Token", TUUVM_NULL_TUPLE,
        "kind", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.uint8Type,
        "sourcePosition", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sourcePositionType,
        "value", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        NULL);

    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.astNodeType, "ASTNode", TUUVM_NULL_TUPLE,
        "sourcePosition", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sourcePositionType,
        "analyzerToken", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "analyzedType", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeType,
        NULL);
    context->roots.astArgumentNodeType = tuuvm_context_createIntrinsicClass(context, "ASTArgumentNode", context->roots.astNodeType,
        "isForAll", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "name", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "type", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "binding", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        NULL);
    context->roots.astBinaryExpressionSequenceNodeType = tuuvm_context_createIntrinsicClass(context, "ASTBinaryExpressionSequenceNode", context->roots.astNodeType,
        "operands", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "operations", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.astBreakNodeType = tuuvm_context_createIntrinsicClass(context, "ASTBreakNode", context->roots.astNodeType,
        NULL);
    context->roots.astCoerceValueNodeType = tuuvm_context_createIntrinsicClass(context, "ASTCoerceValueNode", context->roots.astNodeType,
        "typeExpression", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "valueExpression", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);
    context->roots.astContinueNodeType = tuuvm_context_createIntrinsicClass(context, "ASTContinueNode", context->roots.astNodeType,
        NULL);
    context->roots.astDoWhileContinueWithNodeType = tuuvm_context_createIntrinsicClass(context, "ASTDoWhileContinueWithNode", context->roots.astNodeType,
        "bodyExpression", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "conditionExpression", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "continueExpression", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);
    context->roots.astErrorNodeType = tuuvm_context_createIntrinsicClass(context, "ASTErrorNode", context->roots.astNodeType,
        "errorMessage", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.stringType,
        NULL);
    context->roots.astFunctionApplicationNodeType = tuuvm_context_createIntrinsicClass(context, "ASTFunctionApplicationNode", context->roots.astNodeType,
        "functionExpression", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "arguments", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "applicationFlags", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.bitflagsType,
        NULL);
    context->roots.astLambdaNodeType = tuuvm_context_createIntrinsicClass(context, "ASTLambdaNode", context->roots.astNodeType,
        "flags", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "name", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "arguments", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "resultType", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "body", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "hasLazyAnalysis", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "functionDefinition", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionDefinitionType,
        "binding", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        NULL);
    context->roots.astLexicalBlockNodeType = tuuvm_context_createIntrinsicClass(context, "ASTLexicalBlockNode", context->roots.astNodeType,
        "body", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "bodyEnvironment", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.environmentType,
        NULL);
    context->roots.astLiteralNodeType = tuuvm_context_createIntrinsicClass(context, "ASTLiteralNode", context->roots.astNodeType,
        "value", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        NULL);
    context->roots.astLocalDefinitionNodeType = tuuvm_context_createIntrinsicClass(context, "ASTLocalDefinitionNode", context->roots.astNodeType,
        "nameExpression", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "typeExpression", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "valueExpression", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "binding", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "isMacroSymbol", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "isMutable", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "analyzedValueType", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeType,
        NULL);
    context->roots.astIdentifierReferenceNodeType = tuuvm_context_createIntrinsicClass(context, "ASTIdentifierReferenceNode", context->roots.astNodeType,
        "value", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "binding", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        NULL);
    context->roots.astIfNodeType = tuuvm_context_createIntrinsicClass(context, "ASTIfNode", context->roots.astNodeType,
        "conditionExpression", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "trueExpression", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "falseExpression", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);

    context->roots.astMakeAssociationNodeType = tuuvm_context_createIntrinsicClass(context, "ASTMakeAssociationNode", context->roots.astNodeType,
        "key", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "value", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);
    context->roots.astMakeByteArrayNodeType = tuuvm_context_createIntrinsicClass(context, "ASTMakeByteArrayNode", context->roots.astNodeType,
        "elements", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.astMakeDictionaryNodeType = tuuvm_context_createIntrinsicClass(context, "ASTMakeDictionaryNode", context->roots.astNodeType,
        "elements", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.astMakeArrayNodeType = tuuvm_context_createIntrinsicClass(context, "ASTMakeArrayNode", context->roots.astNodeType,
        "elements", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);

    context->roots.astMessageSendNodeType = tuuvm_context_createIntrinsicClass(context, "ASTMessageSendNode", context->roots.astNodeType,
        "receiver", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "receiverLookupType", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "selector", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "arguments", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "boundMethod", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        NULL);
    context->roots.astMessageChainNodeType = tuuvm_context_createIntrinsicClass(context, "ASTMessageChainNode", context->roots.astNodeType,
        "receiver", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "receiverLookupType", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "messages", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.astMessageChainMessageNodeType = tuuvm_context_createIntrinsicClass(context, "ASTMessageChainMessageNode", context->roots.astNodeType,
        "selector", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "arguments", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.astObjectWithLookupStartingFromNodeType = tuuvm_context_createIntrinsicClass(context, "ASTObjectWithLookupStartingFromNode", context->roots.astNodeType,
        "objectExpression", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "typeExpression", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);
    context->roots.astPragmaNodeType = tuuvm_context_createIntrinsicClass(context, "ASTPragmaNode", context->roots.astNodeType,
        "selector", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "arguments", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.astReturnNodeType = tuuvm_context_createIntrinsicClass(context, "ASTReturnNode", context->roots.astNodeType,
        "expression", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);
    context->roots.astSequenceNodeType = tuuvm_context_createIntrinsicClass(context, "ASTSequenceNode", context->roots.astNodeType,
        "pragmas", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "expressions", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.astUnexpandedApplicationNodeType = tuuvm_context_createIntrinsicClass(context, "ASTUnexpandedApplicationNode", context->roots.astNodeType,
        "functionOrMacroExpression", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "arguments", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.astUnexpandedSExpressionNodeType = tuuvm_context_createIntrinsicClass(context, "ASTUnexpandedSExpressionNode", context->roots.astNodeType,
        "elements", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.astWhileContinueWithNodeType = tuuvm_context_createIntrinsicClass(context, "ASTWhileContinueWithNode", context->roots.astNodeType,
        "conditionExpression", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "bodyExpression", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "continueExpression", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);

    context->roots.astQuoteNodeType = tuuvm_context_createIntrinsicClass(context, "ASTQuoteNode", context->roots.astNodeType,
        "node", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);
    context->roots.astQuasiQuoteNodeType = tuuvm_context_createIntrinsicClass(context, "ASTQuasiQuoteNode", context->roots.astNodeType,
        "node", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);
    context->roots.astQuasiUnquoteNodeType = tuuvm_context_createIntrinsicClass(context, "ASTQuasiUnquoteNode", context->roots.astNodeType,
        "expression", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "astTemplateParameterIndex", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        NULL);
    context->roots.astSpliceNodeType = tuuvm_context_createIntrinsicClass(context, "ASTSpliceNode", context->roots.astNodeType,
        "expression", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "astTemplateParameterIndex", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        NULL);

    context->roots.bytecodeCompilerInstructionOperandType = tuuvm_context_createIntrinsicClass(context, "BootstrapBytecodeCompilerInstructionOperand", context->roots.objectType,
        NULL);
    context->roots.bytecodeCompilerInstructionType = tuuvm_context_createIntrinsicClass(context, "BootstrapBytecodeCompilerInstruction", context->roots.objectType,
        "pc", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "endPC", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "opcode", TUUVM_TYPE_SLOT_FLAG_PUBLIC, NULL,
        "operands", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,

        "sourcePosition", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sourcePositionType,
        "sourceEnvironment", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.environmentType,
        "sourceASTNode", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,

        NULL);
    context->roots.bytecodeCompilerInstructionVectorOperandType = tuuvm_context_createIntrinsicClass(context, "BootstrapBytecodeCompilerInstructionVectorOperand", context->roots.bytecodeCompilerInstructionOperandType,
        "index", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.int16Type,
        "vectorType", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.int16Type,

        "hasAllocaDestination", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "hasNonAllocaDestination", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "hasLoadStoreUsage", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "hasNonLoadStoreUsage", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        NULL);
    context->roots.bytecodeCompilerType = tuuvm_context_createIntrinsicClass(context, "BootstrapBytecodeCompiler", context->roots.objectType,
        "arguments", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "captures", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "literals", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayListType,
        "literalDictionary", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.identityDictionaryType,
        "temporaries", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayListType,
        "usedTemporaryCount", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,

        "firstInstruction", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.bytecodeCompilerInstructionType,
        "lastInstruction", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.bytecodeCompilerInstructionType,

        "breakLabel", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.bytecodeCompilerInstructionOperandType,
        "continueLabel", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.bytecodeCompilerInstructionOperandType,

        "sourcePosition", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sourcePositionType,
        "sourceEnvironment", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.environmentType,
        "sourceASTNode", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,

        "bindingDictionary", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.dictionaryType,
        NULL);
    
    // Fill the immediate type table.
    context->roots.immediateTypeTable[TUUVM_TUPLE_TAG_NIL] = context->roots.undefinedObjectType;
    context->roots.immediateTypeTable[TUUVM_TUPLE_TAG_INTEGER] = context->roots.integerType;
    context->roots.immediateTypeTable[TUUVM_TUPLE_TAG_CHAR8] = context->roots.char8Type;
    context->roots.immediateTypeTable[TUUVM_TUPLE_TAG_UINT8] = context->roots.uint8Type;
    context->roots.immediateTypeTable[TUUVM_TUPLE_TAG_INT8] = context->roots.int8Type;
    context->roots.immediateTypeTable[TUUVM_TUPLE_TAG_CHAR16] = context->roots.char16Type;
    context->roots.immediateTypeTable[TUUVM_TUPLE_TAG_UINT16] = context->roots.uint16Type;
    context->roots.immediateTypeTable[TUUVM_TUPLE_TAG_INT16] = context->roots.int16Type;
    context->roots.immediateTypeTable[TUUVM_TUPLE_TAG_CHAR32] = context->roots.char32Type;
    context->roots.immediateTypeTable[TUUVM_TUPLE_TAG_UINT32] = context->roots.uint32Type;
    context->roots.immediateTypeTable[TUUVM_TUPLE_TAG_INT32] = context->roots.int32Type;
    context->roots.immediateTypeTable[TUUVM_TUPLE_TAG_UINT64] = context->roots.uint64Type;
    context->roots.immediateTypeTable[TUUVM_TUPLE_TAG_INT64] = context->roots.int64Type;
    context->roots.immediateTypeTable[TUUVM_TUPLE_TAG_FLOAT32] = context->roots.float32Type;
    context->roots.immediateTypeTable[TUUVM_TUPLE_TAG_FLOAT64] = context->roots.float64Type;
    context->roots.immediateTypeTable[TUUVM_TUPLE_TAG_TRIVIAL] = context->roots.undefinedObjectType;

    // Fill the immediate trivial type table.
    context->roots.immediateTrivialTypeTable[TUUVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_FALSE] = context->roots.falseType;
    context->roots.immediateTrivialTypeTable[TUUVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_TRUE] = context->roots.trueType;
    context->roots.immediateTrivialTypeTable[TUUVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_VOID] = context->roots.voidType;
    context->roots.immediateTrivialTypeTable[TUUVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_HASHTABLE_EMPTY_ELEMENT] = context->roots.hashtableEmptyType;
    context->roots.immediateTrivialTypeTable[TUUVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_TOMBSTONE] = context->roots.tombstoneType;
    context->roots.immediateTrivialTypeTable[TUUVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_PENDING_MEMOIZATION_VALUE] = context->roots.pendingMemoizationValueType;
}

TUUVM_API tuuvm_context_t *tuuvm_context_create(void)
{
    tuuvm_context_t *context = (tuuvm_context_t*)calloc(1, sizeof(tuuvm_context_t));
    context->targetWordSize = sizeof(void*);
    context->identityHashSeed = 1;
    context->jitEnabled = tuuvm_context_default_jitEnabled;
    tuuvm_heap_initialize(&context->heap);
    tuuvm_gc_lock(context);

    tuuvm_context_createBasicTypes(context);
    
    tuuvm_array_setupPrimitives(context);
    tuuvm_arrayList_setupPrimitives(context);
    tuuvm_astInterpreter_setupASTInterpreter(context);
    tuuvm_boolean_setupPrimitives(context);
    tuuvm_bytecode_setupPrimitives(context);
    tuuvm_bytecodeCompiler_setupPrimitives(context);
    tuuvm_dictionary_setupPrimitives(context);
    tuuvm_errors_setupPrimitives(context);
    tuuvm_environment_setupPrimitives(context);
    tuuvm_filesystem_setupPrimitives(context);
    tuuvm_float_setupPrimitives(context);
    tuuvm_function_setupPrimitives(context);
    tuuvm_integer_setupPrimitives(context);
    tuuvm_io_setupPrimitives(context);
    tuuvm_primitiveInteger_setupPrimitives(context);
    tuuvm_string_setupPrimitives(context);
    tuuvm_stringStream_setupPrimitives(context);
    tuuvm_time_setupPrimitives(context);
    tuuvm_tuple_setupPrimitives(context);
    tuuvm_type_setupPrimitives(context);
    
    tuuvm_gc_unlock(context);

    return context;
}

TUUVM_API void tuuvm_context_destroy(tuuvm_context_t *context)
{
    if(!context) return;

    // Destroy the context heap.
    tuuvm_heap_destroy(&context->heap);
    free(context);
}

TUUVM_API tuuvm_context_t *tuuvm_context_loadImageFromFileNamed(const char *filename)
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

    tuuvm_context_t *context = (tuuvm_context_t*)calloc(1, sizeof(tuuvm_context_t));
    if(fread(&context->targetWordSize, sizeof(context->targetWordSize), 1, inputFile) != 1 ||
        fread(&context->identityHashSeed, sizeof(context->identityHashSeed), 1, inputFile) != 1 ||
        fread(&context->roots, sizeof(context->roots), 1, inputFile) != 1 ||
        !tuuvm_heap_loadFromFile(&context->heap, inputFile, sizeof(context->roots) / sizeof(tuuvm_tuple_t), (tuuvm_tuple_t*)&context->roots))
    {
        fclose(inputFile);
        free(context);
        return NULL;
    }
    
    context->jitEnabled = tuuvm_context_default_jitEnabled;

    fclose(inputFile);

    context->roots.sessionToken = tuuvm_tuple_systemHandle_encode(context, tuuvm_tuple_systemHandle_decode(context->roots.sessionToken) +  1);
    return context;
}

TUUVM_API void tuuvm_context_saveImageToFileNamed(tuuvm_context_t *context, const char *filename)
{
    tuuvm_gc_collect(context);
    FILE *outputFile = fopen(filename, "wb");
    fwrite("TVIM", 4, 1, outputFile);
    fwrite(&context->targetWordSize, sizeof(context->targetWordSize), 1, outputFile);
    fwrite(&context->identityHashSeed, sizeof(context->identityHashSeed), 1, outputFile);
    fwrite(&context->roots, sizeof(context->roots), 1, outputFile);
    tuuvm_heap_dumpToFile(&context->heap, outputFile);
    fclose(outputFile);
}

tuuvm_heap_t *tuuvm_context_getHeap(tuuvm_context_t *context)
{
    if(!context) return 0;

    return &context->heap;
}

static size_t tuuvm_context_generateIdentityHash(tuuvm_context_t *context)
{
    context->identityHashSeed = tuuvm_hashMultiply(context->identityHashSeed) + 12345;
    return context->identityHashSeed & TUUVM_HASH_BIT_MASK;
}

tuuvm_object_tuple_t *tuuvm_context_allocateByteTuple(tuuvm_context_t *context, tuuvm_tuple_t type, size_t byteSize)
{
    if(!context) return 0;

    tuuvm_object_tuple_t *result = tuuvm_heap_allocateByteTuple(&context->heap, byteSize);
    tuuvm_tuple_setIdentityHash(result, tuuvm_context_generateIdentityHash(context));
    if(result)
        tuuvm_tuple_setType(result, type);
    return result;
}

tuuvm_object_tuple_t *tuuvm_context_allocatePointerTuple(tuuvm_context_t *context, tuuvm_tuple_t type, size_t slotCount)
{
    if(!context) return 0;

    tuuvm_object_tuple_t *result = tuuvm_heap_allocatePointerTuple(&context->heap, slotCount);
    tuuvm_tuple_setIdentityHash(result, tuuvm_context_generateIdentityHash(context));
    if(result)
        tuuvm_tuple_setType(result, type);
    return result;
}

TUUVM_API tuuvm_tuple_t tuuvm_context_shallowCopy(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    if(!tuuvm_tuple_isNonNullPointer(tuple))
        return tuple;

    tuuvm_object_tuple_t *result = tuuvm_heap_shallowCopyTuple(&context->heap, (tuuvm_object_tuple_t*)tuple);
    tuuvm_tuple_setIdentityHash(result, tuuvm_context_generateIdentityHash(context));
    return (tuuvm_tuple_t)result;    
}
