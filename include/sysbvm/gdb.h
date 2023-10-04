#ifndef SYSBVM_GDB_H
#define SYSBVM_GDB_H

#include "common.h"
#include <stdint.h>
#include <stddef.h>

typedef enum
{
  SYSBVM_GDB_JIT_NOACTION = 0,
  SYSBVM_GDB_JIT_REGISTER_FN,
  SYSBVM_GDB_JIT_UNREGISTER_FN
} sysbvm_gdb_jit_actions_t;

typedef struct sysbvm_gdb_jit_code_entry_s
{
  struct sysbvm_gdb_jit_code_entry_s *next_entry;
  struct sysbvm_gdb_jit_code_entry_s *prev_entry;
  const char *symfile_addr;
  uint64_t symfile_size;
} sysbvm_gdb_jit_code_entry_t;

typedef struct sysbvm_gdb_jit_descriptor_s
{
  uint32_t version;
  /* This type should be jit_actions_t, but we use uint32_t
     to be explicit about the bitwidth.  */
  uint32_t action_flag;
  sysbvm_gdb_jit_code_entry_t *relevant_entry;
  sysbvm_gdb_jit_code_entry_t *first_entry;
} sysbvm_gdb_jit_descriptor_t;

SYSBVM_API void sysbvm_gdb_registerObjectFile(sysbvm_gdb_jit_code_entry_t *entry, const void *objectFileAddress, size_t objectFileSize);
SYSBVM_API void sysbvm_gdb_unregisterObjectFile(sysbvm_gdb_jit_code_entry_t *entry);

#endif //SYSBVM_GDB_H
