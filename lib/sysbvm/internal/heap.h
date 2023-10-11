#ifndef SYSBVM_INTERNAL_HEAP_H
#define SYSBVM_INTERNAL_HEAP_H

#pragma once

#include "sysbvm/heap.h"
#include "sysbvm/chunkedAllocator.h"
#include <stdio.h>

typedef struct sysbvm_heap_mallocObjectHeader_s
{
    union
    {
        struct
        {
            struct sysbvm_heap_mallocObjectHeader_s *next;
            uint32_t size;
        };

        uint32_t words[4];
    };
} sysbvm_heap_mallocObjectHeader_t;

struct sysbvm_heap_s
{
    sysbvm_heap_mallocObjectHeader_t *firstMallocObject;
    sysbvm_heap_mallocObjectHeader_t *lastMallocObject;

    bool shouldAttemptToCollect;

    size_t totalSize;
    size_t totalCapacity;
    size_t nextGCSizeThreshold;

    uint32_t gcWhiteColor;
    uint32_t gcGrayColor;
    uint32_t gcBlackColor;
    
    sysbvm_chunkedAllocator_t gcRootTableAllocator;
    sysbvm_chunkedAllocator_t picTableAllocator;

    size_t codeZoneCapacity;
    size_t codeZoneSize;
    uint8_t *codeZone;
};

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

void sysbvm_heap_replaceWeakReferencesWithTombstones(sysbvm_heap_t *heap);
void sysbvm_heap_sweep(sysbvm_heap_t *heap);
void sysbvm_heap_swapGCColors(sysbvm_heap_t *heap);

#endif //SYSBVM_INTERNAL_HEAP_H
