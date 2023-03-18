#ifndef TUUVM_INTERNAL_CONTEXT_H
#define TUUVM_INTERNAL_CONTEXT_H

#pragma once

#include "tuuvm/context.h"
#include "heap.h"

#define GLOBAL_LOOKUP_CACHE_ENTRY_COUNT 256

typedef struct tuuvm_globalLookupCacheEntry_s
{
    tuuvm_tuple_t type;
    tuuvm_tuple_t selector;
    tuuvm_tuple_t method;
}tuuvm_globalLookupCacheEntry_t;

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
    tuuvm_tuple_t doesNotUnderstandSelector;

    tuuvm_tuple_t loadAtOffsetWithTypeSelector;
    tuuvm_tuple_t storeAtOffsetWithTypeSelector;

    tuuvm_tuple_t assignmentSelector;
    tuuvm_tuple_t underscoreSelector;

    tuuvm_tuple_t pointerLikeLoadPrimitive;
    tuuvm_tuple_t pointerLikeStorePrimitive;
    tuuvm_tuple_t pointerTypeTemplate;
    tuuvm_tuple_t referenceTypeTemplate;
    tuuvm_tuple_t simpleFunctionTypeTemplate;
    tuuvm_tuple_t anyValueToVoidPrimitive;

    tuuvm_tuple_t applyWithoutArgumentsSelector;
    tuuvm_tuple_t applyWithArgumentsSelector;

    tuuvm_tuple_t primitiveNamedSelector;

    tuuvm_tuple_t astNodeAnalysisSelector;
    tuuvm_tuple_t astNodeEvaluationSelector;
    tuuvm_tuple_t astNodeAnalysisAndEvaluationSelector;

    tuuvm_tuple_t analyzeAndEvaluateMessageSendNodeForReceiverWithEnvironmentSelector;
    tuuvm_tuple_t analyzeMessageSendNodeWithEnvironmentSelector;
    tuuvm_tuple_t analyzeAndEvaluateMessageChainNodeForReceiverWithEnvironmentSelector;
    tuuvm_tuple_t analyzeMessageChainNodeWithEnvironmentSelector;
    tuuvm_tuple_t analyzeAndEvaluateConcreteMetaValueWithEnvironmentSelector;
    tuuvm_tuple_t analyzeConcreteMetaValueWithEnvironmentSelector;

    tuuvm_tuple_t genericAddressSpaceName;
    tuuvm_tuple_t functionLocalAddressSpaceName;

    tuuvm_tuple_t emptyArrayConstant;
    tuuvm_tuple_t emptyByteArrayConstant;

    tuuvm_tuple_t coerceValueSelector;
    tuuvm_tuple_t coerceASTNodeWithEnvironmentSelector;
    tuuvm_tuple_t typeCheckFunctionApplicationWithEnvironmentSelector;

    tuuvm_tuple_t anyValueType;
    tuuvm_tuple_t anyReferenceType;
    tuuvm_tuple_t anyPointerType;

    tuuvm_tuple_t arrayType;
    tuuvm_tuple_t arraySliceType;
    tuuvm_tuple_t arrayListType;
    tuuvm_tuple_t arrayedCollectionType;
    tuuvm_tuple_t associationType;
    tuuvm_tuple_t booleanType;
    tuuvm_tuple_t byteArrayType;
    tuuvm_tuple_t classType;
    tuuvm_tuple_t collectionType;
    tuuvm_tuple_t dictionaryType;
    tuuvm_tuple_t analysisAndEvaluationEnvironmentType;
    tuuvm_tuple_t analysisEnvironmentType;
    tuuvm_tuple_t environmentType;
    tuuvm_tuple_t falseType;
    tuuvm_tuple_t functionActivationEnvironmentType;
    tuuvm_tuple_t functionAnalysisEnvironmentType;
    tuuvm_tuple_t functionType;
    tuuvm_tuple_t functionDefinitionType;
    tuuvm_tuple_t dependentFunctionTypeType;
    tuuvm_tuple_t simpleFunctionTypeType;
    tuuvm_tuple_t functionTypeType;
    tuuvm_tuple_t generatedSymbolType;
    tuuvm_tuple_t hashedCollectionType;
    tuuvm_tuple_t hashtableEmptyType;
    tuuvm_tuple_t identityDictionaryType;
    tuuvm_tuple_t identitySetType;
    tuuvm_tuple_t integerType;
    tuuvm_tuple_t localAnalysisEnvironmentType;
    tuuvm_tuple_t macroContextType;
    tuuvm_tuple_t messageType;
    tuuvm_tuple_t metaclassType;
    tuuvm_tuple_t methodDictionaryType;
    tuuvm_tuple_t namespaceType;
    tuuvm_tuple_t negativeIntegerType;
    tuuvm_tuple_t objectType;
    tuuvm_tuple_t pendingMemoizationValueType;
    tuuvm_tuple_t pointerLikeType;
    tuuvm_tuple_t pointerType;
    tuuvm_tuple_t positiveIntegerType;
    tuuvm_tuple_t pragmaType;
    tuuvm_tuple_t primitiveValueType;
    tuuvm_tuple_t programEntityType;
    tuuvm_tuple_t referenceType;
    tuuvm_tuple_t sequenceableCollectionType;
    tuuvm_tuple_t setType;
    tuuvm_tuple_t streamType;
    tuuvm_tuple_t stringType;
    tuuvm_tuple_t stringStreamType;
    tuuvm_tuple_t stringSymbolType;
    tuuvm_tuple_t structureType;
    tuuvm_tuple_t symbolType;
    tuuvm_tuple_t symbolBindingType;
    tuuvm_tuple_t symbolAnalysisBindingType;
    tuuvm_tuple_t symbolArgumentBindingType;
    tuuvm_tuple_t symbolCaptureBindingType;
    tuuvm_tuple_t symbolLocalBindingType;
    tuuvm_tuple_t symbolMacroValueBindingType;
    tuuvm_tuple_t symbolValueBindingType;
    tuuvm_tuple_t tombstoneType;
    tuuvm_tuple_t trueType;
    tuuvm_tuple_t typeType;
    tuuvm_tuple_t metatypeType;
    tuuvm_tuple_t typeSlotType;
    tuuvm_tuple_t undefinedObjectType;
    tuuvm_tuple_t valueBoxType;
    tuuvm_tuple_t valueType;
    tuuvm_tuple_t valueMetatypeType;
    tuuvm_tuple_t voidType;
    tuuvm_tuple_t weakSetType;
    tuuvm_tuple_t weakIdentitySetType;
    tuuvm_tuple_t weakKeyDictionaryType;
    tuuvm_tuple_t weakValueAssociationType;
    tuuvm_tuple_t weakValueDictionaryType;

    tuuvm_tuple_t controlFlowEscapeType;
    tuuvm_tuple_t controlFlowBreakType;
    tuuvm_tuple_t controlFlowContinueType;
    tuuvm_tuple_t controlFlowReturnType;
    tuuvm_tuple_t controlFlowNoReturnType;
    tuuvm_tuple_t decayedTypeInferenceType;
    tuuvm_tuple_t directTypeInferenceType;

    tuuvm_tuple_t primitiveNumberType;
    tuuvm_tuple_t primitiveIntegerType;
    tuuvm_tuple_t primitiveCharacterType;
    tuuvm_tuple_t primitiveUnsignedIntegerType;
    tuuvm_tuple_t primitiveSignedIntegerType;
    tuuvm_tuple_t primitiveFloatType;

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

    tuuvm_tuple_t sizeType;
    tuuvm_tuple_t uintptrType;
    tuuvm_tuple_t intptrType;

    tuuvm_tuple_t float32Type;
    tuuvm_tuple_t float64Type;

    tuuvm_tuple_t sourceCodeType;
    tuuvm_tuple_t sourcePositionType;
    tuuvm_tuple_t tokenType;

    tuuvm_tuple_t astNodeType;
    tuuvm_tuple_t astArgumentNodeType;
    tuuvm_tuple_t astBinaryExpressionSequenceNodeType;
    tuuvm_tuple_t astBreakNodeType;
    tuuvm_tuple_t astCoerceValueNodeType;
    tuuvm_tuple_t astContinueNodeType;
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
    tuuvm_tuple_t astObjectWithLookupStartingFromNodeType;
    tuuvm_tuple_t astPragmaNodeType;
    tuuvm_tuple_t astReturnNodeType;
    tuuvm_tuple_t astSequenceNodeType;
    tuuvm_tuple_t astUnexpandedApplicationNodeType;
    tuuvm_tuple_t astUnexpandedSExpressionNodeType;
    tuuvm_tuple_t astWhileContinueWithNodeType;

    tuuvm_tuple_t astQuoteNodeType;
    tuuvm_tuple_t astQuasiQuoteNodeType;
    tuuvm_tuple_t astQuasiUnquoteNodeType;
    tuuvm_tuple_t astSpliceNodeType;

    tuuvm_tuple_t globalNamespace;
    tuuvm_tuple_t intrinsicTypes;

    tuuvm_globalLookupCacheEntry_t globalMethodLookupCache[GLOBAL_LOOKUP_CACHE_ENTRY_COUNT];
} tuuvm_context_roots_t;

struct tuuvm_context_s
{
    tuuvm_heap_t heap;
    tuuvm_context_roots_t roots;
    size_t identityHashSeed;
};

#endif //TUUVM_INTERNAL_CONTEXT_H
