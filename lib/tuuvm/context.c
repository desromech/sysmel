#include "internal/context.h"
#include "tuuvm/type.h"
#include "tuuvm/string.h"
#include "tuuvm/set.h"
#include "tuuvm/function.h"
#include <stdlib.h>

static void tuuvm_context_createBasicTypes(tuuvm_context_t *context)
{
    // Make a circular base type.
    context->roots.typeType = tuuvm_type_createAnonymous(context);
    tuuvm_tuple_setType((tuuvm_object_tuple_t*)context->roots.typeType, context->roots.typeType);

    // Create the symbol and set type.
    context->roots.symbolType = tuuvm_type_createAnonymous(context);
    context->roots.primitiveFunctionType = tuuvm_type_createAnonymous(context);
    context->roots.setType = tuuvm_type_createAnonymous(context);
    context->roots.internedSymbolSet = tuuvm_set_create(context, tuuvm_function_createPrimitive(context, 0, tuuvm_string_primitive_equals), tuuvm_function_createPrimitive(context, 0, tuuvm_string_primitive_hash));

    // Set the name of the root basic type.
    tuuvm_type_setName(context->roots.typeType, tuuvm_symbol_internWithCString(context, "Type"));
    tuuvm_type_setName(context->roots.primitiveFunctionType, tuuvm_symbol_internWithCString(context, "PrimitiveFunction"));
    tuuvm_type_setName(context->roots.symbolType, tuuvm_symbol_internWithCString(context, "Symbol"));
    tuuvm_type_setName(context->roots.setType, tuuvm_symbol_internWithCString(context, "Set"));

    // Create other root basic types.
    context->roots.arrayType = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "Array"));
    context->roots.arraySliceType = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "ArraySlice"));
    context->roots.arrayListType = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "ArrayList"));
    context->roots.falseType = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "False"));
    context->roots.hashtableEmptyType = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "HashtableEmpty"));
    context->roots.integerType = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "Integer"));
    context->roots.nilType = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "Nil"));
    context->roots.stringType = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "String"));
    context->roots.trueType = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "True"));
    context->roots.voidType = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "Void"));

    context->roots.char8Type = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "Char8"));
    context->roots.uint8Type = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "UInt8"));
    context->roots.int8Type = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "Int8"));

    context->roots.char16Type = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "Char16"));
    context->roots.uint16Type = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "UInt16"));
    context->roots.int16Type = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "Int16"));

    context->roots.char32Type = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "Char32"));
    context->roots.uint32Type = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "UInt32"));
    context->roots.int32Type = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "Int32"));

    context->roots.uint64Type = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "UInt64"));
    context->roots.int64Type = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "Int64"));

    context->roots.floatType = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "Float"));
    context->roots.doubleType = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "Double"));

    context->roots.sourceCodeType = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "SourceCode"));
    context->roots.sourcePositionType = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "SourcePosition"));
    context->roots.tokenType = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "Token"));

    context->roots.astNodeType = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "ASTNode"));
    context->roots.astErrorNodeType = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "ASTErrorNode"));
    context->roots.astFunctionApplicationNodeType = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "ASTFunctionApplicationNode"));
    context->roots.astLiteralNodeType = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "ASTLiteralNode"));
    context->roots.astIdentifierReferenceNodeType = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "ASTIdentifierReferenceNode"));
    context->roots.astSequenceNodeType = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "ASTSequenceNode"));
    context->roots.astUnexpandedApplicationNodeType = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "ASTUnexpandedApplicationNode"));

    context->roots.astQuoteNodeType = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "ASTQuoteNode"));
    context->roots.astQuasiQuoteNodeType = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "ASTQuasiQuoteNode"));
    context->roots.astQuasiUnquoteNodeType = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "ASTQuasiUnquoteNode"));
    context->roots.astSpliceNodeType = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "ASTSpliceNode"));

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
    tuuvm_context_createBasicTypes(context);

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

tuuvm_object_tuple_t *tuuvm_context_allocateByteTuple(tuuvm_context_t *context, tuuvm_tuple_t type, size_t byteSize)
{
    if(!context) return 0;

    tuuvm_object_tuple_t *result = tuuvm_heap_allocateByteTuple(&context->heap, byteSize);
    if(result)
        tuuvm_tuple_setType(result, type);
    return result;
}

tuuvm_object_tuple_t *tuuvm_context_allocatePointerTuple(tuuvm_context_t *context, tuuvm_tuple_t type, size_t slotCount)
{
    if(!context) return 0;

    tuuvm_object_tuple_t *result = tuuvm_heap_allocatePointerTuple(&context->heap, slotCount);
    if(result)
        tuuvm_tuple_setType(result, type);
    return result;
}
