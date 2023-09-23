#ifndef SYSBVM_ORDERED_OFFSET_TABLE_H
#define SYSBVM_ORDERED_OFFSET_TABLE_H

#include "tuple.h"

typedef struct sysbvm_orderedOffsetTable_s
{
    sysbvm_tuple_header_t header;
    sysbvm_tuple_t keys;
    sysbvm_tuple_t values;
} sysbvm_orderedOffsetTable_t;

typedef struct sysbvm_orderedOffsetTableBuilder_s
{
    sysbvm_tuple_header_t header;
    sysbvm_tuple_t keys;
    sysbvm_tuple_t values;
} sysbvm_orderedOffsetTableBuilder_t;

SYSBVM_API sysbvm_tuple_t sysbvm_orderedOffsetTableBuilder_create(sysbvm_context_t *context);

SYSBVM_API void sysbvm_orderedOffsetTableBuilder_withOffsetAddValue(sysbvm_context_t *context, sysbvm_tuple_t builder, uint32_t offset, sysbvm_tuple_t value);

SYSBVM_API sysbvm_tuple_t sysbvm_orderedOffsetTableBuilder_finish(sysbvm_context_t *context, sysbvm_tuple_t builder);
SYSBVM_API sysbvm_tuple_t sysbvm_orderedOffsetTable_findValueWithOffset(sysbvm_context_t *context, sysbvm_tuple_t table, uint32_t offset);

#endif //SYSBVM_ORDERED_OFFSET_TABLE_H