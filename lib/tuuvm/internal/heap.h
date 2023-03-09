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

void tuuvm_heap_initialize(tuuvm_heap_t *heap);
void tuuvm_heap_destroy(tuuvm_heap_t *heap);

void tuuvm_heap_computeCompactionForwardingPointers(tuuvm_heap_t *heap);
void tuuvm_heap_applyForwardingPointers(tuuvm_heap_t *heap);
void tuuvm_heap_compact(tuuvm_heap_t *heap);
void tuuvm_heap_swapGCColors(tuuvm_heap_t *heap);

void tuuvm_heap_dumpToFile(tuuvm_heap_t *heap, FILE *file);

#endif //TUUVM_INTERNAL_HEAP_H
