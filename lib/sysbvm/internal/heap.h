#ifndef SYSBVM_INTERNAL_HEAP_H
#define SYSBVM_INTERNAL_HEAP_H

#pragma once

#include "sysbvm/heap.h"
#include <stdio.h>

typedef struct sysbvm_heap_chunk_s
{
    uint32_t capacity;
    uint32_t size;
    struct sysbvm_heap_chunk_s *nextChunk;
} sysbvm_heap_chunk_t;

struct sysbvm_heap_s
{
    sysbvm_heap_chunk_t *firstChunk;
    sysbvm_heap_chunk_t *lastChunk;
    bool shouldAttemptToCollect;

    size_t totalSize;
    size_t totalCapacity;
    size_t nextGCSizeThreshold;

    uint32_t gcWhiteColor;
    uint32_t gcGrayColor;
    uint32_t gcBlackColor;

    size_t gcRootTableCapacity;
    size_t gcRootTableSize;
    sysbvm_tuple_t *gcRootTable;

    size_t codeZoneCapacity;
    size_t codeZoneSize;
    uint8_t *codeZone;
};

typedef struct sysbvm_heapIterator_s
{
    sysbvm_heap_t *heap;
    sysbvm_heap_chunk_t *chunk;
    size_t offset;
} sysbvm_heapIterator_t;

typedef struct sysbvm_heap_relocationRecord_s
{
    uintptr_t sourceStartAddress;
    uintptr_t sourceEndAddress;
    uintptr_t destinationAddress;
} sysbvm_heap_relocationRecord_t;

typedef struct sysbvm_heap_relocationTable_s
{
    size_t entryCount;
    sysbvm_heap_relocationRecord_t *entries;
} sysbvm_heap_relocationTable_t;

typedef struct sysbvm_heap_chunkRecord_s
{
    uintptr_t address;
    uint32_t capacity;
    uint32_t size;
} sysbvm_heap_chunkRecord_t;

void sysbvm_heap_initialize(sysbvm_heap_t *heap);
void sysbvm_heap_destroy(sysbvm_heap_t *heap);

sysbvm_tuple_t *sysbvm_heap_allocateGCRootTableEntry(sysbvm_heap_t *heap);

void *sysbvm_heap_allocateAndLockCodeZone(sysbvm_heap_t *heap, size_t size, size_t alignment);
void sysbvm_heap_lockCodeZone(sysbvm_heap_t *heap, void *codePointer, size_t size);
void sysbvm_heap_unlockCodeZone(sysbvm_heap_t *heap, void *codePointer, size_t size);

void sysbvm_heap_computeCompactionForwardingPointers(sysbvm_heap_t *heap);
void sysbvm_heap_applyForwardingPointers(sysbvm_heap_t *heap);
void sysbvm_heap_compact(sysbvm_heap_t *heap);
void sysbvm_heap_swapGCColors(sysbvm_heap_t *heap);

void sysbvm_heap_dumpToFile(sysbvm_heap_t *heap, FILE *file);
bool sysbvm_heap_loadFromFile(sysbvm_heap_t *heap, FILE *file, size_t numberOfRootsToRelocate, sysbvm_tuple_t *rootsToRelocate);

void sysbvm_heapIterator_begin(sysbvm_heap_t *heap, sysbvm_heapIterator_t *iterator);
void sysbvm_heapIterator_beginWithPointer(sysbvm_heap_t *heap, sysbvm_tuple_t pointer, sysbvm_heapIterator_t *iterator);
bool sysbvm_heapIterator_isAtEnd(sysbvm_heapIterator_t *iterator);

sysbvm_object_tuple_t *sysbvm_heapIterator_get(sysbvm_heapIterator_t *iterator);
void sysbvm_heapIterator_advance(sysbvm_heapIterator_t *iterator);
bool sysbvm_heapIterator_advanceUntilInstanceWithType(sysbvm_heapIterator_t *iterator, sysbvm_tuple_t expectedType);
void sysbvm_heapIterator_compactionAdvance(sysbvm_heapIterator_t *iterator, size_t sizeToAdvance, sysbvm_object_tuple_t **outObjectPointer, bool commitNewSize);
void sysbvm_heapIterator_compactionFinish(sysbvm_heapIterator_t *iterator, bool commitNewSize);

#endif //SYSBVM_INTERNAL_HEAP_H
