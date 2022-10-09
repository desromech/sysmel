#include "tuuvm/ast.h"
#include "tuuvm/arraySlice.h"
#include "tuuvm/string.h"
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

TUUVM_API bool tuuvm_astNode_isLiteralNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_getType(context, tuple) == context->roots.astLiteralNodeType;
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

TUUVM_API bool tuuvm_astNode_isUnexpandedApplicationNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_getType(context, tuple) == context->roots.astUnexpandedApplicationNodeType;
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
