#include "tuuvm/pragma.h"
#include "internal/context.h"
#include <stdlib.h>

TUUVM_API tuuvm_tuple_t tuuvm_pragma_create(tuuvm_context_t *context, tuuvm_tuple_t selector, tuuvm_tuple_t arguments)
{
    tuuvm_pragma_t *result = (tuuvm_pragma_t*)tuuvm_context_allocatePointerTuple(context, context->roots.pragmaType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_pragma_t));
    result->selector = selector;
    result->arguments = arguments;
    return (tuuvm_tuple_t)result;
}
