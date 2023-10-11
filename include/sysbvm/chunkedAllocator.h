#ifndef SYSBVM_CHUNKED_ALLOCATOR_H
#define SYSBVM_CHUNKED_ALLOCATOR_H

#include "common.h"
#include <stddef.h>
#include <stdbool.h>

#define SYSBVM_CHUNKED_ALLOCATOR_DEFAULT_CHUNK_SIZE ((size_t)(2<<20))

typedef struct sysbvm_chunkedAllocatorChunk_s
{
    struct sysbvm_chunkedAllocatorChunk_s *previous;
    struct sysbvm_chunkedAllocatorChunk_s *next;
    struct sysbvm_chunkedAllocatorChunk_s *writeableMapping;
    struct sysbvm_chunkedAllocatorChunk_s *executableMapping;
    size_t capacity;
    size_t size;
} sysbvm_chunkedAllocatorChunk_t;

typedef struct sysbvm_chunkedAllocator_s
{
    sysbvm_chunkedAllocatorChunk_t* firstChunk;
    sysbvm_chunkedAllocatorChunk_t* lastChunk;
    sysbvm_chunkedAllocatorChunk_t* currentChunk;
    size_t chunkSize;
    bool requiresExecutableMapping;
} sysbvm_chunkedAllocator_t;

SYSBVM_API void sysbvm_chunkedAllocator_initialize(sysbvm_chunkedAllocator_t *allocator, size_t chunkSize, bool requiresExecutableMapping);
SYSBVM_API void sysbvm_chunkedAllocator_destroy(sysbvm_chunkedAllocator_t *allocator);

SYSBVM_API void* sysbvm_chunkedAllocator_allocate(sysbvm_chunkedAllocator_t *allocator, size_t size, size_t alignment);
SYSBVM_API void sysbvm_chunkedAllocator_allocateWithDualMapping(sysbvm_chunkedAllocator_t *allocator, size_t size, size_t alignment, void **writeableMapping, void **executableMapping);

#endif //SYSBVM_CHUNKED_ALLOCATOR_H
