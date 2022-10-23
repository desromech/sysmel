#include "tuuvm/ast.h"
#include "tuuvm/arraySlice.h"
#include "tuuvm/string.h"
#include "tuuvm/function.h"
#include "internal/context.h"

TUUVM_API bool tuuvm_astNode_isErrorNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_getType(context, tuple) == context->roots.astErrorNodeType;
}

TUUVM_API bool tuuvm_astNode_isFunctionApplicationNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_getType(context, tuple) == context->roots.astFunctionApplicationNodeType;
}

TUUVM_API bool tuuvm_astNode_isIdentifierReferenceNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_getType(context, tuple) == context->roots.astIdentifierReferenceNodeType;
}

TUUVM_API bool tuuvm_astNode_isLambdaNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_getType(context, tuple) == context->roots.astLambdaNodeType;
}

TUUVM_API bool tuuvm_astNode_isLiteralNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_getType(context, tuple) == context->roots.astLiteralNodeType;
}

TUUVM_API bool tuuvm_astNode_isLocalDefinitionNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_getType(context, tuple) == context->roots.astLocalDefinitionNodeType;
}

TUUVM_API bool tuuvm_astNode_isSequenceNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_getType(context, tuple) == context->roots.astSequenceNodeType;
}

TUUVM_API bool tuuvm_astNode_isQuoteNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_getType(context, tuple) == context->roots.astQuoteNodeType;
}

TUUVM_API bool tuuvm_astNode_isQuasiQuoteNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_getType(context, tuple) == context->roots.astQuasiQuoteNodeType;
}

TUUVM_API bool tuuvm_astNode_isQuasiUnquoteNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_getType(context, tuple) == context->roots.astQuasiUnquoteNodeType;
}

TUUVM_API bool tuuvm_astNode_isSpliceNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_getType(context, tuple) == context->roots.astSpliceNodeType;
}

TUUVM_API bool tuuvm_astNode_isMacroExpression(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    if(tuuvm_astNode_isLiteralNode(context, tuple))
        return tuuvm_function_isMacro(context, tuuvm_astLiteralNode_getValue(tuple));

    return false;
}

TUUVM_API bool tuuvm_astNode_isUnexpandedApplicationNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_getType(context, tuple) == context->roots.astUnexpandedApplicationNodeType;
}

TUUVM_API bool tuuvm_astNode_isUnexpandedSExpressionNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_getType(context, tuple) == context->roots.astUnexpandedSExpressionNodeType;
}

TUUVM_API tuuvm_tuple_t tuuvm_astNode_getSourcePosition(tuuvm_tuple_t node)
{
    if(!tuuvm_tuple_isNonNullPointer(node)) return TUUVM_NULL_TUPLE;
    return ((tuuvm_astNode_t*)node)->sourcePosition;
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
    if(!tuuvm_tuple_isNonNullPointer(node)) return 0;
    return ((tuuvm_astIdentifierReferenceNode_t*)node)->value;
}

TUUVM_API tuuvm_tuple_t tuuvm_astLambdaNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t flags, tuuvm_tuple_t arguments, tuuvm_tuple_t body)
{
    tuuvm_astLambdaNode_t *result = (tuuvm_astLambdaNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astLambdaNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astLambdaNode_t));
    result->super.sourcePosition = sourcePosition;
    result->flags = flags;
    result->arguments = arguments;
    result->body = body;
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

TUUVM_API tuuvm_tuple_t tuuvm_astLiteralNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t value)
{
    tuuvm_astLiteralNode_t *result = (tuuvm_astLiteralNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astLiteralNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astLiteralNode_t));
    result->super.sourcePosition = sourcePosition;
    result->value = value;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astLiteralNode_getValue(tuuvm_tuple_t node)
{
    if(!tuuvm_tuple_isNonNullPointer(node)) return 0;
    return ((tuuvm_astLiteralNode_t*)node)->value;
}

TUUVM_API tuuvm_tuple_t tuuvm_astLocalDefinitionNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t nameExpression, tuuvm_tuple_t valueExpression)
{
    tuuvm_astLocalDefinitionNode_t *result = (tuuvm_astLocalDefinitionNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astLocalDefinitionNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astLocalDefinitionNode_t));
    result->super.sourcePosition = sourcePosition;
    result->nameExpression = nameExpression;
    result->valueExpression = valueExpression;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astLocalDefinitionNode_getNameExpression(tuuvm_tuple_t node)
{
    if(!tuuvm_tuple_isNonNullPointer(node)) return 0;
    return ((tuuvm_astLocalDefinitionNode_t*)node)->nameExpression;
}

TUUVM_API tuuvm_tuple_t tuuvm_astLocalDefinitionNode_getValueExpression(tuuvm_tuple_t node)
{
    if(!tuuvm_tuple_isNonNullPointer(node)) return 0;
    return ((tuuvm_astLocalDefinitionNode_t*)node)->valueExpression;
}

TUUVM_API tuuvm_tuple_t tuuvm_astSequenceNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t expressions)
{
    tuuvm_astSequenceNode_t *result = (tuuvm_astSequenceNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astSequenceNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astSequenceNode_t));
    result->super.sourcePosition = sourcePosition;
    result->expressions = expressions;
    return (tuuvm_tuple_t)result;
}

TUUVM_API size_t tuuvm_astSequenceNode_getExpressionCount(tuuvm_tuple_t sequenceNode)
{
    if(!tuuvm_tuple_isNonNullPointer(sequenceNode)) return 0;
    return tuuvm_arraySlice_getSize( ((tuuvm_astSequenceNode_t*)sequenceNode)->expressions );
}

TUUVM_API tuuvm_tuple_t tuuvm_astSequenceNode_getExpressionAt(tuuvm_tuple_t sequenceNode, size_t index)
{
    if(!tuuvm_tuple_isNonNullPointer(sequenceNode)) return TUUVM_NULL_TUPLE;
    return tuuvm_arraySlice_at( ((tuuvm_astSequenceNode_t*)sequenceNode)->expressions, index);
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
    tuuvm_astQuasiQuoteNode_t *result = (tuuvm_astQuasiQuoteNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astQuoteNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astQuasiQuoteNode_t));
    result->super.sourcePosition = sourcePosition;
    result->node = node;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astQuasiUnquoteNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t expression)
{
    tuuvm_astQuasiUnquoteNode_t *result = (tuuvm_astQuasiUnquoteNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astQuoteNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astQuasiUnquoteNode_t));
    result->super.sourcePosition = sourcePosition;
    result->expression = expression;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_astSpliceNode_create(tuuvm_context_t *context, tuuvm_tuple_t sourcePosition, tuuvm_tuple_t expression)
{
    tuuvm_astSpliceNode_t *result = (tuuvm_astSpliceNode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.astQuoteNodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_astSpliceNode_t));
    result->super.sourcePosition = sourcePosition;
    result->expression = expression;
    return (tuuvm_tuple_t)result;
}
