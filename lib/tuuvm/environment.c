#include "tuuvm/environment.h"
#include "tuuvm/dictionary.h"
#include "internal/context.h"

TUUVM_API tuuvm_tuple_t tuuvm_environment_create(tuuvm_context_t *context, tuuvm_tuple_t parent)
{
    tuuvm_environment_t *result = (tuuvm_environment_t*)tuuvm_context_allocatePointerTuple(context, context->roots.environmentType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_environment_t));
    result->parent = parent;
    result->symbolTable = tuuvm_identityDictionary_create(context);
    return (tuuvm_tuple_t)result;
}
