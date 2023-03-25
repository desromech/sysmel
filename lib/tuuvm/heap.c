#include "internal/heap.h"
#include "tuuvm/assert.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TUUVM_HEAP_MIN_CHUNK_SIZE (4<<20)
#define TUUVM_HEAP_FAST_GROWTH_THRESHOLD (TUUVM_HEAP_MIN_CHUNK_SIZE*4)

#define TUUVM_HEAP_CODE_ZONE_CHUNK_SIZE TUUVM_HEAP_MIN_CHUNK_SIZE

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

static void *tuuvm_heap_allocateSystemMemoryForCode(size_t sizeToAllocate)
{
    void *result = mmap(0, sizeToAllocate, PROT_READ | PROT_EXEC, MAP_PRIVATE | MAP_ANON | MAP_EXECUTABLE, -1, 0);
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

static void tuuvm_heap_lockCodePagesForWriting(void *codePointer, size_t size)
{
    size_t pageAlignment = tuuvm_heap_getSystemAllocationAlignment();
    uintptr_t startAddress = (uintptr_t)codePointer & (-pageAlignment);
    uintptr_t endAddress = ((uintptr_t)codePointer + size + pageAlignment - 1) & (-pageAlignment);

    mprotect((void*)startAddress, endAddress - startAddress, PROT_READ | PROT_WRITE);
}

static void tuuvm_heap_unlockCodePagesForExecution(void *codePointer, size_t size)
{
    size_t pageAlignment = tuuvm_heap_getSystemAllocationAlignment();
    uintptr_t startAddress = (uintptr_t)codePointer & (-pageAlignment);
    uintptr_t endAddress = ((uintptr_t)codePointer + size + pageAlignment - 1) & (-pageAlignment);

    mprotect((void*)startAddress, endAddress - startAddress, PROT_READ | PROT_EXEC);
}

#endif

static uintptr_t uintptrAlignedTo(uintptr_t pointer, size_t alignment)
{
    return (pointer + alignment - 1) & (~(alignment - 1));
}

static tuuvm_heap_chunk_t *tuuvm_heap_allocateChunkWithRequiredChunkCapacity(tuuvm_heap_t *heap, size_t chunkCapacity)
{
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

    return newChunk;
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

    return tuuvm_heap_allocateChunkWithRequiredChunkCapacity(heap, chunkCapacity);
}

static void tuuvm_heap_checkForGCThreshold(tuuvm_heap_t *heap)
{
    if(heap->shouldAttemptToCollect)
        return;

    // Monitor for GC collection threshold.
    heap->shouldAttemptToCollect = heap->totalSize > heap->nextGCSizeThreshold;
    //if(heap->shouldAttemptToCollect)
    //    printf("heap->shouldAttemptToCollect %zu > %zu | %zu\n", heap->totalSize, heap->nextGCSizeThreshold, heap->totalCapacity);
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

    result->header.typePointerAndFlags = TUUVM_TUPLE_TYPE_BYTES_BIT | heap->gcWhiteColor;
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

tuuvm_tuple_t *tuuvm_heap_allocateGCRootTableEntry(tuuvm_heap_t *heap)
{
    if(!heap->gcRootTable)
    {
        heap->gcRootTable = (tuuvm_tuple_t*)tuuvm_heap_allocateSystemMemory(TUUVM_HEAP_CODE_ZONE_CHUNK_SIZE);
        heap->gcRootTableCapacity = TUUVM_HEAP_CODE_ZONE_CHUNK_SIZE / sizeof(tuuvm_tuple_t);
        heap->gcRootTableSize = 0;
    }

    if(heap->gcRootTableSize >= heap->gcRootTableCapacity)
        abort();
    
    tuuvm_tuple_t *result = heap->gcRootTable + heap->gcRootTableSize;
    *result = TUUVM_NULL_TUPLE;
    ++heap->gcRootTableSize;
    return result;
}

void *tuuvm_heap_allocateAndLockCodeZone(tuuvm_heap_t *heap, size_t size, size_t alignment)
{
    if(!heap->codeZone)
    {
        heap->codeZone = (uint8_t*)tuuvm_heap_allocateSystemMemoryForCode(TUUVM_HEAP_CODE_ZONE_CHUNK_SIZE);
        if(!heap->codeZone)
            abort();

        heap->codeZoneCapacity = TUUVM_HEAP_CODE_ZONE_CHUNK_SIZE;
        heap->codeZoneSize = 0;
    }

    uintptr_t alignedOffset = uintptrAlignedTo(heap->codeZoneSize, alignment);
    if(alignedOffset + size > heap->codeZoneCapacity)
        abort();

    heap->codeZoneSize = alignedOffset + size;;

    uint8_t *result = heap->codeZone + alignedOffset;
    tuuvm_heap_lockCodePagesForWriting(result, size);
    return result;
}

void tuuvm_heap_unlockCodeZone(tuuvm_heap_t *heap, void *codePointer, size_t size)
{
    (void)heap;
    tuuvm_heap_unlockCodePagesForExecution(codePointer, size);
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

    if(heap->codeZone)
        tuuvm_heap_freeSystemMemory(heap->codeZone, heap->codeZoneCapacity);
}

void tuuvm_heapIterator_begin(tuuvm_heap_t *heap, tuuvm_heapIterator_t *iterator)
{
    iterator->heap = heap;
    iterator->chunk = heap->firstChunk;
    iterator->offset = uintptrAlignedTo(sizeof(tuuvm_heap_chunk_t), 16);
}

void tuuvm_heapIterator_beginWithPointer(tuuvm_heap_t *heap, tuuvm_tuple_t pointer, tuuvm_heapIterator_t *iterator)
{
    memset(iterator, 0, sizeof(*iterator));
    if(!pointer || tuuvm_tuple_isImmediate(pointer))
        return;

    tuuvm_heap_chunk_t *chunk = heap->firstChunk;
    for(; chunk; chunk = chunk->nextChunk)
    {
        tuuvm_tuple_t startAddress = (uintptr_t)chunk + uintptrAlignedTo(sizeof(tuuvm_heap_chunk_t), 16);
        tuuvm_tuple_t endAddress = (uintptr_t)chunk + chunk->size;
        if(startAddress <= pointer && pointer < endAddress)
            break;
    }

    if(!chunk)
        return;

    iterator->heap = heap;
    iterator->chunk = chunk;
    iterator->offset = pointer - (uintptr_t)iterator->chunk;
}

bool tuuvm_heapIterator_isAtEnd(tuuvm_heapIterator_t *iterator)
{
    return !iterator->heap || !iterator->chunk || (iterator->offset >= iterator->chunk->size && !iterator->chunk->nextChunk);
}

tuuvm_object_tuple_t *tuuvm_heapIterator_get(tuuvm_heapIterator_t *iterator)
{
    if(!iterator->chunk)
        return NULL;

    uintptr_t chunkAddress = (uintptr_t)iterator->chunk;
    return (tuuvm_object_tuple_t*)(chunkAddress + iterator->offset);
}

void tuuvm_heapIterator_advance(tuuvm_heapIterator_t *iterator)
{
    if(tuuvm_heapIterator_isAtEnd(iterator))
        return;

    size_t objectSize = sizeof(tuuvm_object_tuple_t) + tuuvm_heapIterator_get(iterator)->header.objectSize;
    
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
            iterator->offset = uintptrAlignedTo(sizeof(tuuvm_heap_chunk_t), 16);
        else
            iterator->offset = 0;
    }
}

bool tuuvm_heapIterator_advanceUntilInstanceWithType(tuuvm_heapIterator_t *iterator, tuuvm_tuple_t expectedType)
{
    while(!tuuvm_heapIterator_isAtEnd(iterator))
    {
        tuuvm_object_tuple_t *object = tuuvm_heapIterator_get(iterator);
        tuuvm_tuple_t objectType = object->header.typePointerAndFlags & TUUVM_TUPLE_TYPE_POINTER_MASK;
        if(objectType == expectedType)
            return true;

        tuuvm_heapIterator_advance(iterator);
    }

    return false;
}

void tuuvm_heapIterator_compactionAdvance(tuuvm_heapIterator_t *iterator, size_t increment, tuuvm_object_tuple_t **outObjectPointer, bool commitNewSize)
{
    if(tuuvm_heapIterator_isAtEnd(iterator))
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
                *outObjectPointer = (tuuvm_object_tuple_t*) ((uintptr_t)iterator->chunk + iterator->offset);

            iterator->offset = newOffset;
            if(newOffset == iterator->chunk->capacity)
            {
                if(commitNewSize)
                    iterator->chunk->size = iterator->offset;

                iterator->chunk = iterator->chunk->nextChunk;
                iterator->offset = iterator->chunk ? uintptrAlignedTo(sizeof(tuuvm_heap_chunk_t), 16) : 0;
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
        iterator->offset = uintptrAlignedTo(sizeof(tuuvm_heap_chunk_t), 16);
    }

    TUUVM_ASSERT(iterator->chunk && "Out of memory for compaction.");
}

void tuuvm_heapIterator_compactionFinish(tuuvm_heapIterator_t *iterator, bool commitNewSize)
{
    if(tuuvm_heapIterator_isAtEnd(iterator))
        return;

    if(commitNewSize && iterator->chunk)
        iterator->chunk->size = iterator->offset;

    if(commitNewSize)
    {
        // Reset the size of the remaining chunks.
        for(tuuvm_heap_chunk_t *chunk = iterator->chunk->nextChunk; chunk; chunk = chunk->nextChunk)
            chunk->size = uintptrAlignedTo(sizeof(tuuvm_heap_chunk_t), 16);
    }

    iterator->chunk = NULL;
    iterator->offset = 0;
}

static void tuuvm_heap_computeNextCollectionThreshold(tuuvm_heap_t *heap)
{
    heap->shouldAttemptToCollect = false;

    const size_t ChunkThreshold = TUUVM_HEAP_MIN_CHUNK_SIZE * 5 / 100;
    const size_t NextChunkAllocateThreshold = TUUVM_HEAP_MIN_CHUNK_SIZE * 50 / 100;
    if(heap->totalCapacity == 0)
    {
        heap->nextGCSizeThreshold = TUUVM_HEAP_MIN_CHUNK_SIZE - ChunkThreshold;
    }
    else
    {
        size_t capacityDelta = heap->totalCapacity - heap->totalSize;
        if(capacityDelta < NextChunkAllocateThreshold)
            heap->nextGCSizeThreshold = heap->totalCapacity + TUUVM_HEAP_MIN_CHUNK_SIZE - ChunkThreshold;
        else
            heap->nextGCSizeThreshold = heap->totalCapacity - ChunkThreshold;
    }

    if(heap->nextGCSizeThreshold < TUUVM_HEAP_FAST_GROWTH_THRESHOLD - ChunkThreshold)
        heap->nextGCSizeThreshold = TUUVM_HEAP_FAST_GROWTH_THRESHOLD - ChunkThreshold;
}

void tuuvm_heap_computeCompactionForwardingPointers(tuuvm_heap_t *heap)
{
    tuuvm_heapIterator_t compactedIterator = {};
    tuuvm_heapIterator_t heapIterator = {};
    tuuvm_heapIterator_begin(heap, &compactedIterator);
    tuuvm_heapIterator_begin(heap, &heapIterator);

    while(!tuuvm_heapIterator_isAtEnd(&heapIterator))
    {
        tuuvm_object_tuple_t *object = tuuvm_heapIterator_get(&heapIterator);
        tuuvm_heapIterator_advance(&heapIterator);

        if((object->header.typePointerAndFlags & TUUVM_TUPLE_TYPE_GC_COLOR_MASK) == heap->gcBlackColor)
        {
            size_t objectSize = sizeof(tuuvm_object_tuple_t) + object->header.objectSize;

            tuuvm_object_tuple_t *compactedObject = NULL;
            tuuvm_heapIterator_compactionAdvance(&compactedIterator, objectSize, &compactedObject, false);
            object->header.forwardingPointer = (tuuvm_tuple_t)compactedObject;
        }
        else
        {
            // Replace with tombstone.
            object->header.forwardingPointer = TUUVM_TOMBSTONE_TUPLE;
        }
    }
}

void tuuvm_heap_applyForwardingPointers(tuuvm_heap_t *heap)
{
    tuuvm_heapIterator_t heapIterator = {};
    tuuvm_heapIterator_begin(heap, &heapIterator);

    while(!tuuvm_heapIterator_isAtEnd(&heapIterator))
    {
        tuuvm_object_tuple_t *object = tuuvm_heapIterator_get(&heapIterator);
        if((object->header.typePointerAndFlags & TUUVM_TUPLE_TYPE_GC_COLOR_MASK) == heap->gcBlackColor)
        {
            // Apply the forwarding to the type pointer.
            tuuvm_tuple_t type = object->header.typePointerAndFlags & TUUVM_TUPLE_TYPE_POINTER_MASK;
            if(tuuvm_tuple_isNonNullPointer(type))
                tuuvm_tuple_setType(object, TUUVM_CAST_OOP_TO_OBJECT_TUPLE(type)->header.forwardingPointer);

            // Apply the forwarding to the slots.
            if((object->header.typePointerAndFlags & TUUVM_TUPLE_TYPE_BYTES_BIT) == 0)
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

        tuuvm_heapIterator_advance(&heapIterator);
    }
}

void tuuvm_heap_compact(tuuvm_heap_t *heap)
{
    tuuvm_heapIterator_t compactedIterator = {};
    tuuvm_heapIterator_t heapIterator = {};
    tuuvm_heapIterator_begin(heap, &compactedIterator);
    tuuvm_heapIterator_begin(heap, &heapIterator);

    while(!tuuvm_heapIterator_isAtEnd(&heapIterator))
    {
        tuuvm_object_tuple_t *object = tuuvm_heapIterator_get(&heapIterator);
        tuuvm_heapIterator_advance(&heapIterator);

        if((object->header.typePointerAndFlags & TUUVM_TUPLE_TYPE_GC_COLOR_MASK) == heap->gcBlackColor)
        {
            size_t objectSize = sizeof(tuuvm_object_tuple_t) + object->header.objectSize;

            tuuvm_object_tuple_t *compactedObject = NULL;
            tuuvm_heapIterator_compactionAdvance(&compactedIterator, objectSize, &compactedObject, true);
            memmove(compactedObject, object, objectSize);
        }
    }

    tuuvm_heapIterator_compactionFinish(&compactedIterator, true);

    // Compute the new heap size.
    heap->totalSize = 0;
    heap->totalCapacity = 0;

    for(tuuvm_heap_chunk_t *chunk = heap->firstChunk; chunk; chunk = chunk->nextChunk)
    {
        heap->totalSize += chunk->size;
        heap->totalCapacity += chunk->capacity;
    }

    tuuvm_heap_computeNextCollectionThreshold(heap);
}

static inline tuuvm_heap_relocationRecord_t *tuuvm_heap_relocationTable_findRecord(tuuvm_heap_relocationTable_t *relocationTable, uintptr_t address)
{
    size_t count = relocationTable->entryCount;
    for(size_t i = 0; i < count; ++i)
    {
        tuuvm_heap_relocationRecord_t *record = relocationTable->entries + i;
        if(record->sourceStartAddress <= address && address < record->sourceEndAddress)
            return record;
    }

    return NULL;
}

static inline tuuvm_tuple_t tuuvm_heap_relocatePointerWithTable(tuuvm_heap_relocationTable_t *relocationTable, tuuvm_tuple_t pointer)
{
    if(!tuuvm_tuple_isNonNullPointer(pointer))
        return pointer;
    
    tuuvm_heap_relocationRecord_t *record = tuuvm_heap_relocationTable_findRecord(relocationTable, pointer);
    return pointer - record->sourceStartAddress + record->destinationAddress;
}

void tuuvm_heap_relocateWithTable(tuuvm_heap_t *heap, tuuvm_heap_relocationTable_t *relocationTable)
{
    tuuvm_heapIterator_t heapIterator = {};
    tuuvm_heapIterator_begin(heap, &heapIterator);

    while(!tuuvm_heapIterator_isAtEnd(&heapIterator))
    {
        tuuvm_object_tuple_t *object = tuuvm_heapIterator_get(&heapIterator);

        // Relocate type pointer.
        tuuvm_tuple_setType(object, tuuvm_heap_relocatePointerWithTable(relocationTable, object->header.typePointerAndFlags & TUUVM_TUPLE_TYPE_POINTER_MASK));

        // Relocate the slots.
        if((object->header.typePointerAndFlags & TUUVM_TUPLE_TYPE_BYTES_BIT) == 0)
        {
            size_t slotCount = object->header.objectSize / sizeof(tuuvm_tuple_t);
            tuuvm_tuple_t *slots = object->pointers;

            for(size_t i = 0; i < slotCount; ++i)
                slots[i] = tuuvm_heap_relocatePointerWithTable(relocationTable, slots[i]);
        }

        // Reset the GC color.
        object->header.forwardingPointer = TUUVM_NULL_TUPLE;
        tuuvm_tuple_setGCColor((tuuvm_tuple_t)object, heap->gcWhiteColor);

        tuuvm_heapIterator_advance(&heapIterator);
    }
}

void tuuvm_heap_swapGCColors(tuuvm_heap_t *heap)
{
    uint32_t temp = heap->gcBlackColor;
    heap->gcBlackColor = heap->gcWhiteColor;
    heap->gcWhiteColor = temp;
}

void tuuvm_heap_dumpToFile(tuuvm_heap_t *heap, FILE *file)
{
    uint32_t chunkCount = 0;
    size_t chunkHeaderSize = uintptrAlignedTo(sizeof(tuuvm_heap_chunkRecord_t), 16);
    for(tuuvm_heap_chunk_t *chunk = heap->firstChunk; chunk; chunk = chunk->nextChunk)
    {
        if(chunk->size > chunkHeaderSize)
            ++chunkCount;
    }

    uint32_t destIndex = 0;
    tuuvm_heap_chunkRecord_t *chunkRecords = (tuuvm_heap_chunkRecord_t*)calloc(sizeof(tuuvm_heap_chunkRecord_t), chunkCount);
    for(tuuvm_heap_chunk_t *chunk = heap->firstChunk; chunk; chunk = chunk->nextChunk)
    {
        if(chunk->size <= chunkHeaderSize)
            continue;

        tuuvm_heap_chunkRecord_t *record = chunkRecords + destIndex++;
        record->address = (uintptr_t)chunk;
        record->capacity = chunk->capacity;
        record->size = chunk->size;
    }

    fwrite(&chunkCount, 4, 1, file);
    fwrite(chunkRecords, sizeof(tuuvm_heap_chunkRecord_t), chunkCount, file);
    for(tuuvm_heap_chunk_t *chunk = heap->firstChunk; chunk; chunk = chunk->nextChunk)
    {
        fwrite((void*)((uintptr_t)chunk + chunkHeaderSize), chunk->size - chunkHeaderSize, 1, file);
    }

    free(chunkRecords);
}

bool tuuvm_heap_loadFromFile(tuuvm_heap_t *heap, FILE *file, size_t numberOfRootsToRelocate, tuuvm_tuple_t *rootsToRelocate)
{
    tuuvm_heap_initialize(heap);

    uint32_t chunkCount = 0;
    if(fread(&chunkCount, 4, 1, file) != 1) return false;

    tuuvm_heap_chunkRecord_t *chunkRecords = (tuuvm_heap_chunkRecord_t*)calloc(sizeof(tuuvm_heap_chunkRecord_t), chunkCount);
    if(fread(chunkRecords, sizeof(tuuvm_heap_chunkRecord_t), chunkCount, file) != chunkCount) return false;

    tuuvm_heap_relocationTable_t relocationTable = {
        .entryCount = chunkCount,
        .entries = (tuuvm_heap_relocationRecord_t*)calloc(sizeof(tuuvm_heap_relocationRecord_t), chunkCount),
    };

    // Load the chunks.
    size_t chunkHeaderSize = uintptrAlignedTo(sizeof(tuuvm_heap_chunkRecord_t), 16);
    for(uint32_t i = 0; i < chunkCount; ++i)
    {
        tuuvm_heap_chunkRecord_t *record = chunkRecords + i;
        tuuvm_heap_chunk_t *allocatedChunk = tuuvm_heap_allocateChunkWithRequiredChunkCapacity(heap, record->capacity);

        if(fread((void*)((uintptr_t)allocatedChunk + chunkHeaderSize), record->size - chunkHeaderSize, 1, file) != 1)
            return false;
        allocatedChunk->size = record->size;

        tuuvm_heap_relocationRecord_t *relocationRecord = relocationTable.entries + i;
        relocationRecord->sourceStartAddress = record->address;
        relocationRecord->sourceEndAddress = relocationRecord->sourceStartAddress + record->size;
        relocationRecord->destinationAddress = (uintptr_t)allocatedChunk;
    }

    free(chunkRecords);

    // Relocate the roots.
    for(size_t i = 0; i < numberOfRootsToRelocate; ++i)
        rootsToRelocate[i] = tuuvm_heap_relocatePointerWithTable(&relocationTable, rootsToRelocate[i]);

    // Relocate the objects.
    tuuvm_heap_relocateWithTable(heap, &relocationTable);
    free(relocationTable.entries);
    return true;
}
