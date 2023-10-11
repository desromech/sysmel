#ifndef SYSBVM_INTERNAL_VIRTUAL_MEMORY_H
#define SYSBVM_INTERNAL_VIRTUAL_MEMORY_H

#pragma once

#include "sysbvm/common.h"
#include <stddef.h>

void *sysbvm_virtualMemory_allocateSystemMemory(size_t sizeToAllocate);
void *sysbvm_virtualMemory_allocateSystemMemoryForCode(size_t sizeToAllocate);
void sysbvm_virtualMemory_freeSystemMemory(void *memory, size_t sizeToFree);
size_t sysbvm_virtualMemory_getSystemAllocationAlignment(void);
void sysbvm_virtualMemory_lockCodePagesForWriting(void *codePointer, size_t size);
void sysbvm_virtualMemory_unlockCodePagesForExecution(void *codePointer, size_t size);

#endif //SYSBVM_INTERNAL_VIRTUAL_MEMORY_H
