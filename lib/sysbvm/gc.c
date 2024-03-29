#include "sysbvm/gc.h"
#include "sysbvm/pic.h"
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
    sysbvm_dynarray_add(&context->markingStack, &pointer);
}

static void sysbvm_gc_markObjectContent(sysbvm_context_t *context, sysbvm_tuple_t pointer)
{
    // Mark the object type. We do not need to mark the immediate types since there are already present in the root object set.
    sysbvm_tuple_t objectType = sysbvm_tuple_getType(context, pointer);
    sysbvm_gc_markPointer(context, &objectType);

    // Do not traverse the slot of byte objects, and the slots of weak objects
    if(!sysbvm_tuple_isBytes(pointer))
    {
        sysbvm_object_tuple_t *objectTuple = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(pointer);

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
        sysbvm_tuple_t *slots = objectTuple->pointers;
        for(size_t i = 0; i < strongSlotCount; ++i)
            sysbvm_gc_markPointer(context, &slots[i]);
    }

    sysbvm_tuple_setGCColor(pointer, context->heap.gcBlackColor);
}

static void sysbvm_gc_markUntilStackIsEmpty(sysbvm_context_t *context)
{
    while(context->markingStack.size > 0)
    {
        sysbvm_tuple_t *pendingObjects = (sysbvm_tuple_t*)context->markingStack.data;
        sysbvm_tuple_t nextPendingObject = pendingObjects[--context->markingStack.size];
        sysbvm_gc_markObjectContent(context, nextPendingObject);
    }
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
    // Phase 1: marking phase
    sysbvm_gc_iterateRoots(context, context, sysbvm_gc_markPointer);
    sysbvm_gc_markUntilStackIsEmpty(context);

    // Phase 2: Replace the weak references with their tombstones.
    sysbvm_heap_replaceWeakReferencesWithTombstones(&context->heap);

    // Phase 3: Sweep
    sysbvm_heap_sweep(&context->heap);

    // Phase 4: Swap the GC colors.
    sysbvm_heap_swapGCColors(&context->heap);
}

SYSBVM_API void sysbvm_gc_safepoint(sysbvm_context_t *context)
{
    if(sysbvm_gc_perThreadLockCount != 0)
        return;

    // Check the attempt collection flag on the heap.
    if(!context->heap.shouldAttemptToCollect || context->gcDisabled)
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
    }

    // GC Root table
    {
        sysbvm_chunkedAllocatorIterator_t iterator;
        for(sysbvm_chunkedAllocatorIterator_begin(&context->heap.gcRootTableAllocator, &iterator);
            sysbvm_chunkedAllocatorIterator_isValid(&iterator);
            sysbvm_chunkedAllocatorIterator_advance(&iterator))
        {
            size_t entryCount = iterator.size / sizeof(sysbvm_tuple_t);
            sysbvm_tuple_t *entries = (sysbvm_tuple_t*)iterator.data;
            for(size_t i = 0; i < entryCount; ++i)
                iterationFunction(userdata, entries + i);
        }
    }

    // PIC table
    {
        sysbvm_chunkedAllocatorIterator_t iterator;
        for(sysbvm_chunkedAllocatorIterator_begin(&context->heap.picTableAllocator, &iterator);
            sysbvm_chunkedAllocatorIterator_isValid(&iterator);
            sysbvm_chunkedAllocatorIterator_advance(&iterator))
        {
            size_t picCount = iterator.size / sizeof(sysbvm_pic_t);
            sysbvm_pic_t *pics = (sysbvm_pic_t*)iterator.data;
            for(size_t i = 0; i < picCount; ++i)
            {
                sysbvm_pic_t *pic = pics + i;
                for(size_t j = 0; j < SYSBVM_PIC_ENTRY_COUNT; ++j)
                {
                    sysbvm_picEntry_t *picEntry = pic->entries + j;
                    iterationFunction(userdata, &picEntry->selector);
                    iterationFunction(userdata, &picEntry->type);
                    iterationFunction(userdata, &picEntry->method);
                }
            }
        }
    }

    // Stack roots.
    sysbvm_stackFrame_iterateGCRootsInStackWith(sysbvm_stackFrame_getActiveRecord(), userdata, iterationFunction);
}