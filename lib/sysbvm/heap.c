#include "internal/heap.h"
#include "sysbvm/assert.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SYSBVM_HEAP_MIN_CHUNK_SIZE (4<<20)
#define SYSBVM_HEAP_FAST_GROWTH_THRESHOLD (SYSBVM_HEAP_MIN_CHUNK_SIZE*512ull)

#define SYSBVM_HEAP_CODE_ZONE_CHUNK_SIZE SYSBVM_HEAP_MIN_CHUNK_SIZE

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

static void *sysbvm_heap_allocateSystemMemory(size_t sizeToAllocate)
{
    return VirtualAlloc(NULL, sizeToAllocate, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

static void *sysbvm_heap_allocateSystemMemoryForCode(size_t sizeToAllocate)
{
    return VirtualAlloc(NULL, sizeToAllocate, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READ);
}

static void sysbvm_heap_freeSystemMemory(void *memory, size_t sizeToFree)
{
    (void)sizeToFree;
    VirtualFree(memory, 0, MEM_RELEASE);
}

static size_t sysbvm_heap_getSystemAllocationAlignment(void)
{
    SYSTEM_INFO systemInfo;
    memset(&systemInfo, 0, sizeof(systemInfo));
    GetSystemInfo(&systemInfo);
    return systemInfo.dwPageSize;
}

static void sysbvm_heap_lockCodePagesForWriting(void *codePointer, size_t size)
{
    size_t pageAlignment = sysbvm_heap_getSystemAllocationAlignment();
    uintptr_t startAddress = (uintptr_t)codePointer & (-pageAlignment);
    uintptr_t endAddress = ((uintptr_t)codePointer + size + pageAlignment - 1) & (-pageAlignment);

    DWORD oldProtection = 0;
    VirtualProtect((void*)startAddress, endAddress - startAddress, PAGE_READWRITE, &oldProtection);
}

static void sysbvm_heap_unlockCodePagesForExecution(void *codePointer, size_t size)
{
    size_t pageAlignment = sysbvm_heap_getSystemAllocationAlignment();
    uintptr_t startAddress = (uintptr_t)codePointer & (-pageAlignment);
    uintptr_t endAddress = ((uintptr_t)codePointer + size + pageAlignment - 1) & (-pageAlignment);

    DWORD oldProtection = 0;
    VirtualProtect((void*)startAddress, endAddress - startAddress, PAGE_EXECUTE_READ, &oldProtection);
}

#else

#include <sys/mman.h>
#include <unistd.h>

static void *sysbvm_heap_allocateSystemMemory(size_t sizeToAllocate)
{
    void *result = mmap(0, sizeToAllocate, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if(result == MAP_FAILED)
        return 0;

    return result;
}

static void *sysbvm_heap_allocateSystemMemoryForCode(size_t sizeToAllocate)
{
    void *result = mmap(0, sizeToAllocate, PROT_READ | PROT_EXEC, MAP_PRIVATE | MAP_ANON, -1, 0);
    if(result == MAP_FAILED)
        return 0;

    return result;
}

static void sysbvm_heap_freeSystemMemory(void *memory, size_t sizeToFree)
{
    munmap(memory, sizeToFree);
}

static size_t sysbvm_heap_getSystemAllocationAlignment(void)
{
    return getpagesize();
}

static void sysbvm_heap_lockCodePagesForWriting(void *codePointer, size_t size)
{
    size_t pageAlignment = sysbvm_heap_getSystemAllocationAlignment();
    uintptr_t startAddress = (uintptr_t)codePointer & (-pageAlignment);
    uintptr_t endAddress = ((uintptr_t)codePointer + size + pageAlignment - 1) & (-pageAlignment);

    mprotect((void*)startAddress, endAddress - startAddress, PROT_READ | PROT_WRITE);
}

static void sysbvm_heap_unlockCodePagesForExecution(void *codePointer, size_t size)
{
    size_t pageAlignment = sysbvm_heap_getSystemAllocationAlignment();
    uintptr_t startAddress = (uintptr_t)codePointer & (-pageAlignment);
    uintptr_t endAddress = ((uintptr_t)codePointer + size + pageAlignment - 1) & (-pageAlignment);

    mprotect((void*)startAddress, endAddress - startAddress, PROT_READ | PROT_EXEC);
}

#endif

static uintptr_t uintptrAlignedTo(uintptr_t pointer, size_t alignment)
{
    return (pointer + alignment - 1) & (~(alignment - 1));
}

static sysbvm_heap_chunk_t *sysbvm_heap_allocateChunkWithRequiredChunkCapacity(sysbvm_heap_t *heap, size_t chunkCapacity)
{
    chunkCapacity = uintptrAlignedTo(chunkCapacity, sysbvm_heap_getSystemAllocationAlignment());

    // Allocate the chunk.
    sysbvm_heap_chunk_t *newChunk = (sysbvm_heap_chunk_t*)sysbvm_heap_allocateSystemMemory(chunkCapacity);
    if(!newChunk)
        return 0;

    memset(newChunk, 0, sizeof(sysbvm_heap_chunk_t));
    newChunk->capacity = (uint32_t)chunkCapacity;
    newChunk->size = sizeof(sysbvm_heap_chunk_t);

    // Find the chunk insertion position;
    sysbvm_heap_chunk_t *chunkInsertionPosition = 0;
    for(sysbvm_heap_chunk_t *chunkPosition = heap->firstChunk; chunkPosition; chunkPosition = chunkPosition->nextChunk)
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
        SYSBVM_ASSERT(!heap->firstChunk && !heap->lastChunk);
        heap->firstChunk = heap->lastChunk = newChunk;
    }

    heap->totalSize += newChunk->size;
    heap->totalCapacity += newChunk->capacity;

    return newChunk;
}

static sysbvm_heap_chunk_t *sysbvm_heap_findOrAllocateChunkWithRequiredCapacity(sysbvm_heap_t *heap, size_t requiredCapacity, size_t requiredAlignment)
{
    // Find a chunk that can accomodate the allocation.
    for(sysbvm_heap_chunk_t *currentChunk = heap->firstChunk; currentChunk; currentChunk = currentChunk->nextChunk)
    {
        size_t remainingCapacity = uintptrAlignedTo(currentChunk->size, requiredAlignment) + requiredCapacity;
        if(remainingCapacity <= currentChunk->capacity)
            return currentChunk;
    }

    // Compute the chunk allocation size.
    size_t chunkCapacity = SYSBVM_HEAP_MIN_CHUNK_SIZE;
    size_t requiredChunkCapacity = requiredCapacity + sizeof(sysbvm_heap_chunk_t);
    if(requiredChunkCapacity > chunkCapacity)
        chunkCapacity = requiredChunkCapacity;

    return sysbvm_heap_allocateChunkWithRequiredChunkCapacity(heap, chunkCapacity);
}

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
    sysbvm_heap_chunk_t *allocationChunk = sysbvm_heap_findOrAllocateChunkWithRequiredCapacity(heap, allocationSize, allocationAlignment);
    if(!allocationChunk)
        return 0;

    size_t allocationOffset = uintptrAlignedTo(allocationChunk->size, allocationAlignment);
    size_t newChunkSize = allocationOffset + allocationSize;
    size_t chunkSizeDelta = newChunkSize - allocationChunk->size;
    allocationChunk->size = (uint32_t)newChunkSize;
    heap->totalSize += chunkSizeDelta;
    SYSBVM_ASSERT(allocationChunk->size <= allocationChunk->capacity);
    sysbvm_object_tuple_t *result = (sysbvm_object_tuple_t*)((uintptr_t)allocationChunk + allocationOffset);
    memset(result, 0, allocationSize);

    sysbvm_heap_checkForGCThreshold(heap);
    return result;
}

SYSBVM_API sysbvm_object_tuple_t *sysbvm_heap_allocateByteTuple(sysbvm_heap_t *heap, size_t byteSize)
{
    size_t allocationSize = sizeof(sysbvm_object_tuple_t) + byteSize;
    sysbvm_object_tuple_t *result = sysbvm_heap_allocateTupleWithRawSize(heap, allocationSize, 16);
    if(!result) return 0;

    result->header.typePointerAndFlags = SYSBVM_TUPLE_TYPE_BYTES_BIT | heap->gcWhiteColor;
    result->header.objectSize = byteSize;
    return result;
}

SYSBVM_API sysbvm_object_tuple_t *sysbvm_heap_allocatePointerTuple(sysbvm_heap_t *heap, size_t slotCount)
{
    size_t objectSize = slotCount*sizeof(sysbvm_object_tuple_t*);
    size_t allocationSize = sizeof(sysbvm_object_tuple_t) + objectSize;
    sysbvm_object_tuple_t *result = sysbvm_heap_allocateTupleWithRawSize(heap, allocationSize, 16);
    if(!result) return 0;

    result->header.typePointerAndFlags = heap->gcWhiteColor;
    result->header.objectSize = objectSize;
    return result;
}

sysbvm_tuple_t *sysbvm_heap_allocateGCRootTableEntry(sysbvm_heap_t *heap)
{
    if(!heap->gcRootTable)
    {
        heap->gcRootTable = (sysbvm_tuple_t*)sysbvm_heap_allocateSystemMemory(SYSBVM_HEAP_CODE_ZONE_CHUNK_SIZE);
        heap->gcRootTableCapacity = SYSBVM_HEAP_CODE_ZONE_CHUNK_SIZE / sizeof(sysbvm_tuple_t);
        heap->gcRootTableSize = 0;
    }

    if(heap->gcRootTableSize >= heap->gcRootTableCapacity)
        abort();
    
    sysbvm_tuple_t *result = heap->gcRootTable + heap->gcRootTableSize;
    *result = SYSBVM_NULL_TUPLE;
    ++heap->gcRootTableSize;
    return result;
}

void *sysbvm_heap_allocateAndLockCodeZone(sysbvm_heap_t *heap, size_t size, size_t alignment)
{
    if(!heap->codeZone)
    {
        heap->codeZone = (uint8_t*)sysbvm_heap_allocateSystemMemoryForCode(SYSBVM_HEAP_CODE_ZONE_CHUNK_SIZE);
        if(!heap->codeZone)
            abort();

        heap->codeZoneCapacity = SYSBVM_HEAP_CODE_ZONE_CHUNK_SIZE;
        heap->codeZoneSize = 0;
    }

    uintptr_t alignedOffset = uintptrAlignedTo(heap->codeZoneSize, alignment);
    if(alignedOffset + size > heap->codeZoneCapacity)
        abort();

    heap->codeZoneSize = alignedOffset + size;;

    uint8_t *result = heap->codeZone + alignedOffset;
    sysbvm_heap_lockCodePagesForWriting(result, size);
    return result;
}

void sysbvm_heap_lockCodeZone(sysbvm_heap_t *heap, void *codePointer, size_t size)
{
    (void)heap;
    sysbvm_heap_lockCodePagesForWriting(codePointer, size);
}

void sysbvm_heap_unlockCodeZone(sysbvm_heap_t *heap, void *codePointer, size_t size)
{
    (void)heap;
    sysbvm_heap_unlockCodePagesForExecution(codePointer, size);
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
}

void sysbvm_heap_destroy(sysbvm_heap_t *heap)
{
    sysbvm_heap_chunk_t *position = heap->firstChunk;
    while(position)
    {
        sysbvm_heap_chunk_t *chunkToFree = position;
        position = position->nextChunk;
        sysbvm_heap_freeSystemMemory(chunkToFree, chunkToFree->capacity);
    }

    if(heap->codeZone)
        sysbvm_heap_freeSystemMemory(heap->codeZone, heap->codeZoneCapacity);
}

void sysbvm_heapIterator_begin(sysbvm_heap_t *heap, sysbvm_heapIterator_t *iterator)
{
    iterator->heap = heap;
    iterator->chunk = heap->firstChunk;
    iterator->offset = uintptrAlignedTo(sizeof(sysbvm_heap_chunk_t), 16);
}

void sysbvm_heapIterator_beginWithPointer(sysbvm_heap_t *heap, sysbvm_tuple_t pointer, sysbvm_heapIterator_t *iterator)
{
    memset(iterator, 0, sizeof(*iterator));
    if(!pointer || sysbvm_tuple_isImmediate(pointer))
        return;

    sysbvm_heap_chunk_t *chunk = heap->firstChunk;
    for(; chunk; chunk = chunk->nextChunk)
    {
        sysbvm_tuple_t startAddress = (uintptr_t)chunk + uintptrAlignedTo(sizeof(sysbvm_heap_chunk_t), 16);
        sysbvm_tuple_t endAddress = (uintptr_t)chunk + chunk->size;
        if(startAddress <= pointer && pointer < endAddress)
            break;
    }

    if(!chunk)
        return;

    iterator->heap = heap;
    iterator->chunk = chunk;
    iterator->offset = pointer - (uintptr_t)iterator->chunk;
}

bool sysbvm_heapIterator_isAtEnd(sysbvm_heapIterator_t *iterator)
{
    return !iterator->heap || !iterator->chunk || (iterator->offset >= iterator->chunk->size && !iterator->chunk->nextChunk);
}

sysbvm_object_tuple_t *sysbvm_heapIterator_get(sysbvm_heapIterator_t *iterator)
{
    if(!iterator->chunk)
        return NULL;

    uintptr_t chunkAddress = (uintptr_t)iterator->chunk;
    return (sysbvm_object_tuple_t*)(chunkAddress + iterator->offset);
}

void sysbvm_heapIterator_advance(sysbvm_heapIterator_t *iterator)
{
    if(sysbvm_heapIterator_isAtEnd(iterator))
        return;

    size_t objectSize = sizeof(sysbvm_object_tuple_t) + sysbvm_heapIterator_get(iterator)->header.objectSize;
    
    size_t newUnalignedOffset = iterator->offset + objectSize;
    size_t newOffset = uintptrAlignedTo(newUnalignedOffset, 16);
    if(newOffset < iterator->chunk->size)
    {
        iterator->offset = newOffset;
    }
    else
    {
        iterator->chunk = iterator->chunk->nextChunk;
        if(iterator->chunk)
            iterator->offset = uintptrAlignedTo(sizeof(sysbvm_heap_chunk_t), 16);
        else
            iterator->offset = 0;
    }
}

bool sysbvm_heapIterator_advanceUntilInstanceWithType(sysbvm_heapIterator_t *iterator, sysbvm_tuple_t expectedType)
{
    while(!sysbvm_heapIterator_isAtEnd(iterator))
    {
        sysbvm_object_tuple_t *object = sysbvm_heapIterator_get(iterator);
        sysbvm_tuple_t objectType = object->header.typePointerAndFlags & SYSBVM_TUPLE_TYPE_POINTER_MASK;
        if(objectType == expectedType)
            return true;

        sysbvm_heapIterator_advance(iterator);
    }

    return false;
}

void sysbvm_heapIterator_compactionAdvance(sysbvm_heapIterator_t *iterator, size_t increment, sysbvm_object_tuple_t **outObjectPointer, bool commitNewSize)
{
    if(sysbvm_heapIterator_isAtEnd(iterator))
    {
        if(outObjectPointer)
            *outObjectPointer = NULL;
        return;
    }

    while(iterator->chunk)
    {
        size_t newUnalignedOffset = iterator->offset + increment;
        size_t newOffset = uintptrAlignedTo(newUnalignedOffset, 16);

        // Does it fit in the current chunk?
        if(newOffset <= iterator->chunk->capacity)
        {
            if(outObjectPointer)
                *outObjectPointer = (sysbvm_object_tuple_t*) ((uintptr_t)iterator->chunk + iterator->offset);

            iterator->offset = newOffset;
            if(newOffset == iterator->chunk->capacity)
            {
                if(commitNewSize)
                    iterator->chunk->size = iterator->offset;

                iterator->chunk = iterator->chunk->nextChunk;
                iterator->offset = iterator->chunk ? uintptrAlignedTo(sizeof(sysbvm_heap_chunk_t), 16) : 0;
            }
            else
            {
                iterator->offset = newOffset;
            }
            return;
        }

        if(commitNewSize)
            iterator->chunk->size = iterator->offset;

        iterator->chunk = iterator->chunk->nextChunk;
        iterator->offset = uintptrAlignedTo(sizeof(sysbvm_heap_chunk_t), 16);
    }

    SYSBVM_ASSERT(iterator->chunk && "Out of memory for compaction.");
}

void sysbvm_heapIterator_compactionFinish(sysbvm_heapIterator_t *iterator, bool commitNewSize)
{
    if(sysbvm_heapIterator_isAtEnd(iterator))
        return;

    if(commitNewSize && iterator->chunk)
        iterator->chunk->size = iterator->offset;

    if(commitNewSize)
    {
        // Reset the size of the remaining chunks.
        for(sysbvm_heap_chunk_t *chunk = iterator->chunk->nextChunk; chunk; chunk = chunk->nextChunk)
            chunk->size = uintptrAlignedTo(sizeof(sysbvm_heap_chunk_t), 16);
    }

    iterator->chunk = NULL;
    iterator->offset = 0;
}

static void sysbvm_heap_computeNextCollectionThreshold(sysbvm_heap_t *heap)
{
    heap->shouldAttemptToCollect = false;

    const size_t ChunkThreshold = SYSBVM_HEAP_MIN_CHUNK_SIZE * 5 / 100;
    const size_t NextChunkAllocateThreshold = SYSBVM_HEAP_MIN_CHUNK_SIZE * 50 / 100;
    if(heap->totalCapacity == 0)
    {
        heap->nextGCSizeThreshold = SYSBVM_HEAP_MIN_CHUNK_SIZE - ChunkThreshold;
    }
    else
    {
        size_t capacityDelta = heap->totalCapacity - heap->totalSize;
        if(capacityDelta < NextChunkAllocateThreshold)
            heap->nextGCSizeThreshold = heap->totalCapacity + SYSBVM_HEAP_MIN_CHUNK_SIZE - ChunkThreshold;
        else
            heap->nextGCSizeThreshold = heap->totalCapacity - ChunkThreshold;
    }

    if(heap->nextGCSizeThreshold < SYSBVM_HEAP_FAST_GROWTH_THRESHOLD - ChunkThreshold)
        heap->nextGCSizeThreshold = SYSBVM_HEAP_FAST_GROWTH_THRESHOLD - ChunkThreshold;
}

void sysbvm_heap_computeCompactionForwardingPointers(sysbvm_heap_t *heap)
{
    sysbvm_heapIterator_t compactedIterator = {0};
    sysbvm_heapIterator_t heapIterator = {0};
    sysbvm_heapIterator_begin(heap, &compactedIterator);
    sysbvm_heapIterator_begin(heap, &heapIterator);

    while(!sysbvm_heapIterator_isAtEnd(&heapIterator))
    {
        sysbvm_object_tuple_t *object = sysbvm_heapIterator_get(&heapIterator);
        sysbvm_heapIterator_advance(&heapIterator);

        if((object->header.typePointerAndFlags & SYSBVM_TUPLE_TYPE_GC_COLOR_MASK) == heap->gcBlackColor)
        {
            size_t objectSize = sizeof(sysbvm_object_tuple_t) + object->header.objectSize;

            sysbvm_object_tuple_t *compactedObject = NULL;
            sysbvm_heapIterator_compactionAdvance(&compactedIterator, objectSize, &compactedObject, false);
            object->header.forwardingPointer = (sysbvm_tuple_t)compactedObject;
        }
        else
        {
            // Replace with tombstone.
            object->header.forwardingPointer = SYSBVM_TOMBSTONE_TUPLE;
        }
    }
}

void sysbvm_heap_applyForwardingPointers(sysbvm_heap_t *heap)
{
    sysbvm_heapIterator_t heapIterator = {0};
    sysbvm_heapIterator_begin(heap, &heapIterator);

    while(!sysbvm_heapIterator_isAtEnd(&heapIterator))
    {
        sysbvm_object_tuple_t *object = sysbvm_heapIterator_get(&heapIterator);
        if((object->header.typePointerAndFlags & SYSBVM_TUPLE_TYPE_GC_COLOR_MASK) == heap->gcBlackColor)
        {
            // Apply the forwarding to the type pointer.
            sysbvm_tuple_t type = object->header.typePointerAndFlags & SYSBVM_TUPLE_TYPE_POINTER_MASK;
            if(sysbvm_tuple_isNonNullPointer(type))
                sysbvm_tuple_setType(object, SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(type)->header.forwardingPointer);

            // Apply the forwarding to the slots.
            if((object->header.typePointerAndFlags & SYSBVM_TUPLE_TYPE_BYTES_BIT) == 0)
            {
                size_t slotCount = object->header.objectSize / sizeof(sysbvm_tuple_t);
                sysbvm_tuple_t *slots = object->pointers;

                for(size_t i = 0; i < slotCount; ++i)
                {
                    if(sysbvm_tuple_isNonNullPointer(slots[i]))
                        slots[i] = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(slots[i])->header.forwardingPointer;
                }
            }
        }

        sysbvm_heapIterator_advance(&heapIterator);
    }
}

void sysbvm_heap_compact(sysbvm_heap_t *heap)
{
    sysbvm_heapIterator_t compactedIterator = {0};
    sysbvm_heapIterator_t heapIterator = {0};
    sysbvm_heapIterator_begin(heap, &compactedIterator);
    sysbvm_heapIterator_begin(heap, &heapIterator);

    while(!sysbvm_heapIterator_isAtEnd(&heapIterator))
    {
        sysbvm_object_tuple_t *object = sysbvm_heapIterator_get(&heapIterator);
        sysbvm_heapIterator_advance(&heapIterator);

        if((object->header.typePointerAndFlags & SYSBVM_TUPLE_TYPE_GC_COLOR_MASK) == heap->gcBlackColor)
        {
            size_t objectSize = sizeof(sysbvm_object_tuple_t) + object->header.objectSize;

            sysbvm_object_tuple_t *compactedObject = NULL;
            sysbvm_heapIterator_compactionAdvance(&compactedIterator, objectSize, &compactedObject, true);
            memmove(compactedObject, object, objectSize);
        }
    }

    sysbvm_heapIterator_compactionFinish(&compactedIterator, true);

    // Compute the new heap size.
    heap->totalSize = 0;
    heap->totalCapacity = 0;

    for(sysbvm_heap_chunk_t *chunk = heap->firstChunk; chunk; chunk = chunk->nextChunk)
    {
        heap->totalSize += chunk->size;
        heap->totalCapacity += chunk->capacity;
    }

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

void sysbvm_heap_relocateWithTable(sysbvm_heap_t *heap, sysbvm_heap_relocationTable_t *relocationTable)
{
    sysbvm_heapIterator_t heapIterator = {0};
    sysbvm_heapIterator_begin(heap, &heapIterator);

    while(!sysbvm_heapIterator_isAtEnd(&heapIterator))
    {
        sysbvm_object_tuple_t *object = sysbvm_heapIterator_get(&heapIterator);

        // Relocate type pointer.
        sysbvm_tuple_setType(object, sysbvm_heap_relocatePointerWithTable(relocationTable, object->header.typePointerAndFlags & SYSBVM_TUPLE_TYPE_POINTER_MASK));

        // Relocate the slots.
        if((object->header.typePointerAndFlags & SYSBVM_TUPLE_TYPE_BYTES_BIT) == 0)
        {
            size_t slotCount = object->header.objectSize / sizeof(sysbvm_tuple_t);
            sysbvm_tuple_t *slots = object->pointers;

            for(size_t i = 0; i < slotCount; ++i)
                slots[i] = sysbvm_heap_relocatePointerWithTable(relocationTable, slots[i]);
        }

        // Reset the GC color.
        object->header.forwardingPointer = SYSBVM_NULL_TUPLE;
        sysbvm_tuple_setGCColor((sysbvm_tuple_t)object, heap->gcWhiteColor);

        sysbvm_heapIterator_advance(&heapIterator);
    }
}

void sysbvm_heap_swapGCColors(sysbvm_heap_t *heap)
{
    uint32_t temp = heap->gcBlackColor;
    heap->gcBlackColor = heap->gcWhiteColor;
    heap->gcWhiteColor = temp;
}

void sysbvm_heap_dumpToFile(sysbvm_heap_t *heap, FILE *file)
{
    uint32_t chunkCount = 0;
    size_t chunkHeaderSize = uintptrAlignedTo(sizeof(sysbvm_heap_chunkRecord_t), 16);
    for(sysbvm_heap_chunk_t *chunk = heap->firstChunk; chunk; chunk = chunk->nextChunk)
    {
        if(chunk->size > chunkHeaderSize)
            ++chunkCount;
    }

    uint32_t destIndex = 0;
    sysbvm_heap_chunkRecord_t *chunkRecords = (sysbvm_heap_chunkRecord_t*)calloc(sizeof(sysbvm_heap_chunkRecord_t), chunkCount);
    for(sysbvm_heap_chunk_t *chunk = heap->firstChunk; chunk; chunk = chunk->nextChunk)
    {
        if(chunk->size <= chunkHeaderSize)
            continue;

        sysbvm_heap_chunkRecord_t *record = chunkRecords + destIndex++;
        record->address = (uintptr_t)chunk;
        record->capacity = chunk->capacity;
        record->size = chunk->size;
    }

    fwrite(&chunkCount, 4, 1, file);
    fwrite(chunkRecords, sizeof(sysbvm_heap_chunkRecord_t), chunkCount, file);
    for(sysbvm_heap_chunk_t *chunk = heap->firstChunk; chunk; chunk = chunk->nextChunk)
    {
        fwrite((void*)((uintptr_t)chunk + chunkHeaderSize), chunk->size - chunkHeaderSize, 1, file);
    }

    free(chunkRecords);
}

bool sysbvm_heap_loadFromFile(sysbvm_heap_t *heap, FILE *file, size_t numberOfRootsToRelocate, sysbvm_tuple_t *rootsToRelocate)
{
    sysbvm_heap_initialize(heap);

    uint32_t chunkCount = 0;
    if(fread(&chunkCount, 4, 1, file) != 1) return false;

    sysbvm_heap_chunkRecord_t *chunkRecords = (sysbvm_heap_chunkRecord_t*)calloc(sizeof(sysbvm_heap_chunkRecord_t), chunkCount);
    if(fread(chunkRecords, sizeof(sysbvm_heap_chunkRecord_t), chunkCount, file) != chunkCount) return false;

    sysbvm_heap_relocationTable_t relocationTable = {
        .entryCount = chunkCount,
        .entries = (sysbvm_heap_relocationRecord_t*)calloc(sizeof(sysbvm_heap_relocationRecord_t), chunkCount),
    };

    // Load the chunks.
    size_t chunkHeaderSize = uintptrAlignedTo(sizeof(sysbvm_heap_chunkRecord_t), 16);
    for(uint32_t i = 0; i < chunkCount; ++i)
    {
        sysbvm_heap_chunkRecord_t *record = chunkRecords + i;
        sysbvm_heap_chunk_t *allocatedChunk = sysbvm_heap_allocateChunkWithRequiredChunkCapacity(heap, record->capacity);

        if(fread((void*)((uintptr_t)allocatedChunk + chunkHeaderSize), record->size - chunkHeaderSize, 1, file) != 1)
            return false;
        allocatedChunk->size = record->size;

        sysbvm_heap_relocationRecord_t *relocationRecord = relocationTable.entries + i;
        relocationRecord->sourceStartAddress = record->address;
        relocationRecord->sourceEndAddress = relocationRecord->sourceStartAddress + record->size;
        relocationRecord->destinationAddress = (uintptr_t)allocatedChunk;
    }

    free(chunkRecords);

    // Relocate the roots.
    for(size_t i = 0; i < numberOfRootsToRelocate; ++i)
        rootsToRelocate[i] = sysbvm_heap_relocatePointerWithTable(&relocationTable, rootsToRelocate[i]);

    // Relocate the objects.
    sysbvm_heap_relocateWithTable(heap, &relocationTable);
    free(relocationTable.entries);
    return true;
}
