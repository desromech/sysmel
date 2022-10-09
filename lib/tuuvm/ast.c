#include "tuuvm/ast.h"
#include "tuuvm/arraySlice.h"
#include "internal/context.h"

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

TUUVM_API bool tuuvm_astNode_isUnexpandedApplicationNode(tuuvm_context_t *context, tuuvm_tuple_t tuple)
{
    return tuuvm_tuple_getType(context, tuple) == context->roots.astUnexpandedApplicationNodeType;
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
