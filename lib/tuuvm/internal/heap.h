#ifndef TUUVM_INTERNAL_HEAP_H
#define TUUVM_INTERNAL_HEAP_H

#pragma once

#include "tuuvm/heap.h"

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
};

void tuuvm_heap_destroy(tuuvm_heap_t *heap);

#endif //TUUVM_INTERNAL_HEAP_H
