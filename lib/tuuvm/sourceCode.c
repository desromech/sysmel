#include "tuuvm/sourceCode.h"
#include "tuuvm/string.h"
#include "internal/context.h"

TUUVM_API tuuvm_tuple_t tuuvm_sourceCode_create(tuuvm_context_t *context, tuuvm_tuple_t text, tuuvm_tuple_t name)
{
    tuuvm_sourceCode_t *result = (tuuvm_sourceCode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.sourceCodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_sourceCode_t));
    result->text = text;
    result->name = name;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_sourceCode_createWithCStrings(tuuvm_context_t *context, const char *text, const char *name)
{
    return tuuvm_sourceCode_create(context, tuuvm_string_createWithCString(context, text), tuuvm_string_createWithCString(context, name));
}

TUUVM_API void tuuvm_sourceCode_computeLineAndColumnForIndex(tuuvm_context_t *context, tuuvm_tuple_t sourceCode, tuuvm_tuple_t index, tuuvm_tuple_t *outLine, tuuvm_tuple_t *outColumn)
{
    *outLine = tuuvm_tuple_size_encode(context, 0);
    *outColumn = tuuvm_tuple_size_encode(context, 0);

    // TODO: Implement this properly.
}