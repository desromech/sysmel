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
};

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

void tuuvm_heap_computeCompactionForwardingPointers(tuuvm_heap_t *heap);
void tuuvm_heap_applyForwardingPointers(tuuvm_heap_t *heap);
void tuuvm_heap_compact(tuuvm_heap_t *heap);
void tuuvm_heap_swapGCColors(tuuvm_heap_t *heap);

void tuuvm_heap_dumpToFile(tuuvm_heap_t *heap, FILE *file);
void tuuvm_heap_loadFromFile(tuuvm_heap_t *heap, FILE *file, size_t numberOfRootsToRelocate, tuuvm_tuple_t *rootsToRelocate);

#endif //TUUVM_INTERNAL_HEAP_H
