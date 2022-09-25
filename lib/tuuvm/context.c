#include "internal/context.h"
#include "tuuvm/type.h"
#include "tuuvm/string.h"
#include <stdlib.h>

static void tuuvm_context_createBasicTypes(tuuvm_context_t *context)
{
    // Make a circular base type.
    context->roots.typeType = tuuvm_type_createAnonymous(context);
    tuuvm_tuple_setType((tuuvm_object_tuple_t*)context->roots.typeType, context->roots.typeType);

    // Create the symbol and set type.
    context->roots.symbolType = tuuvm_type_createAnonymous(context);
    context->roots.setType = tuuvm_type_createAnonymous(context);

    // Set the name of the root basic type.
    tuuvm_type_setName(context->roots.typeType, tuuvm_symbol_internWithCString(context, "Type"));
    tuuvm_type_setName(context->roots.symbolType, tuuvm_symbol_internWithCString(context, "Symbol"));
    tuuvm_type_setName(context->roots.setType, tuuvm_symbol_internWithCString(context, "Set"));

    // Create other root basic types.
    context->roots.arrayType = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "Array"));
    context->roots.arrayListType = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "ArrayList"));
    context->roots.stringType = tuuvm_type_createWithName(context, tuuvm_symbol_internWithCString(context, "String"));
}

TUUVM_API tuuvm_context_t *tuuvm_context_create(void)
{
    tuuvm_context_t *context = (tuuvm_context_t*)calloc(1, sizeof(tuuvm_context_t));
    tuuvm_context_createBasicTypes(context);

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

tuuvm_object_tuple_t *tuuvm_context_allocateByteTuple(tuuvm_context_t *context, tuuvm_tuple_t type, size_t byteSize)
{
    if(!context) return 0;

    tuuvm_object_tuple_t *result = tuuvm_heap_allocateByteTuple(&context->heap, byteSize);
    if(result)
        tuuvm_tuple_setType(result, type);
    return result;
}

tuuvm_object_tuple_t *tuuvm_context_allocatePointerTuple(tuuvm_context_t *context, tuuvm_tuple_t type, size_t slotCount)
{
    if(!context) return 0;

    tuuvm_object_tuple_t *result = tuuvm_heap_allocatePointerTuple(&context->heap, slotCount);
    if(result)
        tuuvm_tuple_setType(result, type);
    return result;
}
