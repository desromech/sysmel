#include "sysbvm/orderedOffsetTable.h"
#include "sysbvm/array.h"
#include "sysbvm/orderedCollection.h"
#include "internal/context.h"

SYSBVM_API sysbvm_tuple_t sysbvm_orderedOffsetTableBuilder_create(sysbvm_context_t *context)
{
    sysbvm_orderedOffsetTableBuilder_t *result = (sysbvm_orderedOffsetTableBuilder_t*)sysbvm_context_allocatePointerTuple(context, context->roots.orderedOffsetTableBuilderType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_orderedOffsetTableBuilder_t));
    result->keys = sysbvm_orderedCollection_create(context);
    result->values = sysbvm_orderedCollection_create(context);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API void sysbvm_orderedOffsetTableBuilder_withOffsetAddValue(sysbvm_context_t *context, sysbvm_tuple_t builder, uint32_t offset, sysbvm_tuple_t value)
{
    sysbvm_orderedOffsetTableBuilder_t *builderObject = (sysbvm_orderedOffsetTableBuilder_t*)builder;
    size_t size = sysbvm_orderedCollection_getSize(builderObject->keys);
    if(size > 0)
    {
        sysbvm_tuple_t lastValue = sysbvm_orderedCollection_at(builderObject->values, size - 1);
        if(lastValue == value)
            return;
    }

    sysbvm_orderedCollection_add(context, builderObject->keys, sysbvm_tuple_uint32_encode(context, offset));
    sysbvm_orderedCollection_add(context, builderObject->values, value);
}

SYSBVM_API sysbvm_tuple_t sysbvm_orderedOffsetTableBuilder_finish(sysbvm_context_t *context, sysbvm_tuple_t builder)
{
    sysbvm_orderedOffsetTableBuilder_t *builderObject = (sysbvm_orderedOffsetTableBuilder_t*)builder;
    size_t size = sysbvm_orderedCollection_getSize(builderObject->keys);
    if(!size)
        return SYSBVM_NULL_TUPLE;

    sysbvm_orderedOffsetTable_t *result = (sysbvm_orderedOffsetTable_t*)sysbvm_context_allocatePointerTuple(context, context->roots.orderedOffsetTableType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_orderedOffsetTable_t));
    result->keys = sysbvm_orderedCollection_asWordArray(context, builderObject->keys);
    result->values = sysbvm_orderedCollection_asArray(context, builderObject->values);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_orderedOffsetTable_findValueWithOffset(sysbvm_context_t *context, sysbvm_tuple_t table, uint32_t offset)
{
    (void)context;
    if(!table)
        return SYSBVM_NULL_TUPLE;

    sysbvm_orderedOffsetTable_t *offsetTable = (sysbvm_orderedOffsetTable_t*)table;
    if(!offsetTable->keys)
        return SYSBVM_NULL_TUPLE;

    size_t size = sysbvm_tuple_getSizeInSlots(offsetTable->values);
    uint32_t *offsets = (uint32_t*)SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(offsetTable->keys)->bytes;

    // Binary search
    uint32_t lower = 0;
    uint32_t upper = size;
    intptr_t bestFound = -1;
    while(lower < upper)
    {
        uint32_t middle = lower + (upper - lower) / 2;
        uint32_t entryPC = offsets[middle];
        if(entryPC <= offset)
        {
            lower = middle + 1;
            bestFound = middle;
        }
        else
        {
            upper = middle;
        }
    }

    if(bestFound < 0 || (size_t)bestFound >= sysbvm_array_getSize(offsetTable->values))
        return SYSBVM_NULL_TUPLE;

    return SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(offsetTable->values)->pointers[bestFound];
}
