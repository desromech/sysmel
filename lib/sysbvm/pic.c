#include "sysbvm/pic.h"
#include <string.h>

SYSBVM_API bool sysbvm_pic_lookupTypeAndSelector(sysbvm_pic_t *pic, sysbvm_tuple_t selector, sysbvm_tuple_t type, sysbvm_tuple_t *outMethod)
{
    uint32_t entrySequence = atomic_load_explicit(&pic->preSequence, memory_order_acquire);

    bool hasFoundEntry = false;
    for(int i = 0; i < SYSBVM_PIC_ENTRY_COUNT; ++i)
    {
        sysbvm_picEntry_t *entry = pic->entries + i;
        if(entry->selector == selector && entry->type == type)
        {
            *outMethod = entry->method;
            hasFoundEntry = true;
        }
    }

    uint32_t exitSequence = atomic_load_explicit(&pic->postSequence, memory_order_acquire);
    return hasFoundEntry && entrySequence == exitSequence;
}

SYSBVM_API void sysbvm_pic_addSelectorTypeAndMethod(sysbvm_pic_t *pic, sysbvm_tuple_t selector, sysbvm_tuple_t type, sysbvm_tuple_t method)
{
    unsigned int sequence = sysbvm_pic_writeLock(pic);
    unsigned int entryIndex = sequence % SYSBVM_PIC_ENTRY_COUNT;
    sysbvm_picEntry_t *entry = pic->entries + entryIndex;
    entry->type = type;
    entry->selector = selector;
    entry->method = method;

    sysbvm_pic_writeUnlock(pic, sequence);
}

SYSBVM_API void sysbvm_pic_flushSelector(sysbvm_pic_t *pic, sysbvm_tuple_t selector)
{
    uint32_t sequence = sysbvm_pic_writeLock(pic);
    for(int i = 0; i < SYSBVM_PIC_ENTRY_COUNT; ++i)
    {
        sysbvm_picEntry_t *entry = pic->entries + i;
        if(entry->selector == selector)
            memset(entry, 0, sizeof(sysbvm_picEntry_t));
    }
    sysbvm_pic_writeUnlock(pic, sequence);
}

SYSBVM_API unsigned int sysbvm_pic_writeLock(sysbvm_pic_t *pic)
{
    atomic_uint entrySequence;
    atomic_uint entryNextSequence;
    do
    {
        entrySequence = pic->preSequence;
        entryNextSequence = (uint32_t)(entrySequence + 1);
    } while(!atomic_compare_exchange_weak_explicit(&pic->preSequence, &entrySequence, entryNextSequence, memory_order_release, memory_order_acquire));

    return entryNextSequence;
}

SYSBVM_API void sysbvm_pic_writeUnlock(sysbvm_pic_t *pic, unsigned int sequence)
{
    atomic_store_explicit(&pic->postSequence, sequence, memory_order_release);
}
