#ifndef SYSBVM_ELF_H
#define SYSBVM_ELF_H

#include <stdint.h>

typedef uint64_t sysbvm_elf64_addr_t;
typedef uint64_t sysbvm_elf64_off_t;
typedef uint16_t sysbvm_elf64_half_t;
typedef uint32_t sysbvm_elf64_word_t;
typedef int32_t sysbvm_elf64_sword_t;
typedef uint64_t sysbvm_elf64_xword_t;
typedef int64_t sysbvm_elf64_xsword_t;

enum {
    SYSBVM_EI_MAG0 = 0,
    SYSBVM_EI_MAG1 = 1,
    SYSBVM_EI_MAG2 = 2,
    SYSBVM_EI_MAG3 = 3,
    SYSBVM_EI_CLASS = 4,
    SYSBVM_EI_DATA = 5,
    SYSBVM_EI_VERSION = 6,
    SYSBVM_EI_OSABI = 7,
    SYSBVM_EI_ABIVERSION = 8,
    SYSBVM_EI_PAD = 9,
    SYSBVM_EI_NIDENT = 16,
};

enum {
    SYSBVM_ELFCLASS32 = 1,
    SYSBVM_ELFCLASS64 = 1,
};

enum {
    SYSBVM_ELFDATA2LSB = 1,
    SYSBVM_ELFDATA2MSB = 2,
};

enum {
    SYSBVM_ET_NONE = 0,
    SYSBVM_ET_REL = 1,
    SYSBVM_ET_EXEC = 2,
    SYSBVM_ET_DYN = 3,
    SYSBVM_ET_CORE = 4,
};

enum {
    SYSBVM_SHN_UNDEF = 0,
    SYSBVM_SHN_ABS = 0xFFF1,
    SYSBVM_SHN_COMMON = 0xFFF2,
};

enum {
    SYSBVM_SHT_NULL = 0,
    SYSBVM_SHT_PROGBITS = 1,
    SYSBVM_SHT_SYMTAB = 2,
    SYSBVM_SHT_STRTAB = 3,
    SYSBVM_SHT_RELA = 4,
    SYSBVM_SHT_HASH = 5,
    SYSBVM_SHT_DYNAMIC = 6,
    SYSBVM_SHT_NOTE = 7,
    SYSBVM_SHT_NOBITS = 8,
    SYSBVM_SHT_REL = 9,
    SYSBVM_SHT_SHLIB = 10,
    SYSBVM_SHT_DYNSYM = 11,
};

enum {
    SYSBVM_STB_LOCAL = 0,
    SYSBVM_STB_GLOBAL = 1,
    SYSBVM_STB_WEAK = 2,
};

enum {
    SYSBVM_STT_NOTYPE = 0,
    SYSBVM_STT_OBJECT = 1,
    SYSBVM_STT_FUNC = 2,
    SYSBVM_STT_SECTION = 3,
};

typedef struct sysbvm_elf64_header_s
{
    uint8_t ident[16];
    sysbvm_elf64_half_t type;
    sysbvm_elf64_half_t machine;
    sysbvm_elf64_word_t version;
    sysbvm_elf64_addr_t entry;
    sysbvm_elf64_off_t programHeadersOffset;
    sysbvm_elf64_off_t sectionHeadersOffset;
    sysbvm_elf64_word_t flags;
    sysbvm_elf64_half_t elfHeaderSize;
    sysbvm_elf64_half_t programHeaderEntrySize;
    sysbvm_elf64_half_t programHeaderCount;
    sysbvm_elf64_half_t sectionHeaderEntrySize;
    sysbvm_elf64_half_t sectionHeaderNum;
    sysbvm_elf64_half_t sectionHeaderNameStringTableIndex;
} sysbvm_elf64_header_t;

typedef struct sysbvm_elf64_sectionHeader_s
{
    sysbvm_elf64_word_t name;
    sysbvm_elf64_word_t type;
    sysbvm_elf64_xword_t flags;
    sysbvm_elf64_addr_t address;
    sysbvm_elf64_off_t offset;
    sysbvm_elf64_xword_t size;
    sysbvm_elf64_word_t link;
    sysbvm_elf64_word_t info;
    sysbvm_elf64_xword_t addressAlignment;
    sysbvm_elf64_xword_t entrySize;
} sysbvm_elf64_sectionHeader_t;

typedef struct sysbvm_elf64_symbol_s
{
    sysbvm_elf64_word_t name;
    uint8_t info;
    uint8_t other;
    sysbvm_elf64_half_t sectionHeaderIndex;
    sysbvm_elf64_addr_t value;
    sysbvm_elf64_xword_t size;
} sysbvm_elf64_symbol_t;

#endif //SYSBVM_ELF_H
