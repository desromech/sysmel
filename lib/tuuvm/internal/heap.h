#ifndef TUUVM_INTERNAL_HEAP_H
#define TUUVM_INTERNAL_HEAP_H

#pragma once

#include "tuuvm/heap.h"
#include <stdio.h>

typedef struct tuuvm_heap_chunk_s
{
    uint32_t capacity;
    uint32_t size;
    struct tuuvm_heap_chunk_s *nextChunk;
} tuuvm_heap_chunk_t;

struct tuuvm_heap_s
{
    tuuvm_heap_chunk_t *firstChunk;
    tuuvm_heap_chunk_t *lastChunk;
    bool shouldAttemptToCollect;

    size_t totalSize;
    size_t totalCapacity;
    size_t nextGCSizeThreshold;

    uint32_t gcWhiteColor;
    uint32_t gcGrayColor;
    uint32_t gcBlackColor;

    size_t gcRootTableCapacity;
    size_t gcRootTableSize;
    tuuvm_tuple_t *gcRootTable;

    size_t codeZoneCapacity;
    size_t codeZoneSize;
    uint8_t *codeZone;
};

typedef struct tuuvm_heapIterator_s
{
    tuuvm_heap_t *heap;
    tuuvm_heap_chunk_t *chunk;
    size_t offset;
} tuuvm_heapIterator_t;

typedef struct tuuvm_heap_relocationRecord_s
{
    uintptr_t sourceStartAddress;
    uintptr_t sourceEndAddress;
    uintptr_t destinationAddress;
} tuuvm_heap_relocationRecord_t;

typedef struct tuuvm_heap_relocationTable_s
{
    size_t entryCount;
    tuuvm_heap_relocationRecord_t *entries;
} tuuvm_heap_relocationTable_t;

typedef struct tuuvm_heap_chunkRecord_s
{
    uintptr_t address;
    uint32_t capacity;
    uint32_t size;
} tuuvm_heap_chunkRecord_t;

void tuuvm_heap_initialize(tuuvm_heap_t *heap);
void tuuvm_heap_destroy(tuuvm_heap_t *heap);

tuuvm_tuple_t *tuuvm_heap_allocateGCRootTableEntry(tuuvm_heap_t *heap);

void *tuuvm_heap_allocateAndLockCodeZone(tuuvm_heap_t *heap, size_t size, size_t alignment);
void tuuvm_heap_unlockCodeZone(tuuvm_heap_t *heap, void *codePointer, size_t size);

void tuuvm_heap_computeCompactionForwardingPointers(tuuvm_heap_t *heap);
void tuuvm_heap_applyForwardingPointers(tuuvm_heap_t *heap);
void tuuvm_heap_compact(tuuvm_heap_t *heap);
void tuuvm_heap_swapGCColors(tuuvm_heap_t *heap);

void tuuvm_heap_dumpToFile(tuuvm_heap_t *heap, FILE *file);
bool tuuvm_heap_loadFromFile(tuuvm_heap_t *heap, FILE *file, size_t numberOfRootsToRelocate, tuuvm_tuple_t *rootsToRelocate);

void tuuvm_heapIterator_begin(tuuvm_heap_t *heap, tuuvm_heapIterator_t *iterator);
void tuuvm_heapIterator_beginWithPointer(tuuvm_heap_t *heap, tuuvm_tuple_t pointer, tuuvm_heapIterator_t *iterator);
bool tuuvm_heapIterator_isAtEnd(tuuvm_heapIterator_t *iterator);

tuuvm_object_tuple_t *tuuvm_heapIterator_get(tuuvm_heapIterator_t *iterator);
void tuuvm_heapIterator_advance(tuuvm_heapIterator_t *iterator);
bool tuuvm_heapIterator_advanceUntilInstanceWithType(tuuvm_heapIterator_t *iterator, tuuvm_tuple_t expectedType);
void tuuvm_heapIterator_compactionAdvance(tuuvm_heapIterator_t *iterator, size_t sizeToAdvance, tuuvm_object_tuple_t **outObjectPointer, bool commitNewSize);
void tuuvm_heapIterator_compactionFinish(tuuvm_heapIterator_t *iterator, bool commitNewSize);

#endif //TUUVM_INTERNAL_HEAP_H
