#include "tuuvm/gc.h"
#include "internal/context.h"
#include <stdio.h>

TUUVM_THREAD_LOCAL uint32_t tuuvm_gc_perThreadLockCount;

static void tuuvm_gc_markPointer(void *userdata, tuuvm_tuple_t *pointerAddress)
{
    tuuvm_context_t *context = (tuuvm_context_t*)userdata;

    tuuvm_tuple_t pointer = *pointerAddress;
    if(!tuuvm_tuple_isNonNullPointer(pointer))
        return;

    bool isWhite = tuuvm_tuple_getGCColor(pointer) == context->heap.gcWhiteColor;
    if(!isWhite)
        return;

    // In the case of byte tuples, change immediately onto black.
    if(tuuvm_tuple_isBytes(pointer))
    {
        tuuvm_tuple_setGCColor(pointer, context->heap.gcBlackColor);
        return;
    }

    tuuvm_tuple_setGCColor(pointer, context->heap.gcGrayColor);

    // TODO: Use a explicit stack here.
    size_t slotCount = tuuvm_tuple_getSizeInSlots(pointer);
    tuuvm_tuple_t *slots = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(pointer)->pointers;
    for(size_t i = 0; i < slotCount; ++i)
        tuuvm_gc_markPointer(userdata, &slots[i]);

    tuuvm_tuple_setGCColor(pointer, context->heap.gcBlackColor);
}

static void tuuvm_gc_applyForwardingPointer(void *userdata, tuuvm_tuple_t *pointerAddress)
{
    tuuvm_context_t *context = (tuuvm_context_t*)userdata;

    tuuvm_tuple_t pointer = *pointerAddress;
    if(!tuuvm_tuple_isNonNullPointer(pointer))
        return;

    *pointerAddress = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(pointer)->header.forwardingPointer;
}

TUUVM_API void tuuvm_gc_collect(tuuvm_context_t *context)
{
    context->heap.shouldAttemptToCollect = true;
    tuuvm_gc_safepoint(context);
}

void tuuvm_gc_debugStackValidationHook(void)
{
}

static void tuuvm_gc_performCycle(tuuvm_context_t *context)
{
    // LISP 2 moving collection algorithm.
    // Phase 1: marking phase
    tuuvm_gc_iterateRoots(context, context, tuuvm_gc_markPointer);

    // Phase 2: Compute the forwarding pointers.
    tuuvm_heap_computeCompactionForwardingPointers(&context->heap);

    // Phase 3: Relocate pointers.
    tuuvm_gc_iterateRoots(context, context, tuuvm_gc_applyForwardingPointer);
    tuuvm_heap_applyForwardingPointers(&context->heap);

    // Phase 4: Sweep
    tuuvm_heap_compact(&context->heap);

    // Phase 5: Swap the GC colors.
    tuuvm_heap_swapGCColors(&context->heap);
}

TUUVM_API void tuuvm_gc_safepoint(tuuvm_context_t *context)
{
    if(tuuvm_gc_perThreadLockCount != 0)
        return;

    // Check the attempt collection flag on the heap.
    if(!context->heap.shouldAttemptToCollect)
        return;

    // TODO: Add Support for multiple threads.

    // Hook location for validating GC stack roots via GDB scripting.
    tuuvm_gc_debugStackValidationHook();
    tuuvm_gc_performCycle(context);
}

TUUVM_API void tuuvm_gc_lock(tuuvm_context_t *context)
{
    (void)context;
    ++tuuvm_gc_perThreadLockCount;
    if(tuuvm_gc_perThreadLockCount != 1)
        return;
}

TUUVM_API void tuuvm_gc_unlock(tuuvm_context_t *context)
{
    --tuuvm_gc_perThreadLockCount;
    if(tuuvm_gc_perThreadLockCount != 0)
        return;

    tuuvm_gc_safepoint(context);
}

TUUVM_API void tuuvm_gc_iterateRoots(tuuvm_context_t *context, void *userdata, tuuvm_GCRootIterationFunction_t iterationFunction)
{
    // Context roots.
    {
        tuuvm_tuple_t *contextRoots = (tuuvm_tuple_t*)&context->roots;
        size_t contextRootCount = sizeof(context->roots) / sizeof(tuuvm_tuple_t);
        for(size_t i = 0; i < contextRootCount; ++i)
            iterationFunction(userdata, &contextRoots[i]);
    }

    // Stack roots.
    tuuvm_stackFrame_iterateGCRootsInStackWith(tuuvm_stackFrame_getActiveRecord(), userdata, iterationFunction);
}