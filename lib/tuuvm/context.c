#include "internal/context.h"
#include "tuuvm/type.h"
#include "tuuvm/array.h"
#include "tuuvm/arrayList.h"
#include "tuuvm/environment.h"
#include "tuuvm/string.h"
#include "tuuvm/set.h"
#include "tuuvm/function.h"
#include <stdlib.h>
#include <stdarg.h>

extern void tuuvm_astInterpreter_setupASTInterpreter(tuuvm_context_t *context);
extern void tuuvm_boolean_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_dictionary_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_errors_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_environment_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_integer_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_io_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_primitiveInteger_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_string_setupPrimitives(tuuvm_context_t *context);
extern void tuuvm_tuple_setupPrimitives(tuuvm_context_t *context);

TUUVM_API tuuvm_tuple_t tuuvm_context_createIntrinsicType(tuuvm_context_t *context, const char *name, tuuvm_tuple_t supertype, ...)
{
    tuuvm_tuple_t nameSymbol = tuuvm_symbol_internWithCString(context, name);
    tuuvm_tuple_t type = tuuvm_type_createWithName(context, nameSymbol);
    tuuvm_type_setSupertype(type, supertype);
    tuuvm_environment_setSymbolBinding(context, context->roots.intrinsicsBuiltInEnvironment, nameSymbol, type);
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
    tuuvm_type_setSupertype(type, supertype);
    tuuvm_environment_setSymbolBinding(context, context->roots.intrinsicsBuiltInEnvironment, nameSymbol, type);
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
}

TUUVM_API void tuuvm_context_setIntrinsicSymbolBinding(tuuvm_context_t *context, tuuvm_tuple_t symbol, tuuvm_tuple_t binding)
{
    tuuvm_environment_setSymbolBinding(context, context->roots.intrinsicsBuiltInEnvironment, symbol, binding);
}

static void tuuvm_context_createBasicTypes(tuuvm_context_t *context)
{
    // Make a circular base type.
    context->roots.typeType = tuuvm_type_createAnonymous(context);
    tuuvm_tuple_setType((tuuvm_object_tuple_t*)context->roots.typeType, context->roots.typeType);

    // Create the symbol and set type.
    context->roots.primitiveFunctionType = tuuvm_type_createAnonymous(context);

    // Create the basic hash functions.
    context->roots.identityEqualsFunction = tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_tuple_primitive_identityEquals);
    context->roots.identityNotEqualsFunction = tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_tuple_primitive_identityNotEquals);
    context->roots.identityHashFunction = tuuvm_function_createPrimitive(context, 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_tuple_primitive_identityHash);
    context->roots.stringEqualsFunction = tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_string_primitive_equals);
    context->roots.stringHashFunction = tuuvm_function_createPrimitive(context, 1, TUUVM_FUNCTION_FLAGS_NONE, NULL, tuuvm_string_primitive_hash);

    context->roots.symbolType = tuuvm_type_createAnonymous(context);
    context->roots.setType = tuuvm_type_createAnonymous(context);
    context->roots.internedSymbolSet = tuuvm_set_create(context, context->roots.stringEqualsFunction, context->roots.stringHashFunction);

    // Create the intrinsic built-in environment
    context->roots.environmentType = tuuvm_type_createAnonymous(context);
    context->roots.arrayType = tuuvm_type_createAnonymous(context);
    context->roots.arrayListType = tuuvm_type_createAnonymous(context);

    context->roots.intrinsicsBuiltInEnvironment = tuuvm_environment_create(context, TUUVM_NULL_TUPLE);
    context->roots.intrinsicTypes = tuuvm_arrayList_create(context);

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

    // Set the name of the root basic type.
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.typeType, "Type", TUUVM_NULL_TUPLE,
        "name", "supertype", "slotNames", "sumTypeAlternatives", "totalSlotCount", "flags",
        "macroMethodDictionary", "methodDictionary", "fallbackMethodDictionary",
        "equalsFunction", "hashFunction", "toStringFunction", "printStringFunction",
        "astNodeAnalysisFunction", "astNodeEvaluationFunction", "astNodeAnalysisAndEvaluationFunction",
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.environmentType, "Environment", TUUVM_NULL_TUPLE, "parent", "symbolTable", NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.primitiveFunctionType, "PrimitiveFunction", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolType, "Symbol", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.setType, "Set", TUUVM_NULL_TUPLE,
        "size", "storage", "equalsFunction", "hashFunction",
        NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.arrayType, "Array", TUUVM_NULL_TUPLE, NULL);
    tuuvm_context_setIntrinsicTypeMetadata(context, context->roots.arrayListType, "ArrayList", TUUVM_NULL_TUPLE, "size", "storage", NULL);

    // Create other root basic types.
    context->roots.arraySliceType = tuuvm_context_createIntrinsicType(context, "ArraySlice", TUUVM_NULL_TUPLE, "elements", "offset", "size", NULL);
    context->roots.closureASTFunctionType = tuuvm_context_createIntrinsicType(context, "ClosureASTFunction", TUUVM_NULL_TUPLE, "sourcePosition", "flags", "closureEnvironment", "argumentSymbols", "body", NULL);
    context->roots.dictionaryType = tuuvm_context_createIntrinsicType(context, "Dictionary", TUUVM_NULL_TUPLE,
        "size", "storage", "equalsFunction", "hashFunction",
        NULL);
    context->roots.falseType = tuuvm_context_createIntrinsicType(context, "False", TUUVM_NULL_TUPLE, NULL);
    context->roots.hashtableEmptyType = tuuvm_context_createIntrinsicType(context, "HashtableEmpty", TUUVM_NULL_TUPLE, NULL);
    context->roots.macroContextType = tuuvm_context_createIntrinsicType(context, "MacroContext", TUUVM_NULL_TUPLE, "sourceNode", "sourcePosition", NULL);
    context->roots.integerType = tuuvm_context_createIntrinsicType(context, "Integer", TUUVM_NULL_TUPLE, NULL);
    context->roots.nilType = tuuvm_context_createIntrinsicType(context, "Nil", TUUVM_NULL_TUPLE, NULL);
    context->roots.stringType = tuuvm_context_createIntrinsicType(context, "String", TUUVM_NULL_TUPLE, NULL);
    context->roots.trueType = tuuvm_context_createIntrinsicType(context, "True", TUUVM_NULL_TUPLE, NULL);
    context->roots.voidType = tuuvm_context_createIntrinsicType(context, "Void", TUUVM_NULL_TUPLE, NULL);

    context->roots.char8Type = tuuvm_context_createIntrinsicType(context, "Char8", TUUVM_NULL_TUPLE, NULL);
    context->roots.uint8Type = tuuvm_context_createIntrinsicType(context, "UInt8", TUUVM_NULL_TUPLE, NULL);
    context->roots.int8Type = tuuvm_context_createIntrinsicType(context, "Int8", TUUVM_NULL_TUPLE, NULL);

    context->roots.char16Type = tuuvm_context_createIntrinsicType(context, "Char16", TUUVM_NULL_TUPLE, NULL);
    context->roots.uint16Type = tuuvm_context_createIntrinsicType(context, "UInt16", TUUVM_NULL_TUPLE, NULL);
    context->roots.int16Type = tuuvm_context_createIntrinsicType(context, "Int16", TUUVM_NULL_TUPLE, NULL);

    context->roots.char32Type = tuuvm_context_createIntrinsicType(context, "Char32", TUUVM_NULL_TUPLE, NULL);
    context->roots.uint32Type = tuuvm_context_createIntrinsicType(context, "UInt32", TUUVM_NULL_TUPLE, NULL);
    context->roots.int32Type = tuuvm_context_createIntrinsicType(context, "Int32", TUUVM_NULL_TUPLE, NULL);

    context->roots.uint64Type = tuuvm_context_createIntrinsicType(context, "UInt64", TUUVM_NULL_TUPLE, NULL);
    context->roots.int64Type = tuuvm_context_createIntrinsicType(context, "Int64", TUUVM_NULL_TUPLE, NULL);

    context->roots.floatType = tuuvm_context_createIntrinsicType(context, "Float", TUUVM_NULL_TUPLE, NULL);
    context->roots.doubleType = tuuvm_context_createIntrinsicType(context, "Double", TUUVM_NULL_TUPLE, NULL);

    context->roots.sourceCodeType = tuuvm_context_createIntrinsicType(context, "SourceCode", TUUVM_NULL_TUPLE, "text", "name", "lineStartIndexTable", NULL);
    context->roots.sourcePositionType = tuuvm_context_createIntrinsicType(context, "SourcePosition", TUUVM_NULL_TUPLE,
        "sourceCode",
        "startIndex", "startLine", "startColumn",
        "endIndex", "endLine", "endColumn",
        NULL);
    context->roots.tokenType = tuuvm_context_createIntrinsicType(context, "Token", TUUVM_NULL_TUPLE,
        "kind", "sourcePosition", "value",
        NULL);

    context->roots.astNodeType = tuuvm_context_createIntrinsicType(context, "ASTNode", TUUVM_NULL_TUPLE, "sourcePosition", "analyzedType", NULL);
    context->roots.astDoWhileContinueWithNodeType = tuuvm_context_createIntrinsicType(context, "ASTDoWhileContinueWithNode", context->roots.astNodeType, "bodyExpression", "conditionExpression", "continueExpression", NULL);
    context->roots.astErrorNodeType = tuuvm_context_createIntrinsicType(context, "ASTErrorNode", context->roots.astNodeType, "errorMessage", NULL);
    context->roots.astFunctionApplicationNodeType = tuuvm_context_createIntrinsicType(context, "ASTFunctionApplicationNode", context->roots.astNodeType, "functionExpression", "arguments", NULL);
    context->roots.astLambdaNodeType = tuuvm_context_createIntrinsicType(context, "ASTLambdaNode", context->roots.astNodeType, "flags", "arguments", "body", NULL);
    context->roots.astLiteralNodeType = tuuvm_context_createIntrinsicType(context, "ASTLiteralNode", context->roots.astNodeType, "value", NULL);
    context->roots.astLocalDefinitionNodeType = tuuvm_context_createIntrinsicType(context, "ASTLocalDefinitionNode", context->roots.astNodeType, "nameExpression", "valueExpression", NULL);
    context->roots.astIdentifierReferenceNodeType = tuuvm_context_createIntrinsicType(context, "ASTIdentifierReferenceNode", context->roots.astNodeType, "value", NULL);
    context->roots.astIfNodeType = tuuvm_context_createIntrinsicType(context, "ASTIfNode", context->roots.astNodeType, "conditionExpression", "trueExpression", "falseExpression", NULL);
    context->roots.astSequenceNodeType = tuuvm_context_createIntrinsicType(context, "ASTSequenceNode", context->roots.astNodeType, "expressions", NULL);
    context->roots.astUnexpandedApplicationNodeType = tuuvm_context_createIntrinsicType(context, "ASTUnexpandedApplicationNode", context->roots.astNodeType, "functionOrMacroExpression", "arguments", NULL);
    context->roots.astUnexpandedSExpressionNodeType = tuuvm_context_createIntrinsicType(context, "ASTUnexpandedSExpressionNode", context->roots.astNodeType, "elements", NULL);
    context->roots.astWhileContinueWithNodeType = tuuvm_context_createIntrinsicType(context, "ASTWhileContinueWithNode", context->roots.astNodeType, "conditionExpression", "bodyExpression", "continueExpression", NULL);

    context->roots.astQuoteNodeType = tuuvm_context_createIntrinsicType(context, "ASTQuoteNode", context->roots.astNodeType, "node", NULL);
    context->roots.astQuasiQuoteNodeType = tuuvm_context_createIntrinsicType(context, "ASTQuasiQuoteNode", context->roots.astNodeType, "node", NULL);
    context->roots.astQuasiUnquoteNodeType = tuuvm_context_createIntrinsicType(context, "ASTQuasiUnquoteNode", context->roots.astNodeType, "expression", NULL);
    context->roots.astSpliceNodeType = tuuvm_context_createIntrinsicType(context, "ASTSpliceNode", context->roots.astNodeType, "expression", NULL);

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
    tuuvm_context_createBasicTypes(context);
    tuuvm_astInterpreter_setupASTInterpreter(context);
    tuuvm_boolean_setupPrimitives(context);
    tuuvm_dictionary_setupPrimitives(context);
    tuuvm_errors_setupPrimitives(context);
    tuuvm_environment_setupPrimitives(context);
    tuuvm_integer_setupPrimitives(context);
    tuuvm_io_setupPrimitives(context);
    tuuvm_primitiveInteger_setupPrimitives(context);
    tuuvm_string_setupPrimitives(context);
    tuuvm_tuple_setupPrimitives(context);

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
