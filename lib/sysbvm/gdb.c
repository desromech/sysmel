#include "sysbvm/gdb.h"
#include <stdio.h>
#include <string.h>

#ifdef _MSC_VER
#define SYSBVM_NOINLINE __declspec(noinline)
#else
#define SYSBVM_NOINLINE __attribute__((noinline))
#endif

void SYSBVM_NOINLINE __jit_debug_register_code() {
#ifndef _MSC_VER
    asm volatile("" ::: "memory");
#endif
};

SYSBVM_API sysbvm_gdb_jit_descriptor_t __jit_debug_descriptor = { 1, 0, 0, 0 };

SYSBVM_API void sysbvm_gdb_registerObjectFile(sysbvm_gdb_jit_code_entry_t *entry, const void *objectFileAddress, size_t objectFileSize)
{
    memset(entry, 0, sizeof(sysbvm_gdb_jit_code_entry_t ));
    entry->symfile_addr = (char*)objectFileAddress;
    entry->symfile_size = objectFileSize;

    entry->next_entry = __jit_debug_descriptor.first_entry;
    if(entry->next_entry)
        entry->next_entry = entry->next_entry->prev_entry;
    __jit_debug_descriptor.relevant_entry = entry;
    __jit_debug_descriptor.action_flag = SYSBVM_GDB_JIT_REGISTER_FN;
    __jit_debug_register_code();
}

SYSBVM_API void sysbvm_gdb_unregisterObjectFile(sysbvm_gdb_jit_code_entry_t *entry)
{
    if(entry->prev_entry)
        entry->prev_entry->next_entry = entry->next_entry;
    else
        __jit_debug_descriptor.first_entry = entry->next_entry;

    if(entry->next_entry)
        entry->next_entry->prev_entry = entry->prev_entry;

    entry->prev_entry = entry->next_entry = NULL;
    
    __jit_debug_descriptor.relevant_entry = entry;
    __jit_debug_descriptor.action_flag = SYSBVM_GDB_JIT_UNREGISTER_FN;
    __jit_debug_register_code();
}
