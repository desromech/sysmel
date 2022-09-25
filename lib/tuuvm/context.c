#include "internal/context.h"
#include <stdlib.h>

TUUVM_API tuuvm_context_t *tuuvm_context_create(void)
{
    tuuvm_context_t *context = (tuuvm_context_t*)calloc(1, sizeof(tuuvm_context_t));
    return context;
}

TUUVM_API void tuuvm_context_destroy(tuuvm_context_t *context)
{
    if(!context) return;

    // Destroy the context heap.
    tuuvm_heap_destroy(&context->heap);
    free(context);
}

tuuvm_heap_t *tuuvm_context_getHeap(tuuvm_context_t *context)
{
    if(!context) return 0;

    return &context->heap;
}

tuuvm_object_tuple_t *tuuvm_context_allocateByteTuple(tuuvm_context_t *context, size_t byteSize)
{
    if(!context) return 0;

    return tuuvm_heap_allocateByteTuple(&context->heap, byteSize);
}

tuuvm_object_tuple_t *tuuvm_context_allocatePointerTuple(tuuvm_context_t *context, size_t slotCount)
{
    if(!context) return 0;

    return tuuvm_heap_allocatePointerTuple(&context->heap, slotCount);
}
