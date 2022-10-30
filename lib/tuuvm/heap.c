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

    return newChunk;
}

static tuuvm_object_tuple_t *tuuvm_heap_allocateTupleWithRawSize(tuuvm_heap_t *heap, size_t allocationSize, size_t allocationAlignment)
{
    tuuvm_heap_chunk_t *allocationChunk = tuuvm_heap_findOrAllocateChunkWithRequiredCapacity(heap, allocationSize, allocationAlignment);
    if(!allocationChunk)
        return 0;

    size_t allocationOffset = uintptrAlignedTo(allocationChunk->size, allocationAlignment);
    allocationChunk->size = (uint32_t)(allocationOffset + allocationSize);
    TUUVM_ASSERT(allocationChunk->size <= allocationChunk->capacity);
    return (tuuvm_object_tuple_t*)((uintptr_t)allocationChunk + allocationOffset);
}

TUUVM_API tuuvm_object_tuple_t *tuuvm_heap_allocateByteTuple(tuuvm_heap_t *heap, size_t byteSize)
{
    size_t allocationSize = sizeof(tuuvm_object_tuple_t) + byteSize;
    tuuvm_object_tuple_t *result = tuuvm_heap_allocateTupleWithRawSize(heap, allocationSize, 16);
    if(!result) return 0;

    result->header.typePointerAndFlags = TUUVM_TUPLE_BYTES_BIT;
    result->header.objectSize = byteSize;
    return result;
}

TUUVM_API tuuvm_object_tuple_t *tuuvm_heap_allocatePointerTuple(tuuvm_heap_t *heap, size_t slotCount)
{
    size_t objectSize = slotCount*sizeof(tuuvm_object_tuple_t*);
    size_t allocationSize = sizeof(tuuvm_object_tuple_t) + objectSize;
    tuuvm_object_tuple_t *result = tuuvm_heap_allocateTupleWithRawSize(heap, allocationSize, 16);
    if(!result) return 0;

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
    return result;
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
