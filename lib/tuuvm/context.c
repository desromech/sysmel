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

extern void tuuvm_arrayList_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_astInterpreter_setupASTInterpreter(tuuvm_context_t *context);
extern void tuuvm_boolean_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_dictionary_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_errors_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_environment_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_function_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_integer_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_io_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_primitiveInteger_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_string_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_stringBuilder_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_tuple_setupPrimitives(tuuvm_context_t *context);

TUUVM_API tuuvm_tuple_t tuuvm_context_createIntrinsicClass(tuuvm_context_t *context, const char *name, tuuvm_tuple_t supertype, ...)
{
    tuuvm_tuple_t nameSymbol = tuuvm_symbol_internWithCString(context, name);
    tuuvm_tuple_t type = tuuvm_type_createAnonymousClassAndMetaclass(context, supertype);
    tuuvm_type_setName(type, nameSymbol);
    tuuvm_environment_setNewSymbolBinding(context, context->roots.intrinsicsBuiltInEnvironment, nameSymbol, type);
    tuuvm_arrayList_add(context, context->roots.intrinsicTypes, type);

    // First pass: count the arguments.
    size_t slotNameCount = 0;
    va_list valist;
    va_start(valist, supertype);
    while(va_arg(valist, const char *))
        ++slotNameCount;
    va_end(valist);

    // Second pass: make the argument list.
    tuuvm_tuple_t slotNames = tuuvm_array_create(context, slotNameCount);
    va_start(valist, supertype);
    for(size_t i = 0; i < slotNameCount; ++i)
        tuuvm_array_atPut(slotNames, i, tuuvm_symbol_internWithCString(context, va_arg(valist, const char *)));

    va_end(valist);
    tuuvm_type_setSlotNames(type, slotNames);

    // Set the total slot count.
    size_t totalSlotCount = slotNameCount;
    if(tuuvm_tuple_isNonNullPointer(supertype))
        totalSlotCount += tuuvm_tuple_size_decode(tuuvm_type_getTotalSlotCount(supertype));
    tuuvm_type_setTotalSlotCount(type, tuuvm_tuple_size_encode(context, totalSlotCount));

    return type;
}

TUUVM_API tuuvm_tuple_t tuuvm_context_createIntrinsicType(tuuvm_context_t *context, const char *name, tuuvm_tuple_t supertype, ...)
{
    tuuvm_tuple_t nameSymbol = tuuvm_symbol_internWithCString(context, name);
    tuuvm_tuple_t type = tuuvm_type_createWithName(context, nameSymbol);
    if(supertype)
        tuuvm_type_setSupertype(type, supertype);
    tuuvm_environment_setNewSymbolBinding(context, context->roots.intrinsicsBuiltInEnvironment, nameSymbol, type);
    tuuvm_arrayList_add(context, context->roots.intrinsicTypes, type);

    // First pass: count the arguments.
    size_t slotNameCount = 0;
    va_list valist;
    va_start(valist, supertype);
    while(va_arg(valist, const char *))
        ++slotNameCount;
    va_end(valist);

    // Second pass: make the argument list.
    tuuvm_tuple_t slotNames = tuuvm_array_create(context, slotNameCount);
    va_start(valist, supertype);
    for(size_t i = 0; i < slotNameCount; ++i)
        tuuvm_array_atPut(slotNames, i, tuuvm_symbol_internWithCString(context, va_arg(valist, const char *)));

    va_end(valist);
    tuuvm_type_setSlotNames(type, slotNames);

    // Set the total slot count.
    size_t totalSlotCount = slotNameCount;
    if(tuuvm_tuple_isNonNullPointer(supertype))
        totalSlotCount += tuuvm_tuple_size_decode(tuuvm_type_getTotalSlotCount(supertype));
    tuuvm_type_setTotalSlotCount(type, tuuvm_tuple_size_encode(context, totalSlotCount));

    return type;
}

static void tuuvm_context_setIntrinsicTypeMetadata(tuuvm_context_t *context, tuuvm_tuple_t type, const char *name, tuuvm_tuple_t supertype, ...)
{
    tuuvm_tuple_t nameSymbol = tuuvm_symbol_internWithCString(context, name);
    tuuvm_type_setName(type, nameSymbol);
    if(supertype)
        tuuvm_type_setSupertype(type, supertype);
    tuuvm_environment_setNewSymbolBinding(context, context->roots.intrinsicsBuiltInEnvironment, nameSymbol, type);
    tuuvm_arrayList_add(context, context->roots.intrinsicTypes, type);

    // First pass: count the arguments.
    size_t slotNameCount = 0;
    va_list valist;
    va_start(valist, supertype);
    while(va_arg(valist, const char *))
        ++slotNameCount;
    va_end(valist);

    // Second pass: make the argument list.
    tuuvm_tuple_t slotNames = tuuvm_array_create(context, slotNameCount);
    va_start(valist, supertype);
    for(size_t i = 0; i < slotNameCount; ++i)
        tuuvm_array_atPut(slotNames, i, tuuvm_symbol_internWithCString(context, va_arg(valist, const char *)));

    va_end(valist);
    tuuvm_type_setSlotNames(type, slotNames);

    // Set the total slot count.
    int64_t totalSlotCount = slotNameCount;
    if(tuuvm_tuple_isNonNullPointer(supertype))
        totalSlotCount += tuuvm_tuple_integer_decodeInt64(tuuvm_type_getTotalSlotCount(supertype));
    tuuvm_type_setTotalSlotCount(type, tuuvm_tuple_integer_encodeInt64(context, totalSlotCount));
}

TUUVM_API void tuuvm_context_setIntrinsicSymbolBinding(tuuvm_context_t *context, tuuvm_tuple_t symbol, tuuvm_tuple_t binding)
{
    tuuvm_environment_setNewSymbolBinding(context, context->roots.intrinsicsBuiltInEnvironment, symbol, binding);
}

TUUVM_API void tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveFunction(tuuvm_context_t *context, const char *symbolString, size_t argumentCount, size_t flags, void *userdata, tuuvm_functionEntryPoint_t entryPoint)
{
    struct {
        tuuvm_tuple_t symbol;
        tuuvm_tuple_t primitiveFunction;
    } gcFrame = {};

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.symbol = tuuvm_symbol_internWithCString(context, symbolString);
    gcFrame.primitiveFunction = tuuvm_function_createPrimitive(context, argumentCount, flags, userdata, entryPoint);
    tuuvm_context_setIntrinsicSymbolBinding(context, gcFrame.symbol, gcFrame.primitiveFunction);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
}

TUUVM_API void tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveMethod(tuuvm_context_t *context, const char *symbolString, tuuvm_tuple_t ownerClass, const char *selectorString, size_t argumentCount, size_t flags, void *userdata, tuuvm_functionEntryPoint_t entryPoint)
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
    tuuvm_context_setIntrinsicSymbolBinding(context, gcFrame.symbol, gcFrame.primitiveFunction);
    tuuvm_type_setMethodWithSelector(context, gcFrame.ownerClass, gcFrame.selector, gcFrame.primitiveFunction);
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
}

static void tuuvm_context_createBasicTypes(tuuvm_context_t *context)
{
    // Make a circular base type.
    context->roots.anyValueType = tuuvm_type_createAnonymous(context);
    context->roots.typeType = tuuvm_type_createAnonymous(context);
    tuuvm_tuple_setType((tuuvm_object_tuple_t*)context->roots.typeType, context->roots.typeType);
    tuuvm_tuple_setType((tuuvm_object_tuple_t*)context->roots.anyValueType, context->roots.typeType);

    context->roots.objectType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.anyValueType);
    tuuvm_type_setSupertype(context->roots.typeType, context->roots.objectType);

    context->roots.classType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.typeType);
    context->roots.metaclassType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.typeType);

    tuuvm_tuple_setType((tuuvm_object_tuple_t*)tuuvm_tuple_getType(context, context->roots.objectType), context->roots.metaclassType);
    tuuvm_type_setSupertype(tuuvm_tuple_getType(context, context->roots.objectType), context->roots.classType);

    tuuvm_tuple_setType((tuuvm_object_tuple_t*)tuuvm_tuple_getType(context, context->roots.classType), context->roots.metaclassType);

    tuuvm_tuple_setType((tuuvm_object_tuple_t*)tuuvm_tuple_getType(context, context->roots.metaclassType), context->roots.metaclassType);

    // Create the function class.
    context->roots.functionType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);


    // Collection base hierarchy
    context->roots.collectionType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    context->roots.hashedCollectionType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.collectionType);
    context->roots.sequenceableCollectionType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.collectionType);
    context->roots.arrayedCollectionType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.sequenceableCollectionType);

    // Create the basic hash functions.
    context->roots.identityEqualsFunction = tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_tuple_primitive_identityEquals);
    context->roots.identityNotEqualsFunction = tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_tuple_primitive_identityNotEquals);
    context->roots.identityHashFunction = tuuvm_function_createPrimitive(context, 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_tuple_primitive_identityHash);
    context->roots.stringEqualsFunction = tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_string_primitive_equals);
    context->roots.stringHashFunction = tuuvm_function_createPrimitive(context, 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_string_primitive_hash);

    context->roots.symbolType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.arrayedCollectionType);
    context->roots.setType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.hashedCollectionType);
    context->roots.internedSymbolSet = tuuvm_set_create(context, context->roots.stringEqualsFunction, context->roots.stringHashFunction);

    // Create the intrinsic built-in environment
    context->roots.environmentType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    context->roots.arrayType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.arrayedCollectionType);
    context->roots.arrayListType = tuuvm_type_createAnonymousClassAndMetaclass(context, context->roots.sequenceableCollectionType);

    context->roots.intrinsicsBuiltInEnvironment = tuuvm_environment_create(context, TUUVM_NULL_TUPLE);
    context->roots.intrinsicTypes = tuuvm_arrayList_create(context);

    context->roots.equalsSelector = tuuvm_symbol_internWithCString(context, "=");
    context->roots.hashSelector = tuuvm_symbol_internWithCString(context, "hash");
    context->roots.asStringSelector = tuuvm_symbol_internWithCString(context, "asString");
    context->roots.printStringSelector = tuuvm_symbol_internWithCString(context, "printString");

    context->roots.astNodeAnalysisSelector = tuuvm_symbol_internWithCString(context, "astAnalyzeWithEnvironment:");
    context->roots.astNodeEvaluationSelector = tuuvm_symbol_internWithCString(context, "astEvaluateWithEnvironment:");
    context->roots.astNodeAnalysisAndEvaluationSelector = tuuvm_symbol_internWithCString(context, "astAnalyzeAndEvaluateWithEnvironment:");

    context->roots.coerceValueSelector = tuuvm_symbol_internWithCString(context, "coerceValue:");

    tuuvm_context_setIntrinsicSymbolBinding(context, tuuvm_symbol_internWithCString(context, "nil"), TUUVM_NULL_TUPLE);
    tuuvm_context_setIntrinsicSymbolBinding(context, tuuvm_symbol_internWithCString(context, "false"), TUUVM_FALSE_TUPLE);
    tuuvm_context_setIntrinsicSymbolBinding(context, tuuvm_symbol_internWithCString(context, "true"), TUUVM_TRUE_TUPLE);
    tuuvm_context_setIntrinsicSymbolBinding(context, tuuvm_symbol_internWithCString(context, "void"), TUUVM_VOID_TUPLE);

    tuuvm_context_setIntrinsicSymbolBinding(context, tuuvm_symbol_internWithCString(context, "BootstrapEnv::IntrinsicsBuiltInEnvironment"), context->roots.intrinsicsBuiltInEnvironment);
    tuuvm_context_setIntrinsicSymbolBinding(context, tuuvm_symbol_internWithCString(context, "BootstrapEnv::IntrinsicTypes"), context->roots.intrinsicTypes);
    tuuvm_context_setIntrinsicSymbolBinding(context, tuuvm_symbol_internWithCString(context, "BootstrapEnv::InternedSymbolSet"), context->roots.internedSymbolSet);

    tuuvm_context_setIntrinsicSymbolBinding(context, tuuvm_symbol_internWithCString(context, "identityHash"), context->roots.identityHashFunction);
    tuuvm_context_setIntrinsicSymbolBinding(context, tuuvm_symbol_internWithCString(context, "=="), context->roots.identityEqualsFunction);
    tuuvm_context_setIntrinsicSymbolBinding(context, tuuvm_symbol_internWithCString(context, "~~"), context->roots.identityNotEqualsFunction);

    tuuvm_context_setIntrinsicSymbolBinding(context, tuuvm_symbol_internWithCString(context, "String::hash"), context->roots.stringHashFunction);
    tuuvm_context_setIntrinsicSymbolBinding(context, tuuvm_symbol_internWithCString(context, "String::equals:"), context->roots.stringEqualsFunction);

    // Some basic method
    tuuvm_type_setHashFunction(context, context->roots.anyValueType, context->roots.identityHashFunction);
    tuuvm_type_setEqualsFunction(context, context->roots.anyValueType, context->roots.identityEqualsFunction);
    
    tuuvm_type_setMethodWithSelector(context, context->roots.anyValueType, tuuvm_symbol_internWithCString(context, "identityHash"), context->roots.identityHashFunction);
    tuuvm_type_setMethodWithSelector(context, context->roots.anyValueType, tuuvm_symbol_internWithCString(context, "=="), context->roots.identityEqualsFunction);
    tuuvm_type_setMethodWithSelector(context, context->roots.anyValueType, tuuvm_symbol_internWithCString(context, "~~"), context->roots.identityNotEqualsFunction);

    // Set the name of the root basic type.
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.anyValueType, "AnyValue", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.objectType, "Object", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.typeType, "Type", TUUVM_NULL_TUPLE,
        "name", "supertype", "slotNames", "sumTypeAlternatives", "totalSlotCount", "flags",
        "macroMethodDictionary", "methodDictionary", "fallbackMethodDictionary",
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.classType, "Class", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.metaclassType, "Metaclass", TUUVM_NULL_TUPLE, "thisClass", NULL);

    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.environmentType, "Environment", TUUVM_NULL_TUPLE, "parent", "symbolTable", NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.functionType, "Function", TUUVM_NULL_TUPLE,
        "flags", "argumentCount",
        "sourcePosition", "closureEnvironment", "argumentNodes", "resultTypeNode", "body",
        "nativeUserdata", "nativeEntryPoint",
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolType, "Symbol", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.setType, "Set", TUUVM_NULL_TUPLE,
        "size", "storage", "equalsFunction", "hashFunction",
        NULL);

    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.collectionType, "Collection", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.hashedCollectionType, "HashedCollection", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.sequenceableCollectionType, "SequenceableCollection", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.arrayedCollectionType, "ArrayedCollection", TUUVM_NULL_TUPLE, NULL);

    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.arrayType, "Array", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.arrayListType, "ArrayList", TUUVM_NULL_TUPLE, "size", "storage", NULL);

    // Create other root basic types.
    context->roots.arraySliceType = tuuvm_context_createIntrinsicClass(context, "ArraySlice", context->roots.sequenceableCollectionType, "elements", "offset", "size", NULL);
    context->roots.associationType = tuuvm_context_createIntrinsicClass(context, "Association", TUUVM_NULL_TUPLE, "key", "value", NULL);
    context->roots.byteArrayType = tuuvm_context_createIntrinsicClass(context, "ByteArray", context->roots.arrayedCollectionType, NULL);
    context->roots.booleanType = tuuvm_context_createIntrinsicClass(context, "Boolean", TUUVM_NULL_TUPLE, NULL);
    context->roots.dictionaryType = tuuvm_context_createIntrinsicClass(context, "Dictionary", TUUVM_NULL_TUPLE,
        "size", "storage", "equalsFunction", "hashFunction",
        NULL);
    context->roots.falseType = tuuvm_context_createIntrinsicClass(context, "False", context->roots.booleanType, NULL);
    context->roots.hashtableEmptyType = tuuvm_context_createIntrinsicClass(context, "HashtableEmpty", TUUVM_NULL_TUPLE, NULL);
    context->roots.macroContextType = tuuvm_context_createIntrinsicClass(context, "MacroContext", TUUVM_NULL_TUPLE, "sourceNode", "sourcePosition", NULL);
    context->roots.integerType = tuuvm_context_createIntrinsicClass(context, "Integer", TUUVM_NULL_TUPLE, NULL);
    context->roots.positiveIntegerType = tuuvm_context_createIntrinsicClass(context, "PositiveInteger", context->roots.integerType, NULL);
    context->roots.negativeIntegerType = tuuvm_context_createIntrinsicClass(context, "NegativeInteger", context->roots.integerType, NULL);
    context->roots.nilType = tuuvm_context_createIntrinsicClass(context, "Nil", TUUVM_NULL_TUPLE, NULL);
    context->roots.stringType = tuuvm_context_createIntrinsicClass(context, "String", context->roots.arrayedCollectionType, NULL);
    context->roots.stringBuilderType = tuuvm_context_createIntrinsicClass(context, "StringBuilder", TUUVM_NULL_TUPLE, "size", "storage", NULL);
    context->roots.trueType = tuuvm_context_createIntrinsicClass(context, "True", context->roots.booleanType, NULL);
    context->roots.valueBoxType = tuuvm_context_createIntrinsicClass(context, "ValueBox", TUUVM_NULL_TUPLE, "value", NULL);
    context->roots.voidType = tuuvm_context_createIntrinsicClass(context, "Void", TUUVM_NULL_TUPLE, NULL);

    context->roots.char8Type = tuuvm_context_createIntrinsicClass(context, "Char8", TUUVM_NULL_TUPLE, NULL);
    context->roots.uint8Type = tuuvm_context_createIntrinsicClass(context, "UInt8", TUUVM_NULL_TUPLE, NULL);
    context->roots.int8Type = tuuvm_context_createIntrinsicClass(context, "Int8", TUUVM_NULL_TUPLE, NULL);

    context->roots.char16Type = tuuvm_context_createIntrinsicClass(context, "Char16", TUUVM_NULL_TUPLE, NULL);
    context->roots.uint16Type = tuuvm_context_createIntrinsicClass(context, "UInt16", TUUVM_NULL_TUPLE, NULL);
    context->roots.int16Type = tuuvm_context_createIntrinsicClass(context, "Int16", TUUVM_NULL_TUPLE, NULL);

    context->roots.char32Type = tuuvm_context_createIntrinsicClass(context, "Char32", TUUVM_NULL_TUPLE, NULL);
    context->roots.uint32Type = tuuvm_context_createIntrinsicClass(context, "UInt32", TUUVM_NULL_TUPLE, NULL);
    context->roots.int32Type = tuuvm_context_createIntrinsicClass(context, "Int32", TUUVM_NULL_TUPLE, NULL);

    context->roots.uint64Type = tuuvm_context_createIntrinsicClass(context, "UInt64", TUUVM_NULL_TUPLE, NULL);
    context->roots.int64Type = tuuvm_context_createIntrinsicClass(context, "Int64", TUUVM_NULL_TUPLE, NULL);

    context->roots.floatType = tuuvm_context_createIntrinsicClass(context, "Float", TUUVM_NULL_TUPLE, NULL);
    context->roots.doubleType = tuuvm_context_createIntrinsicClass(context, "Double", TUUVM_NULL_TUPLE, NULL);

    context->roots.sourceCodeType = tuuvm_context_createIntrinsicClass(context, "SourceCode", TUUVM_NULL_TUPLE, "text", "name", "language", "lineStartIndexTable", NULL);
    context->roots.sourcePositionType = tuuvm_context_createIntrinsicClass(context, "SourcePosition", TUUVM_NULL_TUPLE,
        "sourceCode",
        "startIndex", "startLine", "startColumn",
        "endIndex", "endLine", "endColumn",
        NULL);
    context->roots.tokenType = tuuvm_context_createIntrinsicClass(context, "Token", TUUVM_NULL_TUPLE,
        "kind", "sourcePosition", "value",
        NULL);

    context->roots.astNodeType = tuuvm_context_createIntrinsicClass(context, "ASTNode", TUUVM_NULL_TUPLE, "sourcePosition", "analyzedType", NULL);
    context->roots.astArgumentNodeType = tuuvm_context_createIntrinsicClass(context, "ASTArgumentNodeType", context->roots.astNodeType, "isForAll", "name", "type", NULL);
    context->roots.astBinaryExpressionSequenceNodeType = tuuvm_context_createIntrinsicClass(context, "ASTBinaryExpressionSequenceNode", context->roots.astNodeType, "operands", "operations", NULL);
    context->roots.astDoWhileContinueWithNodeType = tuuvm_context_createIntrinsicClass(context, "ASTDoWhileContinueWithNode", context->roots.astNodeType, "bodyExpression", "conditionExpression", "continueExpression", NULL);
    context->roots.astErrorNodeType = tuuvm_context_createIntrinsicClass(context, "ASTErrorNode", context->roots.astNodeType, "errorMessage", NULL);
    context->roots.astFunctionApplicationNodeType = tuuvm_context_createIntrinsicClass(context, "ASTFunctionApplicationNode", context->roots.astNodeType, "functionExpression", "arguments", NULL);
    context->roots.astLambdaNodeType = tuuvm_context_createIntrinsicClass(context, "ASTLambdaNode", context->roots.astNodeType, "flags", "argumentCount", "arguments", "resultType", "body", NULL);
    context->roots.astLexicalBlockNodeType = tuuvm_context_createIntrinsicClass(context, "ASTLexicalBlockNode", context->roots.astNodeType, "body", NULL);
    context->roots.astLiteralNodeType = tuuvm_context_createIntrinsicClass(context, "ASTLiteralNode", context->roots.astNodeType, "value", NULL);
    context->roots.astLocalDefinitionNodeType = tuuvm_context_createIntrinsicClass(context, "ASTLocalDefinitionNode", context->roots.astNodeType, "nameExpression", "valueExpression", NULL);
    context->roots.astIdentifierReferenceNodeType = tuuvm_context_createIntrinsicClass(context, "ASTIdentifierReferenceNode", context->roots.astNodeType, "value", NULL);
    context->roots.astIfNodeType = tuuvm_context_createIntrinsicClass(context, "ASTIfNode", context->roots.astNodeType, "conditionExpression", "trueExpression", "falseExpression", NULL);

    context->roots.astMakeAssociationNodeType = tuuvm_context_createIntrinsicClass(context, "ASTMakeAssociationNode", context->roots.astNodeType, "key", "value",NULL);
    context->roots.astMakeByteArrayNodeType = tuuvm_context_createIntrinsicClass(context, "ASTMakeByteArrayNode", context->roots.astNodeType, "elements", NULL);
    context->roots.astMakeDictionaryNodeType = tuuvm_context_createIntrinsicClass(context, "ASTMakeDictionaryNode", context->roots.astNodeType, "elements", NULL);
    context->roots.astMakeTupleNodeType = tuuvm_context_createIntrinsicClass(context, "ASTMakeTupleNode", context->roots.astNodeType, "elements", NULL);

    context->roots.astMessageSendNodeType = tuuvm_context_createIntrinsicClass(context, "ASTMessageSendNode", context->roots.astNodeType, "receiver", "selector", "arguments", NULL);
    context->roots.astMessageChainNodeType = tuuvm_context_createIntrinsicClass(context, "ASTMessageChainNode", context->roots.astNodeType, "receiver", "messages", NULL);
    context->roots.astMessageChainMessageNodeType = tuuvm_context_createIntrinsicClass(context, "ASTMessageChainMessage", context->roots.astNodeType, "selector", "arguments", NULL);
    context->roots.astSequenceNodeType = tuuvm_context_createIntrinsicClass(context, "ASTSequenceNode", context->roots.astNodeType, "expressions", NULL);
    context->roots.astUnexpandedApplicationNodeType = tuuvm_context_createIntrinsicClass(context, "ASTUnexpandedApplicationNode", context->roots.astNodeType, "functionOrMacroExpression", "arguments", NULL);
    context->roots.astUnexpandedSExpressionNodeType = tuuvm_context_createIntrinsicClass(context, "ASTUnexpandedSExpressionNode", context->roots.astNodeType, "elements", NULL);
    context->roots.astWhileContinueWithNodeType = tuuvm_context_createIntrinsicClass(context, "ASTWhileContinueWithNode", context->roots.astNodeType, "conditionExpression", "bodyExpression", "continueExpression", NULL);

    context->roots.astQuoteNodeType = tuuvm_context_createIntrinsicClass(context, "ASTQuoteNode", context->roots.astNodeType, "node", NULL);
    context->roots.astQuasiQuoteNodeType = tuuvm_context_createIntrinsicClass(context, "ASTQuasiQuoteNode", context->roots.astNodeType, "node", NULL);
    context->roots.astQuasiUnquoteNodeType = tuuvm_context_createIntrinsicClass(context, "ASTQuasiUnquoteNode", context->roots.astNodeType, "expression", NULL);
    context->roots.astSpliceNodeType = tuuvm_context_createIntrinsicClass(context, "ASTSpliceNode", context->roots.astNodeType, "expression", NULL);

    // Fill the immediate type table.
    context->roots.immediateTypeTable[TUUVM_TUPLE_TAG_NIL] = context->roots.nilType;
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
    context->roots.immediateTypeTable[TUUVM_TUPLE_TAG_FLOAT] = context->roots.floatType;
    context->roots.immediateTypeTable[TUUVM_TUPLE_TAG_DOUBLE] = context->roots.doubleType;
    context->roots.immediateTypeTable[TUUVM_TUPLE_TAG_TRIVIAL] = context->roots.nilType;

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
    tuuvm_function_setupPrimitives(context);
    tuuvm_integer_setupPrimitives(context);
    tuuvm_io_setupPrimitives(context);
    tuuvm_primitiveInteger_setupPrimitives(context);
    tuuvm_string_setupPrimitives(context);
    tuuvm_stringBuilder_setupPrimitives(context);
    tuuvm_tuple_setupPrimitives(context);
    
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

tuuvm_heap_t *tuuvm_context_getHeap(tuuvm_context_t *context)
{
    if(!context) return 0;

    return &context->heap;
}

static size_t tuuvm_context_generateIdentityHash(tuuvm_context_t *context)
{
    return context->identityHashSeed = tuuvm_hashMultiply(context->identityHashSeed) + 12345;
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
