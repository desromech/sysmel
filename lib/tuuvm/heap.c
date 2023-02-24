#include "internal/heap.h"
#include "tuuvm/assert.h"
#include <stdlib.h>
#include <string.h>

#define TUUVM_HEAP_MIN_CHUNK_SIZE (1<<20)

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

static void *tuuvm_heap_allocateSystemMemory(size_t sizeToAllocate)
{
    return VirtualAlloc(NULL, sizeToAllocate, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

static void tuuvm_heap_freeSystemMemory(void *memory, size_t sizeToFree)
{
    (void)sizeToFree;
    VirtualFree(memory, 0, MEM_RELEASE);
}

static size_t tuuvm_heap_getSystemAllocationAlignment(void)
{
    SYSTEM_INFO systemInfo;
    memset(&systemInfo, 0, sizeof(systemInfo));
    GetSystemInfo(&systemInfo);
    return systemInfo.dwPageSize;
}

#else

#include <sys/mman.h>
#include <unistd.h>

static void *tuuvm_heap_allocateSystemMemory(size_t sizeToAllocate)
{
    void *result = mmap(0, sizeToAllocate, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if(result == MAP_FAILED)
        return 0;

    return result;
}

static void tuuvm_heap_freeSystemMemory(void *memory, size_t sizeToFree)
{
    munmap(memory, sizeToFree);
}

static size_t tuuvm_heap_getSystemAllocationAlignment(void)
{
    return getpagesize();
}

#endif

static uintptr_t uintptrAlignedTo(uintptr_t pointer, size_t alignment)
{
    return (pointer + alignment - 1) & (~(alignment - 1));
}

static tuuvm_heap_chunk_t *tuuvm_heap_findOrAllocateChunkWithRequiredCapacity(tuuvm_heap_t *heap, size_t requiredCapacity, size_t requiredAlignment)
{
    // Find a chunk that can accomodate the allocation.
    for(tuuvm_heap_chunk_t *currentChunk = heap->firstChunk; currentChunk; currentChunk = currentChunk->nextChunk)
    {
        size_t remainingCapacity = uintptrAlignedTo(currentChunk->size, requiredAlignment) + requiredCapacity;
        if(remainingCapacity <= currentChunk->capacity)
            return currentChunk;
    }

    // Compute the chunk allocation size.
    size_t chunkCapacity = TUUVM_HEAP_MIN_CHUNK_SIZE;
    size_t requiredChunkCapacity = requiredCapacity + sizeof(tuuvm_heap_chunk_t);
    if(requiredChunkCapacity > chunkCapacity)
        chunkCapacity = requiredChunkCapacity;

    chunkCapacity = uintptrAlignedTo(chunkCapacity, tuuvm_heap_getSystemAllocationAlignment());

    // Allocate the chunk.
    tuuvm_heap_chunk_t *newChunk = (tuuvm_heap_chunk_t*)tuuvm_heap_allocateSystemMemory(chunkCapacity);
    if(!newChunk)
        return 0;

    memset(newChunk, 0, sizeof(tuuvm_heap_chunk_t));
    newChunk->capacity = (uint32_t)chunkCapacity;
    newChunk->size = sizeof(tuuvm_heap_chunk_t);

    // Find the chunk insertion position;
    tuuvm_heap_chunk_t *chunkInsertionPosition = 0;
    for(tuuvm_heap_chunk_t *chunkPosition = heap->firstChunk; chunkPosition; chunkPosition = chunkPosition->nextChunk)
    {
        if(chunkPosition < newChunk && newChunk >= chunkInsertionPosition)
            chunkInsertionPosition = chunkPosition;
    }

    // Insert the new chunk into the linked list of chunks.
    if(!chunkInsertionPosition)
        chunkInsertionPosition = heap->lastChunk;

    if(chunkInsertionPosition)
    {
        newChunk->nextChunk = chunkInsertionPosition->nextChunk;
        chunkInsertionPosition->nextChunk = newChunk;
        if(chunkInsertionPosition == heap->lastChunk)
            heap->lastChunk = chunkInsertionPosition;
    }
    else
    {
        TUUVM_ASSERT(!heap->firstChunk && !heap->lastChunk);
        heap->firstChunk = heap->lastChunk = newChunk;
    }

    heap->totalSize += newChunk->size;
    heap->totalCapacity += newChunk->capacity;
    heap->shouldAttemptToCollect = true; // For each new chunk set the GC flag.

    return newChunk;
}

static void tuuvm_heap_checkForGCThreshold(tuuvm_heap_t *heap)
{
    // Monitor for 80% of heap comsumption.
    heap->shouldAttemptToCollect = heap->shouldAttemptToCollect || heap->totalSize > heap->totalCapacity * 4 / 5;
    //heap->shouldAttemptToCollect = true;
}

static tuuvm_object_tuple_t *tuuvm_heap_allocateTupleWithRawSize(tuuvm_heap_t *heap, size_t allocationSize, size_t allocationAlignment)
{
    tuuvm_heap_chunk_t *allocationChunk = tuuvm_heap_findOrAllocateChunkWithRequiredCapacity(heap, allocationSize, allocationAlignment);
    if(!allocationChunk)
        return 0;

    size_t allocationOffset = uintptrAlignedTo(allocationChunk->size, allocationAlignment);
    size_t newChunkSize = allocationOffset + allocationSize;
    size_t chunkSizeDelta = newChunkSize - allocationChunk->size;
    allocationChunk->size = (uint32_t)newChunkSize;
    heap->totalSize += chunkSizeDelta;
    TUUVM_ASSERT(allocationChunk->size <= allocationChunk->capacity);
    tuuvm_object_tuple_t *result = (tuuvm_object_tuple_t*)((uintptr_t)allocationChunk + allocationOffset);
    memset(result, 0, allocationSize);

    tuuvm_heap_checkForGCThreshold(heap);
    return result;
}

TUUVM_API tuuvm_object_tuple_t *tuuvm_heap_allocateByteTuple(tuuvm_heap_t *heap, size_t byteSize)
{
    size_t allocationSize = sizeof(tuuvm_object_tuple_t) + byteSize;
    tuuvm_object_tuple_t *result = tuuvm_heap_allocateTupleWithRawSize(heap, allocationSize, 16);
    if(!result) return 0;

    result->header.typePointerAndFlags = TUUVM_TUPLE_BYTES_BIT | heap->gcWhiteColor;
    result->header.objectSize = byteSize;
    return result;
}

TUUVM_API tuuvm_object_tuple_t *tuuvm_heap_allocatePointerTuple(tuuvm_heap_t *heap, size_t slotCount)
{
    size_t objectSize = slotCount*sizeof(tuuvm_object_tuple_t*);
    size_t allocationSize = sizeof(tuuvm_object_tuple_t) + objectSize;
    tuuvm_object_tuple_t *result = tuuvm_heap_allocateTupleWithRawSize(heap, allocationSize, 16);
    if(!result) return 0;

    result->header.typePointerAndFlags = heap->gcWhiteColor;
    result->header.objectSize = objectSize;
    return result;
}

TUUVM_API tuuvm_object_tuple_t *tuuvm_heap_shallowCopyTuple(tuuvm_heap_t *heap, tuuvm_object_tuple_t *tupleToCopy)
{
    size_t objectSize = tupleToCopy->header.objectSize;
    size_t allocationSize = sizeof(tuuvm_object_tuple_t) + objectSize;

    tuuvm_object_tuple_t *result = tuuvm_heap_allocateTupleWithRawSize(heap, allocationSize, 16);
    if(!result) return 0;

    memcpy(result, tupleToCopy, allocationSize);
    tuuvm_tuple_setGCColor((tuuvm_tuple_t)result, heap->gcWhiteColor);
    return result;
}

void tuuvm_heap_initialize(tuuvm_heap_t *heap)
{
    heap->gcWhiteColor = 0;
    heap->gcGrayColor = 1;
    heap->gcBlackColor = 2;
}

void tuuvm_heap_destroy(tuuvm_heap_t *heap)
{
    tuuvm_heap_chunk_t *position = heap->firstChunk;
    while(position)
    {
        tuuvm_heap_chunk_t *chunkToFree = position;
        position = position->nextChunk;
        tuuvm_heap_freeSystemMemory(chunkToFree, chunkToFree->capacity);
    }
}

static void tuuvm_heap_chunk_computeCompactionForwardingPointers(uint32_t blackColor, tuuvm_heap_chunk_t *chunk)
{
    uintptr_t offset = sizeof(tuuvm_heap_chunk_t);
    uintptr_t newSize = offset;
    uintptr_t chunkAddress = (uintptr_t)chunk;
    while(offset < chunk->size)
    {
        offset = uintptrAlignedTo(offset, 16);
        if(offset >= chunk->size)
            break;

        tuuvm_object_tuple_t *object = (tuuvm_object_tuple_t*)(chunkAddress + offset);
        size_t objectSize = sizeof(tuuvm_object_tuple_t) + object->header.objectSize;
        offset += objectSize;
        if((object->header.typePointerAndFlags & TUUVM_TUPLE_GC_COLOR_MASK) == blackColor)
        {
            newSize = uintptrAlignedTo(newSize, 16);
            object->header.forwardingPointer = chunkAddress + newSize;
            newSize += objectSize;
        }
    }
}

void tuuvm_heap_computeCompactionForwardingPointers(tuuvm_heap_t *heap)
{
    for(tuuvm_heap_chunk_t *chunk = heap->firstChunk; chunk; chunk = chunk->nextChunk)
        tuuvm_heap_chunk_computeCompactionForwardingPointers(heap->gcBlackColor, chunk);
}

static void tuuvm_heap_chunk_applyForwardingPointers(uint32_t blackColor, tuuvm_heap_chunk_t *chunk)
{
    uintptr_t offset = sizeof(tuuvm_heap_chunk_t);
    uintptr_t chunkAddress = (uintptr_t)chunk;
    while(offset < chunk->size)
    {
        offset = uintptrAlignedTo(offset, 16);
        if(offset >= chunk->size)
            break;

        tuuvm_object_tuple_t *object = (tuuvm_object_tuple_t*)(chunkAddress + offset);
        size_t objectSize = sizeof(tuuvm_object_tuple_t) + object->header.objectSize;
        offset += objectSize;
        if((object->header.typePointerAndFlags & TUUVM_TUPLE_GC_COLOR_MASK) == blackColor &&
            (object->header.typePointerAndFlags & TUUVM_TUPLE_BYTES_BIT) == 0)
        {
            size_t slotCount = object->header.objectSize / sizeof(tuuvm_tuple_t);
            tuuvm_tuple_t *slots = object->pointers;

            for(size_t i = 0; i < slotCount; ++i)
            {
                if(tuuvm_tuple_isNonNullPointer(slots[i]))
                    slots[i] = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(slots[i])->header.forwardingPointer;
            }
        }
    }
}

void tuuvm_heap_applyForwardingPointers(tuuvm_heap_t *heap)
{
    for(tuuvm_heap_chunk_t *chunk = heap->firstChunk; chunk; chunk = chunk->nextChunk)
        tuuvm_heap_chunk_applyForwardingPointers(heap->gcBlackColor, chunk);
}

static void tuuvm_heap_chunk_compact(uint32_t blackColor, tuuvm_heap_chunk_t *chunk)
{
    uintptr_t offset = sizeof(tuuvm_heap_chunk_t);
    uintptr_t newSize = offset;
    uintptr_t chunkAddress = (uintptr_t)chunk;
    while(offset < chunk->size)
    {
        offset = uintptrAlignedTo(offset, 16);
        if(offset >= chunk->size)
            break;

        tuuvm_object_tuple_t *object = (tuuvm_object_tuple_t*)(chunkAddress + offset);
        size_t objectSize = sizeof(tuuvm_object_tuple_t) + object->header.objectSize;
        offset += objectSize;
        if((object->header.typePointerAndFlags & TUUVM_TUPLE_GC_COLOR_MASK) == blackColor)
        {
            newSize = uintptrAlignedTo(newSize, 16);
            tuuvm_object_tuple_t *targetObject = (tuuvm_object_tuple_t*)(chunkAddress + newSize);
            TUUVM_ASSERT(object->header.forwardingPointer == (tuuvm_tuple_t)targetObject);
            if(targetObject != object)
                memmove(targetObject, object, objectSize);
            newSize += objectSize;
        }
    }

    TUUVM_ASSERT(newSize <= chunk->capacity);
    TUUVM_ASSERT(newSize <= chunk->size);
    chunk->size = newSize;
}

void tuuvm_heap_compact(tuuvm_heap_t *heap)
{
    heap->totalSize = 0;
    heap->totalCapacity = 0;

    for(tuuvm_heap_chunk_t *chunk = heap->firstChunk; chunk; chunk = chunk->nextChunk)
    {
        tuuvm_heap_chunk_compact(heap->gcBlackColor, chunk);

        heap->totalSize += chunk->size;
        heap->totalCapacity += chunk->capacity;
    }
    
    heap->shouldAttemptToCollect = false;
}

void tuuvm_heap_swapGCColors(tuuvm_heap_t *heap)
{
    uint32_t temp = heap->gcBlackColor;
    heap->gcBlackColor = heap->gcWhiteColor;
    heap->gcWhiteColor = temp;
}
