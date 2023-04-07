#include "sysbvm/gc.h"
#include "sysbvm/type.h"
#include "internal/context.h"
#include <stdio.h>

SYSBVM_THREAD_LOCAL uint32_t sysbvm_gc_perThreadLockCount;

static void sysbvm_gc_markPointer(void *userdata, sysbvm_tuple_t *pointerAddress)
{
    sysbvm_context_t *context = (sysbvm_context_t*)userdata;

    sysbvm_tuple_t pointer = *pointerAddress;
    if(!sysbvm_tuple_isNonNullPointer(pointer))
        return;


    bool isWhite = sysbvm_tuple_getGCColor(pointer) == context->heap.gcWhiteColor;
    if(!isWhite)
        return;

    sysbvm_tuple_setGCColor(pointer, context->heap.gcGrayColor);

    // Mark the object type. We do not need to mark the immediate types since there are already present in the root object set.
    sysbvm_tuple_t objectType = sysbvm_tuple_getType(context, pointer);
    sysbvm_gc_markPointer(userdata, &objectType);

    // Do not traverse the slot of byte objects, and the slots of weak objects
    if(!sysbvm_tuple_isBytes(pointer))
    {
        // By default mark all of the slots.
        size_t strongSlotCount = sysbvm_tuple_getSizeInSlots(pointer);

        // Keep the declared slots as strong.
        if(sysbvm_tuple_isWeakObject(pointer))
        {
            strongSlotCount = 0;
            if(sysbvm_tuple_isNonNullPointer(objectType))
                strongSlotCount = sysbvm_type_getTotalSlotCount(objectType);
        }

        // Mark the object slots
        sysbvm_tuple_t *slots = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(pointer)->pointers;
        for(size_t i = 0; i < strongSlotCount; ++i)
            sysbvm_gc_markPointer(userdata, &slots[i]);
    }

    sysbvm_tuple_setGCColor(pointer, context->heap.gcBlackColor);
}

static void sysbvm_gc_applyForwardingPointer(void *userdata, sysbvm_tuple_t *pointerAddress)
{
    (void)userdata;
    //sysbvm_context_t *context = (sysbvm_context_t*)userdata;

    sysbvm_tuple_t pointer = *pointerAddress;
    if(!sysbvm_tuple_isNonNullPointer(pointer))
        return;

    *pointerAddress = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(pointer)->header.forwardingPointer;
}

SYSBVM_API void sysbvm_gc_collect(sysbvm_context_t *context)
{
    context->heap.shouldAttemptToCollect = true;
    sysbvm_gc_safepoint(context);
}

void sysbvm_gc_debugStackValidationHook(void)
{
}

static void sysbvm_gc_performCycle(sysbvm_context_t *context)
{
    return;
    // LISP 2 moving collection algorithm.
    // Phase 1: marking phase
    sysbvm_gc_iterateRoots(context, context, sysbvm_gc_markPointer);

    // Phase 2: Compute the forwarding pointers.
    sysbvm_heap_computeCompactionForwardingPointers(&context->heap);

    // Phase 3: Relocate pointers.
    sysbvm_gc_iterateRoots(context, context, sysbvm_gc_applyForwardingPointer);
    sysbvm_heap_applyForwardingPointers(&context->heap);

    // Phase 4: Sweep
    sysbvm_heap_compact(&context->heap);

    // Phase 5: Swap the GC colors.
    sysbvm_heap_swapGCColors(&context->heap);
}

SYSBVM_API void sysbvm_gc_safepoint(sysbvm_context_t *context)
{
    if(sysbvm_gc_perThreadLockCount != 0)
        return;

    // Check the attempt collection flag on the heap.
    if(!context->heap.shouldAttemptToCollect)
        return;

    // TODO: Add Support for multiple threads.

    // Hook location for validating GC stack roots via GDB scripting.
    sysbvm_gc_debugStackValidationHook();
    sysbvm_gc_performCycle(context);
}

SYSBVM_API void sysbvm_gc_lock(sysbvm_context_t *context)
{
    (void)context;
    ++sysbvm_gc_perThreadLockCount;
    if(sysbvm_gc_perThreadLockCount != 1)
        return;
}

SYSBVM_API void sysbvm_gc_unlock(sysbvm_context_t *context)
{
    --sysbvm_gc_perThreadLockCount;
    if(sysbvm_gc_perThreadLockCount != 0)
        return;

    sysbvm_gc_safepoint(context);
}

SYSBVM_API void sysbvm_gc_iterateRoots(sysbvm_context_t *context, void *userdata, sysbvm_GCRootIterationFunction_t iterationFunction)
{
    // Context roots.
    {
        sysbvm_tuple_t *contextRoots = (sysbvm_tuple_t*)&context->roots;
        size_t contextRootCount = sizeof(context->roots) / sizeof(sysbvm_tuple_t);
        for(size_t i = 0; i < contextRootCount; ++i)
            iterationFunction(userdata, &contextRoots[i]);

        for(size_t i = 0; i < context->heap.gcRootTableSize; ++i)
            iterationFunction(userdata, context->heap.gcRootTable + i);
    }

    // Stack roots.
    sysbvm_stackFrame_iterateGCRootsInStackWith(sysbvm_stackFrame_getActiveRecord(), userdata, iterationFunction);
}