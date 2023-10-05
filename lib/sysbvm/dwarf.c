#include "sysbvm/dwarf.h"
#include <stdbool.h>
#include <string.h>


SYSBVM_API size_t sysbvm_dwarf_encodeDwarfPointer(sysbvm_dynarray_t *buffer, uint32_t value)
{
    size_t offset = buffer->size;
    sysbvm_dynarray_addAll(buffer, sizeof(value), &value);
    return offset;
}

SYSBVM_API size_t sysbvm_dwarf_encodeDwarfPointerPCRelative(sysbvm_dynarray_t *buffer, uint32_t value)
{
    int32_t pcRelativeValue = buffer->size - value;
    size_t offset = buffer->size;
    sysbvm_dynarray_addAll(buffer, sizeof(pcRelativeValue), &pcRelativeValue);
    return offset;
}

SYSBVM_API size_t sysbvm_dwarf_encodePointer(sysbvm_dynarray_t *buffer, uintptr_t value)
{
    size_t offset = buffer->size;
    sysbvm_dynarray_addAll(buffer, sizeof(value), &value);
    return offset;
}

SYSBVM_API size_t sysbvm_dwarf_encodeByte(sysbvm_dynarray_t *buffer, uint8_t value)
{
    size_t offset = buffer->size;
    sysbvm_dynarray_addAll(buffer, sizeof(value), &value);
    return offset;
}

SYSBVM_API size_t sysbvm_dwarf_encodeWord(sysbvm_dynarray_t *buffer, uint16_t value)
{
    size_t offset = buffer->size;
    sysbvm_dynarray_addAll(buffer, sizeof(value), &value);
    return offset;
}

SYSBVM_API size_t sysbvm_dwarf_encodeDWord(sysbvm_dynarray_t *buffer, uint32_t value)
{
    size_t offset = buffer->size;
    sysbvm_dynarray_addAll(buffer, sizeof(value), &value);
    return offset;
}

SYSBVM_API size_t sysbvm_dwarf_encodeCString(sysbvm_dynarray_t *buffer, const char *cstring)
{
    size_t offset = buffer->size;
    sysbvm_dynarray_addAll(buffer, strlen(cstring) + 1, cstring);
    return offset;
}

SYSBVM_API size_t sysbvm_dwarf_encodeULEB128(sysbvm_dynarray_t *buffer, uintptr_t value)
{
    size_t offset = buffer->size;
    uintptr_t currentValue = value;
    do
    {
        uint8_t byte = currentValue & 127;
        currentValue >>= 7;

        if(currentValue)
            byte |= 128;
        sysbvm_dynarray_add(buffer, &byte);
    } while (currentValue != 0);
    return offset;
}

SYSBVM_API size_t sysbvm_dwarf_encodeSLEB128(sysbvm_dynarray_t *buffer, intptr_t value)
{
    size_t offset = buffer->size;
    bool more = true;

    intptr_t currentValue = value;
    while(more)
    {
        uint8_t byte = currentValue & 127;
        currentValue >>= 7;
        
        bool byteHasSign = byte & 0x40;
        if ((currentValue == 0 && !byteHasSign) || (currentValue == -1 && byteHasSign))
            more = false;
        else
            byte = byte | 0x80;

        sysbvm_dynarray_add(buffer, &byte);
    }
    return offset;
}

SYSBVM_API size_t sysbvm_dwarf_encodeAlignment(sysbvm_dynarray_t *buffer, size_t alignment)
{
    size_t offset = buffer->size;
    size_t alignedSize = (buffer->size + alignment - 1) & (-alignment);
    size_t padding = alignedSize - buffer->size;
    for(size_t i = 0; i < padding; ++i)
        sysbvm_dwarf_encodeByte(buffer, 0);
    return offset;
}

SYSBVM_API void sysbvm_dwarf_cfi_create(sysbvm_dwarf_cfi_builder_t *cfi)
{
    memset(cfi, 0, sizeof(sysbvm_dwarf_cfi_builder_t));
    cfi->version = 1;
    cfi->isEhFrame = true;
    sysbvm_dynarray_initialize(&cfi->buffer, 1, 1024);
}

SYSBVM_API void sysbvm_dwarf_cfi_destroy(sysbvm_dwarf_cfi_builder_t *cfi)
{
    sysbvm_dynarray_destroy(&cfi->buffer);
}

SYSBVM_API void sysbvm_dwarf_cfi_beginCIE(sysbvm_dwarf_cfi_builder_t *cfi, sysbvm_dwarf_cie_t *cie)
{
    cfi->cieOffset = sysbvm_dwarf_encodeDWord(&cfi->buffer, 0);
    cfi->cieContentOffset = sysbvm_dwarf_encodeDwarfPointer(&cfi->buffer, cfi->isEhFrame ? 0 : -1 ); // CIE_id
    sysbvm_dwarf_encodeByte(&cfi->buffer, cfi->version);
    sysbvm_dwarf_encodeCString(&cfi->buffer, ""); // Argumentation
    if(!cfi->isEhFrame)
    {
        sysbvm_dwarf_encodeByte(&cfi->buffer, sizeof(uintptr_t)); // Address size
        sysbvm_dwarf_encodeByte(&cfi->buffer, 0); // Segment size
    }
    sysbvm_dwarf_encodeULEB128(&cfi->buffer, cie->codeAlignmentFactor);
    sysbvm_dwarf_encodeSLEB128(&cfi->buffer, cie->dataAlignmentFactor);
    if(cfi->version <= 2 && !cfi->isEhFrame)
        sysbvm_dwarf_encodeByte(&cfi->buffer, cie->returnAddressRegister);
    else
        sysbvm_dwarf_encodeULEB128(&cfi->buffer, cie->returnAddressRegister);
}

SYSBVM_API void sysbvm_dwarf_cfi_endCIE(sysbvm_dwarf_cfi_builder_t *cfi)
{
    sysbvm_dwarf_encodeAlignment(&cfi->buffer, sizeof(uintptr_t));
    uint32_t cieSize = cfi->buffer.size - cfi->cieContentOffset;
    memcpy(cfi->buffer.data + cfi->cieOffset, &cieSize, 4);
}

SYSBVM_API void sysbvm_dwarf_cfi_beginFDE(sysbvm_dwarf_cfi_builder_t *cfi, size_t pc)
{
    cfi->fdeOffset = sysbvm_dwarf_encodeDWord(&cfi->buffer, 0);
    cfi->fdeContentOffset = sysbvm_dwarf_encodeDwarfPointerPCRelative(&cfi->buffer, cfi->cieOffset);
    cfi->fdeInitialPC = pc;
    cfi->fdeInitialLocationOffset = sysbvm_dwarf_encodePointer(&cfi->buffer, cfi->fdeInitialLocationOffset);
    cfi->fdeAddressingRangeOffset = sysbvm_dwarf_encodePointer(&cfi->buffer, 0);
}

SYSBVM_API void sysbvm_dwarf_cfi_endFDE(sysbvm_dwarf_cfi_builder_t *cfi, size_t pc)
{
    sysbvm_dwarf_encodeAlignment(&cfi->buffer, sizeof(uintptr_t));
    uintptr_t pcRange = pc - cfi->fdeInitialPC;
    memcpy(cfi->buffer.data + cfi->fdeAddressingRangeOffset, &pcRange, sizeof(uintptr_t));

    uint32_t fdeSize = cfi->buffer.size - cfi->fdeContentOffset;
    memcpy(cfi->buffer.data + cfi->fdeOffset, &fdeSize, 4);
}

SYSBVM_API void sysbvm_dwarf_cfi_finish(sysbvm_dwarf_cfi_builder_t *cfi)
{
    sysbvm_dwarf_encodeDWord(&cfi->buffer, 0);
}
