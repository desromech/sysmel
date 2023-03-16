#include "tuuvm/ast.h"
#include "tuuvm/array.h"
#include "tuuvm/string.h"
#include "tuuvm/function.h"
#include "internal/context.h"

TUUVM_API bool tuuvm_astNode_isArgumentNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astArgumentNodeType);
}

TUUVM_API bool tuuvm_astNode_isBinaryExpressionSequenceNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astBinaryExpressionSequenceNodeType);
}

TUUVM_API bool tuuvm_astNode_isBreakNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astBreakNodeType);
}

TUUVM_API bool tuuvm_astNode_isContinueNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astContinueNodeType);
}

TUUVM_API bool tuuvm_astNode_isDoWhileContinueWithNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astDoWhileContinueWithNodeType);
}

TUUVM_API bool tuuvm_astNode_isErrorNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astErrorNodeType);
}

TUUVM_API bool tuuvm_astNode_isFunctionApplicationNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astFunctionApplicationNodeType);
}

TUUVM_API bool tuuvm_astNode_isIdentifierReferenceNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astIdentifierReferenceNodeType);
}

TUUVM_API bool tuuvm_astNode_isIfNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astIfNodeType);
}

TUUVM_API bool tuuvm_astNode_isLambdaNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astLambdaNodeType);
}

TUUVM_API bool tuuvm_astNode_isLexicalBlockNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astLexicalBlockNodeType);
}

TUUVM_API bool tuuvm_astNode_isLiteralNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astLiteralNodeType);
}

TUUVM_API bool tuuvm_astNode_isLocalDefinitionNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astLocalDefinitionNodeType);
}

TUUVM_API bool tuuvm_astNode_isMakeAssociationNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astMakeAssociationNodeType);
}

TUUVM_API bool tuuvm_astNode_isMakeByteArrayNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astMakeByteArrayNodeType);
}

TUUVM_API bool tuuvm_astNode_isMakeDictionaryNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astMakeDictionaryNodeType);
}

TUUVM_API bool tuuvm_astNode_isMakeTupleNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astMakeTupleNodeType);
}

TUUVM_API bool tuuvm_astNode_isMessageSendNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astMessageSendNodeType);
}

TUUVM_API bool tuuvm_astNode_isMessageChainNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astMessageChainNodeType);
}

TUUVM_API bool tuuvm_astNode_isMessageChainMessageNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astMessageChainMessageNodeType);
}

TUUVM_API bool tuuvm_astNode_isObjectWithLookupStartingFromNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astObjectWithLookupStartingFromNodeType);
}

TUUVM_API bool tuuvm_astNode_isPragmaNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astPragmaNodeType);
}

TUUVM_API bool tuuvm_astNode_isReturnNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astReturnNodeType);
}

TUUVM_API bool tuuvm_astNode_isSequenceNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astSequenceNodeType);
}

TUUVM_API bool tuuvm_astNode_isQuoteNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astQuoteNodeType);
}

TUUVM_API bool tuuvm_astNode_isQuasiQuoteNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astQuasiQuoteNodeType);
}

TUUVM_API bool tuuvm_astNode_isQuasiUnquoteNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astQuasiUnquoteNodeType);
}

TUUVM_API bool tuuvm_astNode_isSpliceNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astSpliceNodeType);
}

TUUVM_API bool tuuvm_astNode_isMacroExpression(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    if(tuuvm_astNode_isLiteralNode(context, tuple))
        return tuuvm_function_isMacro(context, tuuvm_astLiteralNode_getValue(tuple));

    return false;
}

TUUVM_API bool tuuvm_astNode_isUnexpandedApplicationNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astUnexpandedApplicationNodeType);
}

TUUVM_API bool tuuvm_astNode_isUnexpandedSExpressionNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astUnexpandedSExpressionNodeType);
}

TUUVM_API bool tuuvm_astNode_isWhileContinueWithNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_isKindOf(context, tuple, context->roots.astWhileContinueWithNodeType);
}

TUUVM_API tuuvm_tuple_t tuuvm_astNode_getSourcePosition(tuuvm_tuple_t node)
{
    if(!tuuvm_tuple_isNonNullPointer(node)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_astNode_t*)node)->sourcePosition;
}

TUUVM_API tuuvm_tuple_t tuuvm_astNode_getAnalyzedType(tuuvm_tuple_t node)
{
    if(!tuuvm_tuple_isNonNullPointer(node)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_astNode_t*)node)->analyzedType;
}

TUUVM_API tuuvm_tuple_t tuuvm_astArgumentNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t isForAll, tuuvm_tuple_t name, tuuvm_tuple_t type)
{
    tuuvm_astArgumentNode_t *result = (tuuvm_astArgumentNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astArgumentNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astArgumentNode_t));
    result->super.sourcePosition = sourcePosition;
    result->isForAll = isForAll;
    result->name = name;
    result->type = type;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astBinaryExpressionSequenceNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t operands, tuuvm_tuple_t operators)
{
    tuuvm_astBinaryExpressionSequenceNode_t *result = (tuuvm_astBinaryExpressionSequenceNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astBinaryExpressionSequenceNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astBinaryExpressionSequenceNode_t));
    result->super.sourcePosition = sourcePosition;
    result->operands = operands;
    result->operators = operators;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astBreakNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition)
{
    tuuvm_astBreakNode_t *result = (tuuvm_astBreakNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astBreakNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astBreakNode_t));
    result->super.sourcePosition = sourcePosition;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astContinueNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition)
{
    tuuvm_astContinueNode_t *result = (tuuvm_astContinueNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astContinueNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astContinueNode_t));
    result->super.sourcePosition = sourcePosition;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astDoWhileContinueWithNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t bodyExpression, tuuvm_tuple_t conditionExpression, tuuvm_tuple_t continueExpression)
{
    tuuvm_astDoWhileContinueWithNode_t *result = (tuuvm_astDoWhileContinueWithNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astDoWhileContinueWithNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astDoWhileContinueWithNode_t));
    result->super.sourcePosition = sourcePosition;
    result->bodyExpression = bodyExpression;
    result->conditionExpression = conditionExpression;
    result->continueExpression = continueExpression;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astDoWhileContinueWithNode_getConditionExpression(tuuvm_tuple_t node)
{
    if(!tuuvm_tuple_isNonNullPointer(node)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_astDoWhileContinueWithNode_t*)node)->conditionExpression;
}

TUUVM_API tuuvm_tuple_t tuuvm_astDoWhileContinueWithNode_getBodyExpression(tuuvm_tuple_t node)
{
    if(!tuuvm_tuple_isNonNullPointer(node)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_astDoWhileContinueWithNode_t*)node)->bodyExpression;
}

TUUVM_API tuuvm_tuple_t tuuvm_astDoWhileContinueWithNode_getContinueExpression(tuuvm_tuple_t node)
{
    if(!tuuvm_tuple_isNonNullPointer(node)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_astDoWhileContinueWithNode_t*)node)->continueExpression;
}

TUUVM_API tuuvm_tuple_t tuuvm_astErrorNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t errorMessage)
{
    tuuvm_astErrorNode_t *result = (tuuvm_astErrorNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astErrorNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astErrorNode_t));
    result->super.sourcePosition = sourcePosition;
    result->errorMessage = errorMessage;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astErrorNode_createWithCString(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, const char *errorMessage)
{
    return tuuvm_astErrorNode_create(context, sourcePosition, tuuvm_string_createWithCString(context, errorMessage));
}

TUUVM_API tuuvm_tuple_t tuuvm_astFunctionApplicationNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t functionExpression, tuuvm_tuple_t arguments)
{
    tuuvm_astFunctionApplicationNode_t *result = (tuuvm_astFunctionApplicationNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astFunctionApplicationNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astFunctionApplicationNode_t));
    result->super.sourcePosition = sourcePosition;
    result->functionExpression = functionExpression;
    result->arguments = arguments;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astIdentifierReferenceNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t value)
{
    tuuvm_astIdentifierReferenceNode_t *result = (tuuvm_astIdentifierReferenceNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astIdentifierReferenceNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astIdentifierReferenceNode_t));
    result->super.sourcePosition = sourcePosition;
    result->value = value;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astIdentifierReferenceNode_getValue(tuuvm_tuple_t node)
{
    if(!tuuvm_tuple_isNonNullPointer(node)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_astIdentifierReferenceNode_t*)node)->value;
}

TUUVM_API bool tuuvm_astIdentifierReferenceNode_isEllipsis(tuuvm_tuple_t node)
{
    if(!tuuvm_tuple_isNonNullPointer(node)) return false;
    return tuuvm_string_equalsCString(((tuuvm_astIdentifierReferenceNode_t*)node)->value, "...");
}

TUUVM_API tuuvm_tuple_t tuuvm_astIfNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t conditionExpression, tuuvm_tuple_t trueExpression, tuuvm_tuple_t falseExpression)
{
    tuuvm_astIfNode_t *result = (tuuvm_astIfNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astIfNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astIfNode_t));
    result->super.sourcePosition = sourcePosition;
    result->conditionExpression = conditionExpression;
    result->trueExpression = trueExpression;
    result->falseExpression = falseExpression;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astIfNode_getConditionExpression(tuuvm_tuple_t node)
{
    if(!tuuvm_tuple_isNonNullPointer(node)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_astIfNode_t*)node)->conditionExpression;
}

TUUVM_API tuuvm_tuple_t tuuvm_astIfNode_getTrueExpression(tuuvm_tuple_t node)
{
    if(!tuuvm_tuple_isNonNullPointer(node)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_astIfNode_t*)node)->trueExpression;
}

TUUVM_API tuuvm_tuple_t tuuvm_astIfNode_getFalseExpression(tuuvm_tuple_t node)
{
    if(!tuuvm_tuple_isNonNullPointer(node)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_astIfNode_t*)node)->falseExpression;
}

TUUVM_API tuuvm_tuple_t tuuvm_astLambdaNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t flags, tuuvm_tuple_t arguments, tuuvm_tuple_t resultType, tuuvm_tuple_t body)
{
    tuuvm_astLambdaNode_t *result = (tuuvm_astLambdaNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astLambdaNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astLambdaNode_t));
    result->super.sourcePosition = sourcePosition;
    result->flags = flags;
    result->arguments = arguments;
    result->resultType = resultType;
    result->body = body;
    result->hasLazyAnalysis = TUUVM_FALSE_TUPLE;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astLambdaNode_getFlags(tuuvm_tuple_t node)
{
    if(!tuuvm_tuple_isNonNullPointer(node)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_astLambdaNode_t*)node)->flags;
}

TUUVM_API tuuvm_tuple_t tuuvm_astLambdaNode_getArguments(tuuvm_tuple_t node)
{
    if(!tuuvm_tuple_isNonNullPointer(node)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_astLambdaNode_t*)node)->arguments;
}

TUUVM_API tuuvm_tuple_t tuuvm_astLambdaNode_getBody(tuuvm_tuple_t node)
{
    if(!tuuvm_tuple_isNonNullPointer(node)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_astLambdaNode_t*)node)->body;
}

TUUVM_API tuuvm_tuple_t tuuvm_astLexicalBlockNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t body)
{
    tuuvm_astLexicalBlockNode_t *result = (tuuvm_astLexicalBlockNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astLexicalBlockNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astLexicalBlockNode_t));
    result->super.sourcePosition = sourcePosition;
    result->body = body;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astLiteralNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t value)
{
    tuuvm_astLiteralNode_t *result = (tuuvm_astLiteralNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astLiteralNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astLiteralNode_t));
    result->super.sourcePosition = sourcePosition;
    result->super.analyzedType = tuuvm_tuple_getType(context, value);
    result->value = value;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astLiteralNode_getValue(tuuvm_tuple_t node)
{
    if(!tuuvm_tuple_isNonNullPointer(node)) return 0;
    return ((tuuvm_astLiteralNode_t*)node)->value;
}

TUUVM_API tuuvm_tuple_t tuuvm_astLocalDefinitionNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t nameExpression, tuuvm_tuple_t typeExpression, tuuvm_tuple_t valueExpression, bool isMutable)
{
    tuuvm_astLocalDefinitionNode_t *result = (tuuvm_astLocalDefinitionNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astLocalDefinitionNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astLocalDefinitionNode_t));
    result->super.sourcePosition = sourcePosition;
    result->nameExpression = nameExpression;
    result->typeExpression = typeExpression;
    result->valueExpression = valueExpression;
    result->isMacroSymbol = TUUVM_FALSE_TUPLE;
    result->isMutable = tuuvm_tuple_boolean_encode(isMutable);
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astLocalDefinitionNode_createMacro(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t nameExpression, tuuvm_tuple_t typeExpression, tuuvm_tuple_t valueExpression)
{
    tuuvm_astLocalDefinitionNode_t *result = (tuuvm_astLocalDefinitionNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astLocalDefinitionNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astLocalDefinitionNode_t));
    result->super.sourcePosition = sourcePosition;
    result->nameExpression = nameExpression;
    result->typeExpression = typeExpression;
    result->valueExpression = valueExpression;
    result->isMacroSymbol = TUUVM_TRUE_TUPLE;
    result->isMutable = TUUVM_FALSE_TUPLE;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astLocalDefinitionNode_getNameExpression(tuuvm_tuple_t node)
{
    if(!tuuvm_tuple_isNonNullPointer(node)) return 0;
    return ((tuuvm_astLocalDefinitionNode_t*)node)->nameExpression;
}

TUUVM_API tuuvm_tuple_t tuuvm_astLocalDefinitionNode_getTypeExpression(tuuvm_tuple_t node)
{
    if(!tuuvm_tuple_isNonNullPointer(node)) return 0;
    return ((tuuvm_astLocalDefinitionNode_t*)node)->typeExpression;
}

TUUVM_API tuuvm_tuple_t tuuvm_astLocalDefinitionNode_getValueExpression(tuuvm_tuple_t node)
{
    if(!tuuvm_tuple_isNonNullPointer(node)) return 0;
    return ((tuuvm_astLocalDefinitionNode_t*)node)->valueExpression;
}

TUUVM_API tuuvm_tuple_t tuuvm_astMakeAssociationNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t key, tuuvm_tuple_t value)
{
    tuuvm_astMakeAssociationNode_t *result = (tuuvm_astMakeAssociationNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astMakeAssociationNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astMakeAssociationNode_t));
    result->super.sourcePosition = sourcePosition;
    result->key = key;
    result->value = value;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astMakeByteArrayNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t elements)
{
    tuuvm_astMakeByteArrayNode_t *result = (tuuvm_astMakeByteArrayNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astMakeByteArrayNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astMakeByteArrayNode_t));
    result->super.sourcePosition = sourcePosition;
    result->elements = elements;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astMakeDictionaryNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t elements)
{
    tuuvm_astMakeDictionaryNode_t *result = (tuuvm_astMakeDictionaryNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astMakeDictionaryNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astMakeDictionaryNode_t));
    result->super.sourcePosition = sourcePosition;
    result->elements = elements;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astMakeTupleNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t elements)
{
    tuuvm_astMakeTupleNode_t *result = (tuuvm_astMakeTupleNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astMakeTupleNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astMakeTupleNode_t));
    result->super.sourcePosition = sourcePosition;
    result->elements = elements;
    return (tuuvm_tuple_t)result;

}

TUUVM_API tuuvm_tuple_t tuuvm_astMessageSendNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t receiver, tuuvm_tuple_t selector, tuuvm_tuple_t arguments)
{
    tuuvm_astMessageSendNode_t *result = (tuuvm_astMessageSendNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astMessageSendNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astMessageSendNode_t));
    result->super.sourcePosition = sourcePosition;
    result->receiver = receiver;
    result->selector = selector;
    result->arguments = arguments;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astMessageChainNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t receiver, tuuvm_tuple_t messages)
{
    tuuvm_astMessageChainNode_t *result = (tuuvm_astMessageChainNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astMessageChainNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astMessageChainNode_t));
    result->super.sourcePosition = sourcePosition;
    result->receiver = receiver;
    result->messages = messages;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astMessageChainMessageNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t selector, tuuvm_tuple_t arguments)
{
    tuuvm_astMessageChainMessageNode_t *result = (tuuvm_astMessageChainMessageNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astMessageChainMessageNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astMessageChainMessageNode_t));
    result->super.sourcePosition = sourcePosition;
    result->selector = selector;
    result->arguments = arguments;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astObjectWithLookupStartingFrom_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t objectExpression, tuuvm_tuple_t typeExpression)
{
    tuuvm_astObjectWithLookupStartingFromNode_t *result = (tuuvm_astObjectWithLookupStartingFromNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astObjectWithLookupStartingFromNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astObjectWithLookupStartingFromNode_t));
    result->super.sourcePosition = sourcePosition;
    result->objectExpression = objectExpression;
    result->typeExpression = typeExpression;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astPragmaNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t selector, tuuvm_tuple_t arguments)
{
    tuuvm_astPragmaNode_t *result = (tuuvm_astPragmaNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astPragmaNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astPragmaNode_t));
    result->super.sourcePosition = sourcePosition;
    result->selector = selector;
    result->arguments = arguments;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astReturnNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t expression)
{
    tuuvm_astReturnNode_t *result = (tuuvm_astReturnNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astReturnNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astReturnNode_t));
    result->super.sourcePosition = sourcePosition;
    result->expression = expression;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astSequenceNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t pragmas, tuuvm_tuple_t expressions)
{
    tuuvm_astSequenceNode_t *result = (tuuvm_astSequenceNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astSequenceNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astSequenceNode_t));
    result->super.sourcePosition = sourcePosition;
    result->pragmas = pragmas;
    result->expressions = expressions;
    return (tuuvm_tuple_t)result;
}

TUUVM_API size_t tuuvm_astSequenceNode_getExpressionCount(tuuvm_tuple_t sequenceNode)
{
    if(!tuuvm_tuple_isNonNullPointer(sequenceNode)) return 0;
    return tuuvm_array_getSize( ((tuuvm_astSequenceNode_t*)sequenceNode)->expressions );
}

TUUVM_API tuuvm_tuple_t tuuvm_astSequenceNode_getExpressionAt(tuuvm_tuple_t sequenceNode, size_t index)
{
    if(!tuuvm_tuple_isNonNullPointer(sequenceNode)) return TUUVM_NULL_TUPLE;
    return tuuvm_array_at( ((tuuvm_astSequenceNode_t*)sequenceNode)->expressions, index);
}

TUUVM_API tuuvm_tuple_t tuuvm_astUnexpandedApplicationNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t functionOrMacroExpression, tuuvm_tuple_t arguments)
{
    tuuvm_astUnexpandedApplicationNode_t *result = (tuuvm_astUnexpandedApplicationNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astUnexpandedApplicationNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astUnexpandedApplicationNode_t));
    result->super.sourcePosition = sourcePosition;
    result->functionOrMacroExpression = functionOrMacroExpression;
    result->arguments = arguments;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astUnexpandedApplicationNode_getFunctionOrMacroExpression(tuuvm_tuple_t unexpandedApplication)
{
    if(!tuuvm_tuple_isNonNullPointer(unexpandedApplication)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_astUnexpandedApplicationNode_t*)unexpandedApplication)->functionOrMacroExpression;
}

TUUVM_API tuuvm_tuple_t tuuvm_astUnexpandedApplicationNode_getArguments(tuuvm_tuple_t unexpandedApplication)
{
    if(!tuuvm_tuple_isNonNullPointer(unexpandedApplication)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_astUnexpandedApplicationNode_t*)unexpandedApplication)->arguments;
}

TUUVM_API tuuvm_tuple_t tuuvm_astUnexpandedSExpressionNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t elements)
{
    tuuvm_astUnexpandedSExpressionNode_t *result = (tuuvm_astUnexpandedSExpressionNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astUnexpandedSExpressionNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astUnexpandedSExpressionNode_t));
    result->super.sourcePosition = sourcePosition;
    result->elements = elements;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astUnexpandedSExpressionNode_getElements(tuuvm_tuple_t unexpandedSExpressionNode)
{
    if(!tuuvm_tuple_isNonNullPointer(unexpandedSExpressionNode)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_astUnexpandedSExpressionNode_t*)unexpandedSExpressionNode)->elements;
}

TUUVM_API tuuvm_tuple_t tuuvm_astQuoteNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t node)
{
    tuuvm_astQuoteNode_t *result = (tuuvm_astQuoteNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astQuoteNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astQuoteNode_t));
    result->super.sourcePosition = sourcePosition;
    result->node = node;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astQuasiQuoteNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t node)
{
    tuuvm_astQuasiQuoteNode_t *result = (tuuvm_astQuasiQuoteNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astQuasiQuoteNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astQuasiQuoteNode_t));
    result->super.sourcePosition = sourcePosition;
    result->node = node;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astQuasiUnquoteNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t expression)
{
    tuuvm_astQuasiUnquoteNode_t *result = (tuuvm_astQuasiUnquoteNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astQuasiUnquoteNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astQuasiUnquoteNode_t));
    result->super.sourcePosition = sourcePosition;
    result->expression = expression;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astSpliceNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t expression)
{
    tuuvm_astSpliceNode_t *result = (tuuvm_astSpliceNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astSpliceNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astSpliceNode_t));
    result->super.sourcePosition = sourcePosition;
    result->expression = expression;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astWhileContinueWithNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t conditionExpression, tuuvm_tuple_t bodyExpression, tuuvm_tuple_t continueExpression)
{
    tuuvm_astWhileContinueWithNode_t *result = (tuuvm_astWhileContinueWithNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astWhileContinueWithNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astWhileContinueWithNode_t));
    result->super.sourcePosition = sourcePosition;
    result->conditionExpression = conditionExpression;
    result->bodyExpression = bodyExpression;
    result->continueExpression = continueExpression;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astWhileContinueWithNode_getConditionExpression(tuuvm_tuple_t node)
{
    if(!tuuvm_tuple_isNonNullPointer(node)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_astWhileContinueWithNode_t*)node)->conditionExpression;
}

TUUVM_API tuuvm_tuple_t tuuvm_astWhileContinueWithNode_getBodyExpression(tuuvm_tuple_t node)
{
    if(!tuuvm_tuple_isNonNullPointer(node)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_astWhileContinueWithNode_t*)node)->bodyExpression;
}

TUUVM_API tuuvm_tuple_t tuuvm_astWhileContinueWithNode_getContinueExpression(tuuvm_tuple_t node)
{
    if(!tuuvm_tuple_isNonNullPointer(node)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_astWhileContinueWithNode_t*)node)->continueExpression;
}
