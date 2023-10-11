#include "internal/virtualMemory.h"
#include <stdint.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

void *sysbvm_virtualMemory_allocateSystemMemory(size_t sizeToAllocate)
{
    return VirtualAlloc(NULL, sizeToAllocate, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

void *sysbvm_virtualMemory_allocateSystemMemoryForCode(size_t sizeToAllocate)
{
    return VirtualAlloc(NULL, sizeToAllocate, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READ);
}

void sysbvm_virtualMemory_freeSystemMemory(void *memory, size_t sizeToFree)
{
    (void)sizeToFree;
    VirtualFree(memory, 0, MEM_RELEASE);
}

size_t sysbvm_virtualMemory_getSystemAllocationAlignment(void)
{
    SYSTEM_INFO systemInfo;
    memset(&systemInfo, 0, sizeof(systemInfo));
    GetSystemInfo(&systemInfo);
    return systemInfo.dwPageSize;
}

void sysbvm_virtualMemory_lockCodePagesForWriting(void *codePointer, size_t size)
{
    size_t pageAlignment = sysbvm_virtualMemory_getSystemAllocationAlignment();
    uintptr_t startAddress = (uintptr_t)codePointer & (-pageAlignment);
    uintptr_t endAddress = ((uintptr_t)codePointer + size + pageAlignment - 1) & (-pageAlignment);

    DWORD oldProtection = 0;
    VirtualProtect((void*)startAddress, endAddress - startAddress, PAGE_READWRITE, &oldProtection);
}

void sysbvm_virtualMemory_unlockCodePagesForExecution(void *codePointer, size_t size)
{
    size_t pageAlignment = sysbvm_virtualMemory_getSystemAllocationAlignment();
    uintptr_t startAddress = (uintptr_t)codePointer & (-pageAlignment);
    uintptr_t endAddress = ((uintptr_t)codePointer + size + pageAlignment - 1) & (-pageAlignment);

    DWORD oldProtection = 0;
    VirtualProtect((void*)startAddress, endAddress - startAddress, PAGE_EXECUTE_READ, &oldProtection);
}

#else

#include <sys/mman.h>
#include <unistd.h>

void *sysbvm_virtualMemory_allocateSystemMemory(size_t sizeToAllocate)
{
    void *result = mmap(0, sizeToAllocate, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if(result == MAP_FAILED)
        return 0;

    return result;
}

void *sysbvm_virtualMemory_allocateSystemMemoryForCode(size_t sizeToAllocate)
{
    void *result = mmap(0, sizeToAllocate, PROT_READ | PROT_EXEC, MAP_PRIVATE | MAP_ANON, -1, 0);
    if(result == MAP_FAILED)
        return 0;

    return result;
}

void sysbvm_virtualMemory_freeSystemMemory(void *memory, size_t sizeToFree)
{
    munmap(memory, sizeToFree);
}

size_t sysbvm_virtualMemory_getSystemAllocationAlignment(void)
{
    return getpagesize();
}

void sysbvm_virtualMemory_lockCodePagesForWriting(void *codePointer, size_t size)
{
    size_t pageAlignment = sysbvm_virtualMemory_getSystemAllocationAlignment();
    uintptr_t startAddress = (uintptr_t)codePointer & (-pageAlignment);
    uintptr_t endAddress = ((uintptr_t)codePointer + size + pageAlignment - 1) & (-pageAlignment);

    mprotect((void*)startAddress, endAddress - startAddress, PROT_READ | PROT_WRITE);
}

void sysbvm_virtualMemory_unlockCodePagesForExecution(void *codePointer, size_t size)
{
    size_t pageAlignment = sysbvm_virtualMemory_getSystemAllocationAlignment();
    uintptr_t startAddress = (uintptr_t)codePointer & (-pageAlignment);
    uintptr_t endAddress = ((uintptr_t)codePointer + size + pageAlignment - 1) & (-pageAlignment);

    mprotect((void*)startAddress, endAddress - startAddress, PROT_READ | PROT_EXEC);
}

#endif