#ifndef TUUVM_INTERNAL_CONTEXT_H
#define TUUVM_INTERNAL_CONTEXT_H

#pragma once

#include "tuuvm/context.h"
#include "heap.h"

typedef struct tuuvm_context_roots_s
{
    tuuvm_tuple_t immediateTypeTable[TUUVM_TUPLE_TAG_COUNT];
    tuuvm_tuple_t immediateTrivialTypeTable[TUUVM_TUPLE_IMMEDIATE_TRIVIAL_COUNT];
    tuuvm_tuple_t internedSymbolSet;

    tuuvm_tuple_t identityEqualsFunction;
    tuuvm_tuple_t identityNotEqualsFunction;
    tuuvm_tuple_t identityHashFunction;
    tuuvm_tuple_t stringEqualsFunction;
    tuuvm_tuple_t stringHashFunction;

    tuuvm_tuple_t equalsSelector;
    tuuvm_tuple_t hashSelector;
    tuuvm_tuple_t asStringSelector;
    tuuvm_tuple_t printStringSelector;

    tuuvm_tuple_t astNodeAnalysisSelector;
    tuuvm_tuple_t astNodeEvaluationSelector;
    tuuvm_tuple_t astNodeAnalysisAndEvaluationSelector;

    tuuvm_tuple_t anyValueType;

    tuuvm_tuple_t arrayType;
    tuuvm_tuple_t arraySliceType;
    tuuvm_tuple_t arrayListType;
    tuuvm_tuple_t booleanType;
    tuuvm_tuple_t byteArrayType;
    tuuvm_tuple_t closureASTFunctionType;
    tuuvm_tuple_t dictionaryType;
    tuuvm_tuple_t environmentType;
    tuuvm_tuple_t falseType;
    tuuvm_tuple_t hashtableEmptyType;
    tuuvm_tuple_t integerType;
    tuuvm_tuple_t positiveIntegerType;
    tuuvm_tuple_t negativeIntegerType;
    tuuvm_tuple_t macroContextType;
    tuuvm_tuple_t primitiveFunctionType;
    tuuvm_tuple_t setType;
    tuuvm_tuple_t stringType;
    tuuvm_tuple_t stringBuilderType;
    tuuvm_tuple_t symbolType;
    tuuvm_tuple_t trueType;
    tuuvm_tuple_t typeType;
    tuuvm_tuple_t nilType;
    tuuvm_tuple_t valueBoxType;
    tuuvm_tuple_t voidType;

    tuuvm_tuple_t char8Type;
    tuuvm_tuple_t uint8Type;
    tuuvm_tuple_t int8Type;

    tuuvm_tuple_t char16Type;
    tuuvm_tuple_t uint16Type;
    tuuvm_tuple_t int16Type;

    tuuvm_tuple_t char32Type;
    tuuvm_tuple_t uint32Type;
    tuuvm_tuple_t int32Type;

    tuuvm_tuple_t uint64Type;
    tuuvm_tuple_t int64Type;

    tuuvm_tuple_t floatType;
    tuuvm_tuple_t doubleType;

    tuuvm_tuple_t sourceCodeType;
    tuuvm_tuple_t sourcePositionType;
    tuuvm_tuple_t tokenType;

    tuuvm_tuple_t astNodeType;
    tuuvm_tuple_t astBinaryExpressionSequenceNodeType;
    tuuvm_tuple_t astDoWhileContinueWithNodeType;
    tuuvm_tuple_t astErrorNodeType;
    tuuvm_tuple_t astFunctionApplicationNodeType;
    tuuvm_tuple_t astLambdaNodeType;
    tuuvm_tuple_t astLexicalBlockNodeType;
    tuuvm_tuple_t astLiteralNodeType;
    tuuvm_tuple_t astLocalDefinitionNodeType;
    tuuvm_tuple_t astIdentifierReferenceNodeType;
    tuuvm_tuple_t astIfNodeType;
    tuuvm_tuple_t astMakeAssociationNodeType;
    tuuvm_tuple_t astMakeByteArrayNodeType;
    tuuvm_tuple_t astMakeDictionaryNodeType;
    tuuvm_tuple_t astMakeTupleNodeType;
    tuuvm_tuple_t astMessageSendNodeType;
    tuuvm_tuple_t astMessageChainNodeType;
    tuuvm_tuple_t astMessageChainMessageNodeType;
    tuuvm_tuple_t astSequenceNodeType;
    tuuvm_tuple_t astUnexpandedApplicationNodeType;
    tuuvm_tuple_t astUnexpandedSExpressionNodeType;
    tuuvm_tuple_t astWhileContinueWithNodeType;

    tuuvm_tuple_t astQuoteNodeType;
    tuuvm_tuple_t astQuasiQuoteNodeType;
    tuuvm_tuple_t astQuasiUnquoteNodeType;
    tuuvm_tuple_t astSpliceNodeType;

    tuuvm_tuple_t intrinsicsBuiltInEnvironment;
    tuuvm_tuple_t intrinsicTypes;
} tuuvm_context_roots_t;

struct tuuvm_context_s
{
    tuuvm_heap_t heap;
    tuuvm_context_roots_t roots;
    size_t identityHashSeed;
};

#endif //TUUVM_INTERNAL_CONTEXT_H
