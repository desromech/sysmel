#include "sysbvm/pragma.h"
#include "internal/context.h"
#include <stdlib.h>

SYSBVM_API sysbvm_tuple_t sysbvm_pragma_create(sysbvm_context_t *context, sysbvm_tuple_t selector, sysbvm_tuple_t arguments)
{
    sysbvm_pragma_t *result = (sysbvm_pragma_t*)sysbvm_context_allocatePointerTuple(context, context->roots.pragmaType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_pragma_t));
    result->selector = selector;
    result->arguments = arguments;
    return (sysbvm_tuple_t)result;
}
