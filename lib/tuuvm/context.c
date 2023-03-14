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

extern void tuuvm_arrayList_registerPrimitives(void);
extern void tuuvm_astInterpreter_registerPrimitives(void);
extern void tuuvm_boolean_registerPrimitives(void);
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
extern void tuuvm_tuple_registerPrimitives(void);
extern void tuuvm_type_registerPrimitives(void);

extern void tuuvm_arrayList_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_astInterpreter_setupASTInterpreter(tuuvm_context_t *context);
extern void tuuvm_boolean_setupPrimitives(tuuvm_context_t *context);
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
extern void tuuvm_tuple_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_type_setupPrimitives(tuuvm_context_t *context);

void tuuvm_context_registerPrimitives(void)
{
    tuuvm_primitiveTable_registerFunction(tuuvm_tuple_primitive_identityEquals);
    tuuvm_primitiveTable_registerFunction(tuuvm_tuple_primitive_identityNotEquals);
    tuuvm_primitiveTable_registerFunction(tuuvm_tuple_primitive_identityHash);
    tuuvm_primitiveTable_registerFunction(tuuvm_string_primitive_equals);
    tuuvm_primitiveTable_registerFunction(tuuvm_string_primitive_hash);

    tuuvm_arrayList_registerPrimitives();
    tuuvm_astInterpreter_registerPrimitives();
    tuuvm_boolean_registerPrimitives();
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
    tuuvm_tuple_registerPrimitives();
    tuuvm_type_registerPrimitives();
}

TUUVM_API tuuvm_tuple_t tuuvm_context_createIntrinsicClass(tuuvm_context_t *context, const char *name, tuuvm_tuple_t supertype, ...)
{
    tuuvm_tuple_t nameSymbol = tuuvm_symbol_internWithCString(context, name);
    tuuvm_tuple_t type = tuuvm_type_createAnonymousClassAndMetaclass(context, supertype);
    tuuvm_type_setName(type, nameSymbol);
    tuuvm_environment_setNewSymbolBindingWithValue(context, context->roots.intrinsicsBuiltInEnvironment, nameSymbol, type);
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

TUUVM_API tuuvm_tuple_t tuuvm_context_createIntrinsicType(tuuvm_context_t *context, const char *name, tuuvm_tuple_t supertype, ...)
{
    tuuvm_tuple_t nameSymbol = tuuvm_symbol_internWithCString(context, name);
    tuuvm_tuple_t type = tuuvm_type_createWithName(context, nameSymbol);
    if(supertype)
        tuuvm_type_setSupertype(type, supertype);
    tuuvm_environment_setNewSymbolBindingWithValue(context, context->roots.intrinsicsBuiltInEnvironment, nameSymbol, type);
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
    tuuvm_environment_setNewSymbolBindingWithValue(context, context->roots.intrinsicsBuiltInEnvironment, nameSymbol, type);
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
    tuuvm_environment_setNewSymbolBindingWithValue(context, context->roots.intrinsicsBuiltInEnvironment, symbol, value);

    if(tuuvm_tuple_isKindOf(context, value, context->roots.functionType))
    {
        tuuvm_function_t *functionObject = (tuuvm_function_t*)value;
        if(!functionObject->owner && !functionObject->name)
            functionObject->name = symbol;
    }
}

TUUVM_API void tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(tuuvm_context_t *context, const char *symbolString, size_t argumentCount, size_t flags, void *userdata, tuuvm_functionEntryPoint_t entryPoint)
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
}

TUUVM_API void tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(tuuvm_context_t *context, const char *symbolString, tuuvm_tuple_t ownerClass, const char *selectorString, size_t argumentCount, size_t flags, void *userdata, tuuvm_functionEntryPoint_t entryPoint)
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
    gcFrame.symbol = tuuvm_symbol_internWithCString(context, symbolString);
    gcFrame.selector = tuuvm_symbol_internWithCString(context, selectorString);
    gcFrame.primitiveFunction = tuuvm_function_createPrimitive(context, argumentCount, flags, userdata, entryPoint);
    tuuvm_type_setMethodWithSelector(context, gcFrame.ownerClass, gcFrame.selector, gcFrame.primitiveFunction);
    tuuvm_context_setIntrinsicSymbolBindingValue(context, gcFrame.symbol, gcFrame.primitiveFunction);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
}

static void tuuvm_context_createBasicTypes(tuuvm_context_t *context)
{
    // Make a circular base type.
    context->roots.anyValueType = tuuvm_type_createAnonymous(context);
    context->roots.typeType = tuuvm_type_createAnonymous(context);
    tuuvm_tuple_setType((tuuvm_object_tuple_t*)context->roots.typeType, context->roots.typeType);
    tuuvm_tuple_setType((tuuvm_object_tuple_t*)context->roots.anyValueType, context->roots.typeType);

    tuuvm_type_setFlags(context, context->roots.anyValueType, TUUVM_TYPE_FLAGS_NULLABLE);
    tuuvm_type_setFlags(context, context->roots.typeType, TUUVM_TYPE_FLAGS_NULLABLE);

    context->roots.objectType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.anyValueType);
    tuuvm_type_setSupertype(context->roots.typeType, context->roots.objectType);

    context->roots.classType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.typeType);
    context->roots.metaclassType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.typeType);

    tuuvm_tuple_setType((tuuvm_object_tuple_t*)tuuvm_tuple_getType(context, context->roots.objectType), context->roots.metaclassType);
    tuuvm_type_setSupertype(tuuvm_tuple_getType(context, context->roots.objectType), context->roots.classType);

    tuuvm_tuple_setType((tuuvm_object_tuple_t*)tuuvm_tuple_getType(context, context->roots.classType), context->roots.metaclassType);

    tuuvm_tuple_setType((tuuvm_object_tuple_t*)tuuvm_tuple_getType(context, context->roots.metaclassType), context->roots.metaclassType);

    // Create the type slot class.
    context->roots.typeSlotType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.typeSlotType);

    // Create the function class.
    context->roots.functionType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    context->roots.functionDefinitionType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);

    context->roots.symbolBindingType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    context->roots.symbolAnalysisBindingType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.symbolBindingType);
    context->roots.symbolArgumentBindingType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.symbolAnalysisBindingType);
    context->roots.symbolCaptureBindingType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.symbolAnalysisBindingType);
    context->roots.symbolLocalBindingType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.symbolAnalysisBindingType);
    context->roots.symbolMacroValueBindingType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.symbolBindingType);
    context->roots.symbolValueBindingType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.symbolBindingType);
    context->roots.environmentType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
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
    context->roots.identityDictionaryType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.dictionaryType);
    context->roots.methodDictionaryType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.hashedCollectionType);
    context->roots.setType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.hashedCollectionType);
    context->roots.identitySetType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.setType);

    context->roots.internedSymbolSet = tuuvm_identitySet_create(context);

    // Create the intrinsic built-in environment
    context->roots.arrayType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.arrayedCollectionType);
    context->roots.arrayListType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.sequenceableCollectionType);

    context->roots.intrinsicsBuiltInEnvironment = tuuvm_environment_create(context, TUUVM_NULL_TUPLE);
    context->roots.intrinsicTypes = tuuvm_arrayList_create(context);

    context->roots.equalsSelector = tuuvm_symbol_internWithCString(context, "=");
    context->roots.hashSelector = tuuvm_symbol_internWithCString(context, "hash");
    context->roots.asStringSelector = tuuvm_symbol_internWithCString(context, "asString");
    context->roots.printStringSelector = tuuvm_symbol_internWithCString(context, "printString");
    context->roots.doesNotUnderstandSelector = tuuvm_symbol_internWithCString(context, "doesNotUnderstand:");

    context->roots.astNodeAnalysisSelector = tuuvm_symbol_internWithCString(context, "astAnalyzeWithEnvironment:");
    context->roots.astNodeEvaluationSelector = tuuvm_symbol_internWithCString(context, "astEvaluateWithEnvironment:");
    context->roots.astNodeAnalysisAndEvaluationSelector = tuuvm_symbol_internWithCString(context, "astAnalyzeAndEvaluateWithEnvironment:");
    
    context->roots.analyzeAndEvaluateMessageSendNodeForReceiverWithEnvironmentSelector = tuuvm_symbol_internWithCString(context, "analyzeAndEvaluateMessageSendNode:forReceiver:withEnvironment:");
    context->roots.analyzeMessageSendNodeWithEnvironmentSelector = tuuvm_symbol_internWithCString(context, "analyzeMessageSendNode:withEnvironment:");
    context->roots.analyzeAndEvaluateMessageChainNodeForReceiverWithEnvironmentSelector = tuuvm_symbol_internWithCString(context, "analyzeAndEvaluateMessageChainNode:forReceiver:withEnvironment:");
    context->roots.analyzeMessageChainNodeWithEnvironmentSelector = tuuvm_symbol_internWithCString(context, "analyzeMessageChainNode:withEnvironment:");
    context->roots.analyzeAndEvaluateConcreteMetaValueWithEnvironmentSelector = tuuvm_symbol_internWithCString(context, "analyzeAndEvaluateConcreteMetaValue:withEnvironment:");
    context->roots.analyzeConcreteMetaValueWithEnvironmentSelector = tuuvm_symbol_internWithCString(context, "analyzeConcreteMetaValue:withEnvironment:");

    context->roots.coerceValueSelector = tuuvm_symbol_internWithCString(context, "coerceValue:");

    tuuvm_context_setIntrinsicSymbolBindingValue(context, tuuvm_symbol_internWithCString(context, "nil"), TUUVM_NULL_TUPLE);
    tuuvm_context_setIntrinsicSymbolBindingValue(context, tuuvm_symbol_internWithCString(context, "false"), TUUVM_FALSE_TUPLE);
    tuuvm_context_setIntrinsicSymbolBindingValue(context, tuuvm_symbol_internWithCString(context, "true"), TUUVM_TRUE_TUPLE);
    tuuvm_context_setIntrinsicSymbolBindingValue(context, tuuvm_symbol_internWithCString(context, "void"), TUUVM_VOID_TUPLE);

    tuuvm_context_setIntrinsicSymbolBindingValue(context, tuuvm_symbol_internWithCString(context, "BootstrapEnv::IntrinsicsBuiltInEnvironment"), context->roots.intrinsicsBuiltInEnvironment);
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

    // Some basic types
    context->roots.voidType = tuuvm_context_createIntrinsicClass(context, "Void", TUUVM_NULL_TUPLE, NULL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.voidType, TUUVM_TYPE_FLAGS_IMMEDIATE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);

    context->roots.char8Type = tuuvm_context_createIntrinsicClass(context, "Char8", TUUVM_NULL_TUPLE, NULL);
    context->roots.uint8Type = tuuvm_context_createIntrinsicClass(context, "UInt8", TUUVM_NULL_TUPLE, NULL);
    context->roots.int8Type = tuuvm_context_createIntrinsicClass(context, "Int8", TUUVM_NULL_TUPLE, NULL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.char8Type, TUUVM_TYPE_FLAGS_IMMEDIATE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.uint8Type, TUUVM_TYPE_FLAGS_IMMEDIATE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.int8Type, TUUVM_TYPE_FLAGS_IMMEDIATE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);

    context->roots.char16Type = tuuvm_context_createIntrinsicClass(context, "Char16", TUUVM_NULL_TUPLE, NULL);
    context->roots.uint16Type = tuuvm_context_createIntrinsicClass(context, "UInt16", TUUVM_NULL_TUPLE, NULL);
    context->roots.int16Type = tuuvm_context_createIntrinsicClass(context, "Int16", TUUVM_NULL_TUPLE, NULL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.char16Type, TUUVM_TYPE_FLAGS_IMMEDIATE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.uint16Type, TUUVM_TYPE_FLAGS_IMMEDIATE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.int16Type, TUUVM_TYPE_FLAGS_IMMEDIATE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);

    context->roots.char32Type = tuuvm_context_createIntrinsicClass(context, "Char32", TUUVM_NULL_TUPLE, NULL);
    context->roots.uint32Type = tuuvm_context_createIntrinsicClass(context, "UInt32", TUUVM_NULL_TUPLE, NULL);
    context->roots.int32Type = tuuvm_context_createIntrinsicClass(context, "Int32", TUUVM_NULL_TUPLE, NULL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.char32Type, TUUVM_TYPE_FLAGS_IMMEDIATE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.uint32Type, TUUVM_TYPE_FLAGS_IMMEDIATE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.int32Type, TUUVM_TYPE_FLAGS_IMMEDIATE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);

    context->roots.uint64Type = tuuvm_context_createIntrinsicClass(context, "UInt64", TUUVM_NULL_TUPLE, NULL);
    context->roots.int64Type = tuuvm_context_createIntrinsicClass(context, "Int64", TUUVM_NULL_TUPLE, NULL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.uint64Type, TUUVM_TYPE_FLAGS_IMMEDIATE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.int64Type, TUUVM_TYPE_FLAGS_IMMEDIATE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);

    context->roots.float32Type = tuuvm_context_createIntrinsicClass(context, "Float32", TUUVM_NULL_TUPLE, NULL);
    context->roots.float64Type = tuuvm_context_createIntrinsicClass(context, "Float64", TUUVM_NULL_TUPLE, NULL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.float32Type, TUUVM_TYPE_FLAGS_IMMEDIATE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.float64Type, TUUVM_TYPE_FLAGS_IMMEDIATE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);

    context->roots.sizeType = sizeof(size_t) == 4 ? context->roots.uint32Type : context->roots.uint64Type;
    context->roots.uintptrType = sizeof(size_t) == 4 ? context->roots.uint32Type : context->roots.uint64Type;
    context->roots.intptrType = sizeof(size_t) == 4 ? context->roots.int32Type : context->roots.int64Type;

    context->roots.booleanType = tuuvm_context_createIntrinsicClass(context, "Boolean", TUUVM_NULL_TUPLE, NULL);
    tuuvm_type_setFlags(context, context->roots.booleanType, TUUVM_TYPE_FLAGS_IMMEDIATE);
    context->roots.trueType = tuuvm_context_createIntrinsicClass(context, "True", context->roots.booleanType, NULL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.trueType, TUUVM_TYPE_FLAGS_IMMEDIATE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);
    context->roots.falseType = tuuvm_context_createIntrinsicClass(context, "False", context->roots.booleanType, NULL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.falseType, TUUVM_TYPE_FLAGS_IMMEDIATE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);
    context->roots.integerType = tuuvm_context_createIntrinsicClass(context, "Integer", TUUVM_NULL_TUPLE, NULL);
    context->roots.positiveIntegerType = tuuvm_context_createIntrinsicClass(context, "PositiveInteger", context->roots.integerType, NULL);
    context->roots.negativeIntegerType = tuuvm_context_createIntrinsicClass(context, "NegativeInteger", context->roots.integerType, NULL);
    context->roots.undefinedObjectType = tuuvm_context_createIntrinsicClass(context, "UndefinedObject", TUUVM_NULL_TUPLE, NULL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.undefinedObjectType, TUUVM_TYPE_FLAGS_NULLABLE | TUUVM_TYPE_FLAGS_IMMEDIATE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);
    context->roots.stringType = tuuvm_context_createIntrinsicClass(context, "String", context->roots.arrayedCollectionType, NULL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.stringType, TUUVM_TYPE_FLAGS_NULLABLE | TUUVM_TYPE_FLAGS_BYTES | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);

    // Set the name of the root basic type.
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.anyValueType, "AnyValue", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.objectType, "Object", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.typeType, "Type", TUUVM_NULL_TUPLE,
        "name", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolType,
        "owner", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "supertype", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeType,
        "slots", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "totalSlotCount", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "flags", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        
        "macroMethodDictionary", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "methodDictionary", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "fallbackMethodDictionary", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,

        "pendingSlots", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.classType, "Class", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.metaclassType, "Metaclass", TUUVM_NULL_TUPLE,
        "thisClass", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.typeSlotType, "TypeSlot", TUUVM_NULL_TUPLE,
        "name", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "flags", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "type", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        NULL);

    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.environmentType, "Environment", TUUVM_NULL_TUPLE,
        "parent", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.environmentType,
        "symbolTable", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "returnTarget", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "breakTarget", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "continueTarget", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        NULL);
    context->roots.analysisEnvironmentType = tuuvm_context_createIntrinsicClass(context, "AnalysisEnvironment", context->roots.environmentType,
        NULL);
    context->roots.functionActivationEnvironmentType = tuuvm_context_createIntrinsicClass(context, "FunctionActivationEnvironment", context->roots.environmentType,
        "function", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionType,
        "functionDefinition", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionDefinitionType,
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
        "flags", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "argumentCount", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "captureVector", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "definition", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionDefinitionType,
        "primitiveTableIndex", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "primitiveName", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolType,
        "nativeUserdata", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "nativeEntryPoint", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.uintptrType,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.functionDefinitionType, "FunctionDefinition", TUUVM_NULL_TUPLE,
        "flags", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "argumentCount", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "sourcePosition", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,

        "definitionEnvironment", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.environmentType,
        "definitionArgumentNodes", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "definitionResultTypeNode", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "definitionBodyNode", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,

        "analysisEnvironment", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.environmentType,
        "analyzedCaptures", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "analyzedArguments", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "analyzedLocals", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,

        "analyzedArgumentNodes", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "analyzedResultTypeNode", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "analyzedBodyNode", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolType, "Symbol", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.stringSymbolType, "StringSymbol", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.setType, "Set", TUUVM_NULL_TUPLE,
        "size", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "storage", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.identitySetType, "IdentitySet", TUUVM_NULL_TUPLE,
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
    tuuvm_typeAndMetatype_setFlags(context, context->roots.arrayType, TUUVM_TYPE_FLAGS_NULLABLE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.arrayListType, TUUVM_TYPE_FLAGS_NULLABLE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);

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
    tuuvm_typeAndMetatype_setFlags(context, context->roots.associationType, TUUVM_TYPE_FLAGS_NULLABLE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);
    context->roots.byteArrayType = tuuvm_context_createIntrinsicClass(context, "ByteArray", context->roots.arrayedCollectionType, NULL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.byteArrayType, TUUVM_TYPE_FLAGS_NULLABLE | TUUVM_TYPE_FLAGS_BYTES | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.dictionaryType, "Dictionary", TUUVM_NULL_TUPLE,
        "size", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "storage", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.identityDictionaryType, "IdentityDictionary", TUUVM_NULL_TUPLE,
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.methodDictionaryType, "MethodDictionary", TUUVM_NULL_TUPLE,
        "size", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "storage", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.generatedSymbolType = tuuvm_context_createIntrinsicClass(context, "GeneratedSymbol", TUUVM_NULL_TUPLE,
        "value", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolType,
        "context", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        NULL);
    context->roots.hashtableEmptyType = tuuvm_context_createIntrinsicClass(context, "HashtableEmpty", TUUVM_NULL_TUPLE, NULL);
    context->roots.macroContextType = tuuvm_context_createIntrinsicClass(context, "MacroContext", TUUVM_NULL_TUPLE,
        "sourceNode", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "sourcePosition", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        NULL);
    context->roots.messageType = tuuvm_context_createIntrinsicClass(context, "Message", TUUVM_NULL_TUPLE,
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
        "value", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        NULL);
    tuuvm_typeAndMetatype_setFlags(context, context->roots.valueBoxType, TUUVM_TYPE_FLAGS_NULLABLE | TUUVM_TYPE_FLAGS_FINAL, TUUVM_TYPE_FLAGS_FINAL);

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
        NULL);
    context->roots.astLambdaNodeType = tuuvm_context_createIntrinsicClass(context, "ASTLambdaNode", context->roots.astNodeType,
        "flags", TUUVM_TYPE_SLOT_FLAG_PUBLIC, TUUVM_NULL_TUPLE,
        "arguments", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "resultType", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "body", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "hasLazyAnalysis", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "functionDefinition", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionDefinitionType,
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
    context->roots.astMakeTupleNodeType = tuuvm_context_createIntrinsicClass(context, "ASTMakeTupleNode", context->roots.astNodeType,
        "elements", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);

    context->roots.astMessageSendNodeType = tuuvm_context_createIntrinsicClass(context, "ASTMessageSendNode", context->roots.astNodeType,
        "receiver", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "receiverLookupType", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "selector", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "arguments", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
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
        NULL);
    context->roots.astSpliceNodeType = tuuvm_context_createIntrinsicClass(context, "ASTSpliceNode", context->roots.astNodeType,
        "expression", TUUVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
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
}

TUUVM_API tuuvm_context_t *tuuvm_context_create(void)
{
    tuuvm_context_t *context = (tuuvm_context_t*)calloc(1, sizeof(tuuvm_context_t));
    context->identityHashSeed = 1;
    tuuvm_heap_initialize(&context->heap);
    tuuvm_gc_lock(context);

    tuuvm_context_createBasicTypes(context);
    
    tuuvm_arrayList_setupPrimitives(context);
    tuuvm_astInterpreter_setupASTInterpreter(context);
    tuuvm_boolean_setupPrimitives(context);
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
    fread(magic, 4, 1, inputFile);
    if(memcmp(magic, "TVIM", 4))
    {
        fclose(inputFile);
        return NULL;
    }

    tuuvm_context_t *context = (tuuvm_context_t*)calloc(1, sizeof(tuuvm_context_t));
    fread(&context->identityHashSeed, sizeof(context->identityHashSeed), 1, inputFile);
    fread(&context->roots, sizeof(context->roots), 1, inputFile);
    tuuvm_heap_loadFromFile(&context->heap, inputFile, sizeof(context->roots) / sizeof(tuuvm_tuple_t), (tuuvm_tuple_t*)&context->roots);
    fclose(inputFile);
    return context;
}

TUUVM_API void tuuvm_context_saveImageToFileNamed(tuuvm_context_t *context, const char *filename)
{
    tuuvm_gc_collect(context);
    FILE *outputFile = fopen(filename, "wb");
    fwrite("TVIM", 4, 1, outputFile);
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
    return context->identityHashSeed & TUUVM_TUPLE_IMMEDIATE_VALUE_BIT_MASK;
}

tuuvm_object_tuple_t *tuuvm_context_allocateByteTuple(tuuvm_context_t *context, tuuvm_tuple_t type, size_t byteSize)
{
    if(!context) return 0;

    tuuvm_object_tuple_t *result = tuuvm_heap_allocateByteTuple(&context->heap, byteSize);
    result->header.identityHash = tuuvm_context_generateIdentityHash(context);
    if(result)
        tuuvm_tuple_setType(result, type);
    return result;
}

tuuvm_object_tuple_t *tuuvm_context_allocatePointerTuple(tuuvm_context_t *context, tuuvm_tuple_t type, size_t slotCount)
{
    if(!context) return 0;

    tuuvm_object_tuple_t *result = tuuvm_heap_allocatePointerTuple(&context->heap, slotCount);
    result->header.identityHash = tuuvm_context_generateIdentityHash(context);
    if(result)
        tuuvm_tuple_setType(result, type);
    return result;
}

TUUVM_API tuuvm_tuple_t tuuvm_context_shallowCopy(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    if(!tuuvm_tuple_isNonNullPointer(tuple))
        return tuple;

    tuuvm_object_tuple_t *result = tuuvm_heap_shallowCopyTuple(&context->heap, (tuuvm_object_tuple_t*)tuple);
    result->header.identityHash = tuuvm_context_generateIdentityHash(context);
    return (tuuvm_tuple_t)result;    
}
