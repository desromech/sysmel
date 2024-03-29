#include "internal/heap.h"
#include "internal/virtualMemory.h"
#include "sysbvm/assert.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SYSBVM_HEAP_MIN_CHUNK_SIZE (2<<20)
#define SYSBVM_HEAP_STARTUP_HEAP_SIZE (SYSBVM_HEAP_MIN_CHUNK_SIZE*4)
#define SYSBVM_HEAP_COLLECTION_GAMMA_FACTOR 3

#define SYSBVM_HEAP_CODE_ZONE_SIZE (16<<20)

static void sysbvm_heap_checkForGCThreshold(sysbvm_heap_t *heap)
{
    if(heap->shouldAttemptToCollect)
        return;

    // Monitor for GC collection threshold.
    heap->shouldAttemptToCollect = heap->totalSize > heap->nextGCSizeThreshold;
    //if(heap->shouldAttemptToCollect)
    //    printf("heap->shouldAttemptToCollect %zu > %zu | %zu\n", heap->totalSize, heap->nextGCSizeThreshold, heap->totalCapacity);
}

static sysbvm_object_tuple_t *sysbvm_heap_allocateTupleWithRawSize(sysbvm_heap_t *heap, size_t allocationSize, size_t allocationAlignment)
{
    (void)allocationAlignment;
    SYSBVM_ASSERT(allocationSize >= sizeof(sysbvm_object_tuple_t));
    sysbvm_object_tuple_t *result = NULL;

    size_t allocationWithHeaderSize = sizeof(sysbvm_heap_mallocObjectHeader_t) + allocationSize;
    sysbvm_heap_mallocObjectHeader_t *resultHeader = malloc(allocationWithHeaderSize);
    resultHeader->next = NULL;
    resultHeader->size = allocationWithHeaderSize;
    if(heap->firstMallocObject)
    {
        heap->lastMallocObject->next = resultHeader;
        heap->lastMallocObject = resultHeader;
    }
    else
    {
        heap->firstMallocObject = heap->lastMallocObject = resultHeader;
    }

    result = (sysbvm_object_tuple_t *)((uintptr_t)resultHeader + 16);
    heap->totalSize += allocationWithHeaderSize;

    memset(result, 0, allocationSize);
    sysbvm_heap_checkForGCThreshold(heap);
    return result;
}

SYSBVM_API sysbvm_object_tuple_t *sysbvm_heap_allocateByteTuple(sysbvm_heap_t *heap, size_t byteSize)
{
    size_t allocationSize = sizeof(sysbvm_object_tuple_t) + byteSize;
    sysbvm_object_tuple_t *result = sysbvm_heap_allocateTupleWithRawSize(heap, allocationSize, 16);
    if(!result) return 0;

    result->header.identityHashAndFlags = (SYSBVM_TUPLE_OBJECT_KIND_BYTES << SYSBVM_TUPLE_OBJECT_KIND_SHIFT) | (heap->gcWhiteColor << SYSBVM_TUPLE_GC_COLOR_SHIFT);
    result->header.objectSize = byteSize;
    return result;
}

SYSBVM_API sysbvm_object_tuple_t *sysbvm_heap_allocatePointerTuple(sysbvm_heap_t *heap, size_t slotCount)
{
    size_t objectSize = slotCount*sizeof(sysbvm_object_tuple_t*);
    size_t allocationSize = sizeof(sysbvm_object_tuple_t) + objectSize;
    sysbvm_object_tuple_t *result = sysbvm_heap_allocateTupleWithRawSize(heap, allocationSize, 16);
    if(!result) return 0;

    result->header.identityHashAndFlags = (SYSBVM_TUPLE_OBJECT_KIND_POINTERS << SYSBVM_TUPLE_OBJECT_KIND_SHIFT) | (heap->gcWhiteColor << SYSBVM_TUPLE_GC_COLOR_SHIFT);
    result->header.objectSize = objectSize;
    return result;
}

sysbvm_tuple_t *sysbvm_heap_allocateGCRootTableEntry(sysbvm_heap_t *heap)
{
    sysbvm_tuple_t *result = sysbvm_chunkedAllocator_allocate(&heap->gcRootTableAllocator, sizeof(sysbvm_tuple_t), sizeof(sysbvm_tuple_t));
    *result = SYSBVM_NULL_TUPLE;
    return result;
}

SYSBVM_API sysbvm_object_tuple_t *sysbvm_heap_shallowCopyTuple(sysbvm_heap_t *heap, sysbvm_object_tuple_t *tupleToCopy)
{
    size_t objectSize = tupleToCopy->header.objectSize;
    size_t allocationSize = sizeof(sysbvm_object_tuple_t) + objectSize;

    sysbvm_object_tuple_t *result = sysbvm_heap_allocateTupleWithRawSize(heap, allocationSize, 16);
    if(!result) return 0;

    memcpy(result, tupleToCopy, allocationSize);
    sysbvm_tuple_setGCColor((sysbvm_tuple_t)result, heap->gcWhiteColor);
    return result;
}

void sysbvm_heap_initialize(sysbvm_heap_t *heap)
{
    heap->gcWhiteColor = 0;
    heap->gcGrayColor = 1;
    heap->gcBlackColor = 2;

    sysbvm_chunkedAllocator_initialize(&heap->gcRootTableAllocator, SYSBVM_CHUNKED_ALLOCATOR_DEFAULT_CHUNK_SIZE, false);
    sysbvm_chunkedAllocator_initialize(&heap->picTableAllocator, SYSBVM_CHUNKED_ALLOCATOR_DEFAULT_CHUNK_SIZE, false);
    sysbvm_chunkedAllocator_initialize(&heap->codeAllocator, SYSBVM_CHUNKED_ALLOCATOR_DEFAULT_CHUNK_SIZE, true);
}

void sysbvm_heap_destroy(sysbvm_heap_t *heap)
{
    {
        sysbvm_heap_mallocObjectHeader_t *position = heap->firstMallocObject;
        while(position)
        {
            sysbvm_heap_mallocObjectHeader_t *objectToFree = position;
            position = position->next;
            free(objectToFree);
        }
    }

    sysbvm_chunkedAllocator_destroy(&heap->gcRootTableAllocator);
    sysbvm_chunkedAllocator_destroy(&heap->picTableAllocator);
    sysbvm_chunkedAllocator_destroy(&heap->codeAllocator);
}

static void sysbvm_heap_computeNextCollectionThreshold(sysbvm_heap_t *heap)
{
    heap->shouldAttemptToCollect = false;

    size_t liveDataSize = heap->totalSize;
    if(liveDataSize < SYSBVM_HEAP_STARTUP_HEAP_SIZE)
        liveDataSize = SYSBVM_HEAP_STARTUP_HEAP_SIZE;
    heap->nextGCSizeThreshold = liveDataSize * SYSBVM_HEAP_COLLECTION_GAMMA_FACTOR;
}

void sysbvm_heap_replaceWeakReferencesWithTombstones(sysbvm_heap_t *heap)
{
    sysbvm_heap_mallocObjectHeader_t *objectHeader = heap->firstMallocObject;
    while(objectHeader)
    {
        sysbvm_object_tuple_t *object = (sysbvm_object_tuple_t*)(objectHeader + 1);
        if(sysbvm_tuple_getGCColor((sysbvm_tuple_t)object) == heap->gcBlackColor)
        {
            // Only check the slots of weak objects.
            if(sysbvm_tuple_isWeakObject((sysbvm_tuple_t)object))
            {
                size_t slotCount = object->header.objectSize / sizeof(sysbvm_tuple_t);
                sysbvm_tuple_t *slots = object->pointers;

                for(size_t i = 0; i < slotCount; ++i)
                {
                    if(sysbvm_tuple_isNonNullPointer(slots[i]) && sysbvm_tuple_getGCColor(slots[i]) != heap->gcBlackColor)
                        slots[i] = SYSBVM_TOMBSTONE_TUPLE;
                }
            }
        }

        objectHeader = objectHeader->next;
    }
}

static void sysbvm_heap_performSweep(sysbvm_heap_t *heap)
{
    sysbvm_heap_mallocObjectHeader_t *position = heap->firstMallocObject;
    heap->firstMallocObject = heap->lastMallocObject = NULL;

    while(position)
    {
        sysbvm_heap_mallocObjectHeader_t *next = position->next;
        position->next = NULL;

        sysbvm_object_tuple_t *object = (sysbvm_object_tuple_t*) (position + 1);
        if(sysbvm_tuple_getGCColor((sysbvm_tuple_t)object) == heap->gcBlackColor)
        {
            if(heap->firstMallocObject)
            {
                heap->lastMallocObject->next = position;
                heap->lastMallocObject = position;
            }
            else
            {
                heap->firstMallocObject = heap->lastMallocObject = position;
            }
        }
        else
        {
            SYSBVM_ASSERT(heap->totalSize >= position->size);
            heap->totalSize -= position->size;
            free(position);
        }

        position = next;
    }
}

void sysbvm_heap_sweep(sysbvm_heap_t *heap)
{
    sysbvm_heap_performSweep(heap);
    sysbvm_heap_computeNextCollectionThreshold(heap);
}

static inline sysbvm_heap_relocationRecord_t *sysbvm_heap_relocationTable_findRecord(sysbvm_heap_relocationTable_t *relocationTable, uintptr_t address)
{
    size_t count = relocationTable->entryCount;
    for(size_t i = 0; i < count; ++i)
    {
        sysbvm_heap_relocationRecord_t *record = relocationTable->entries + i;
        if(record->sourceStartAddress <= address && address < record->sourceEndAddress)
            return record;
    }

    return NULL;
}

static inline sysbvm_tuple_t sysbvm_heap_relocatePointerWithTable(sysbvm_heap_relocationTable_t *relocationTable, sysbvm_tuple_t pointer)
{
    if(!sysbvm_tuple_isNonNullPointer(pointer))
        return pointer;
    
    sysbvm_heap_relocationRecord_t *record = sysbvm_heap_relocationTable_findRecord(relocationTable, pointer);
    return pointer - record->sourceStartAddress + record->destinationAddress;
}

void sysbvm_heap_swapGCColors(sysbvm_heap_t *heap)
{
    uint32_t temp = heap->gcBlackColor;
    heap->gcBlackColor = heap->gcWhiteColor;
    heap->gcWhiteColor = temp;
}
