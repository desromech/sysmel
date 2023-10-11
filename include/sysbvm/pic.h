#ifndef SYSBVM_PIC_H
#define SYSBVM_PIC_H

#pragma once

#include "tuple.h"
#include <stdatomic.h>

#define SYSBVM_PIC_ENTRY_COUNT 4

typedef struct sysbvm_picEntry_s
{
    sysbvm_tuple_t selector;
    sysbvm_tuple_t type;
    sysbvm_tuple_t method;
} sysbvm_picEntry_t;

typedef struct sysbvm_pic_s
{
    atomic_uint preSequence;
    atomic_uint postSequence;
    sysbvm_picEntry_t entries[SYSBVM_PIC_ENTRY_COUNT];
} sysbvm_pic_t;

SYSBVM_API bool sysbvm_pic_lookupTypeAndSelector(sysbvm_pic_t *pic, sysbvm_tuple_t selector, sysbvm_tuple_t type, sysbvm_tuple_t *outMethod);
SYSBVM_API void sysbvm_pic_addSelectorTypeAndMethod(sysbvm_pic_t *pic, sysbvm_tuple_t selector, sysbvm_tuple_t type, sysbvm_tuple_t method);
SYSBVM_API void sysbvm_pic_flushSelector(sysbvm_pic_t *pic, sysbvm_tuple_t selector);
SYSBVM_API unsigned int sysbvm_pic_writeLock(sysbvm_pic_t *pic);
SYSBVM_API void sysbvm_pic_writeUnlock(sysbvm_pic_t *pic, unsigned int sequence);

#endif //SYSBVM_PIC_H
