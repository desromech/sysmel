#include "sysbvm/dwarf.h"
#include "sysbvm/assert.h"
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
    int32_t pcRelativeValue = (int32_t)(buffer->size - value);
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

SYSBVM_API size_t sysbvm_dwarf_encodeQWord(sysbvm_dynarray_t *buffer, uint64_t value)
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

SYSBVM_API size_t sysbvm_dwarf_encodeStringTupleWithDefaultString(sysbvm_dynarray_t *buffer, sysbvm_tuple_t stringTuple, const char *defaultString)
{
    size_t offset = buffer->size;
    if(sysbvm_tuple_isBytes(stringTuple) && sysbvm_tuple_getSizeInBytes(stringTuple) > 0)
    {
        sysbvm_dynarray_addAll(buffer, sysbvm_tuple_getSizeInBytes(stringTuple), SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(stringTuple)->bytes);
        sysbvm_dwarf_encodeByte(buffer, 0);
    }
    else
    {
        sysbvm_dwarf_encodeCString(buffer, defaultString);
    }

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
    cfi->cie = *cie;
    sysbvm_dwarf_encodeByte(&cfi->buffer, cfi->version);
    sysbvm_dwarf_encodeCString(&cfi->buffer, cfi->isEhFrame ? "zR" : ""); // Argumentation
    if(!cfi->isEhFrame)
    {
        sysbvm_dwarf_encodeByte(&cfi->buffer, sizeof(uintptr_t)); // Address size
        sysbvm_dwarf_encodeByte(&cfi->buffer, 0); // Segment size
    }
    sysbvm_dwarf_encodeULEB128(&cfi->buffer, cie->codeAlignmentFactor);
    sysbvm_dwarf_encodeSLEB128(&cfi->buffer, cie->dataAlignmentFactor);
    if(cfi->version <= 2 && !cfi->isEhFrame)
        sysbvm_dwarf_encodeByte(&cfi->buffer, (uint8_t)cie->returnAddressRegister);
    else
        sysbvm_dwarf_encodeULEB128(&cfi->buffer, cie->returnAddressRegister);
    if(cfi->isEhFrame)
    {
        sysbvm_dwarf_encodeULEB128(&cfi->buffer, 1);
        sysbvm_dwarf_encodeByte(&cfi->buffer, DW_EH_PE_pcrel | DW_EH_PE_sdata4);
    }
}

SYSBVM_API void sysbvm_dwarf_cfi_endCIE(sysbvm_dwarf_cfi_builder_t *cfi)
{
    sysbvm_dwarf_encodeAlignment(&cfi->buffer, sizeof(uintptr_t));
    uint32_t cieSize = (uint32_t)(cfi->buffer.size - cfi->cieContentOffset);
    memcpy(cfi->buffer.data + cfi->cieOffset, &cieSize, 4);
}

SYSBVM_API void sysbvm_dwarf_cfi_beginFDE(sysbvm_dwarf_cfi_builder_t *cfi, size_t pc)
{
    cfi->fdeOffset = sysbvm_dwarf_encodeDWord(&cfi->buffer, 0);
    cfi->fdeContentOffset = sysbvm_dwarf_encodeDwarfPointerPCRelative(&cfi->buffer, (uint32_t)cfi->cieOffset);
    cfi->fdeInitialPC = pc;
    if(cfi->isEhFrame)
    {
        cfi->fdeInitialLocationOffset = sysbvm_dwarf_encodeDWord(&cfi->buffer, 0);
        cfi->fdeAddressingRangeOffset = sysbvm_dwarf_encodeDWord(&cfi->buffer, 0);
        sysbvm_dwarf_encodeULEB128(&cfi->buffer, 0);
    }
    else
    {
        cfi->fdeInitialLocationOffset = sysbvm_dwarf_encodePointer(&cfi->buffer, 0);
        cfi->fdeAddressingRangeOffset = sysbvm_dwarf_encodePointer(&cfi->buffer, 0);
    }
    cfi->currentPC = cfi->fdeInitialPC;
    cfi->stackFrameSize = cfi->initialStackFrameSize;
    cfi->framePointerRegister = 0;
    cfi->hasFramePointerRegister = false;
    cfi->isInPrologue = true;
}

SYSBVM_API void sysbvm_dwarf_cfi_endFDE(sysbvm_dwarf_cfi_builder_t *cfi, size_t pc)
{
    sysbvm_dwarf_encodeAlignment(&cfi->buffer, sizeof(uintptr_t));
    if(cfi->isEhFrame)
    {
        uint32_t pcRange = (uint32_t)(pc - cfi->fdeInitialPC);
        memcpy(cfi->buffer.data + cfi->fdeAddressingRangeOffset, &pcRange, sizeof(uint32_t));
    }
    else
    {
        uintptr_t pcRange = pc - cfi->fdeInitialPC;
        memcpy(cfi->buffer.data + cfi->fdeAddressingRangeOffset, &pcRange, sizeof(uintptr_t));
    }

    uint32_t fdeSize = (uint32_t)(cfi->buffer.size - cfi->fdeContentOffset);
    memcpy(cfi->buffer.data + cfi->fdeOffset, &fdeSize, 4);
}

SYSBVM_API void sysbvm_dwarf_cfi_finish(sysbvm_dwarf_cfi_builder_t *cfi)
{
    sysbvm_dwarf_encodeDWord(&cfi->buffer, 0);
}

SYSBVM_API void sysbvm_dwarf_cfi_setPC(sysbvm_dwarf_cfi_builder_t *cfi, size_t pc)
{
    size_t advance = pc - cfi->currentPC;
    if(advance)
    {
        size_t advanceFactor = advance / cfi->cie.codeAlignmentFactor;
        if(advanceFactor <= 63)
        {
            sysbvm_dwarf_encodeByte(&cfi->buffer, (DW_OP_CFA_advance_loc << 6) | (uint8_t)advanceFactor);
        }
        else
        {
            if(advanceFactor <= 0xFF)
            {
                sysbvm_dwarf_encodeByte(&cfi->buffer, DW_OP_CFA_advance_loc1);
                sysbvm_dwarf_encodeByte(&cfi->buffer, (uint8_t)advanceFactor);
            }
            else if(advanceFactor <= 0xFFFF)
            {
                sysbvm_dwarf_encodeByte(&cfi->buffer, DW_OP_CFA_advance_loc2);
                sysbvm_dwarf_encodeWord(&cfi->buffer, (uint16_t)advanceFactor);
            }
            else
            {
                SYSBVM_ASSERT(advanceFactor <= 0xFFFFFFFF);
                sysbvm_dwarf_encodeByte(&cfi->buffer, DW_OP_CFA_advance_loc4);
                sysbvm_dwarf_encodeDWord(&cfi->buffer, (uint32_t)advanceFactor);
            }
        }
    }

    cfi->currentPC = pc;
}

SYSBVM_API void sysbvm_dwarf_cfi_cfaInRegisterWithOffset(sysbvm_dwarf_cfi_builder_t *cfi, uintptr_t reg, intptr_t offset)
{
    sysbvm_dwarf_encodeByte(&cfi->buffer, DW_OP_CFA_def_cfa);
    sysbvm_dwarf_encodeULEB128(&cfi->buffer, reg);
    sysbvm_dwarf_encodeULEB128(&cfi->buffer, offset);
}

SYSBVM_API void sysbvm_dwarf_cfi_cfaInRegisterWithFactoredOffset(sysbvm_dwarf_cfi_builder_t *cfi, uintptr_t reg, size_t offset)
{
    sysbvm_dwarf_cfi_cfaInRegisterWithOffset(cfi, reg, sizeof(uintptr_t) * offset);
}

SYSBVM_API void sysbvm_dwarf_cfi_registerValueAtFactoredOffset(sysbvm_dwarf_cfi_builder_t *cfi, uintptr_t reg, size_t offset)
{
    if(reg <= 63) {
        sysbvm_dwarf_encodeByte(&cfi->buffer, (DW_OP_CFA_offset << 6) | (uint8_t)reg);
        sysbvm_dwarf_encodeULEB128(&cfi->buffer, offset);
    } else {
        sysbvm_dwarf_encodeByte(&cfi->buffer, DW_OP_CFA_offset_extended);
        sysbvm_dwarf_encodeULEB128(&cfi->buffer, reg);
        sysbvm_dwarf_encodeULEB128(&cfi->buffer, offset);
    }
}

SYSBVM_API void sysbvm_dwarf_cfi_pushRegister(sysbvm_dwarf_cfi_builder_t *cfi, uintptr_t reg)
{
    ++cfi->stackFrameSize;
    if(!cfi->hasFramePointerRegister)
        sysbvm_dwarf_cfi_cfaInRegisterWithFactoredOffset(cfi, cfi->stackPointerRegister, cfi->stackFrameSize);
    sysbvm_dwarf_cfi_registerValueAtFactoredOffset(cfi, reg, cfi->stackFrameSize);
}

SYSBVM_API void sysbvm_dwarf_cfi_saveFramePointerInRegister(sysbvm_dwarf_cfi_builder_t *cfi, uintptr_t reg, intptr_t offset)
{
    SYSBVM_ASSERT(!cfi->hasFramePointerRegister);
    SYSBVM_ASSERT((offset % sizeof(uintptr_t)) == 0);

    cfi->hasFramePointerRegister = true;
    cfi->framePointerRegister = reg;
    cfi->stackFrameSizeAtFramePointer = cfi->stackFrameSize - offset / sizeof(uintptr_t);
    sysbvm_dwarf_cfi_cfaInRegisterWithFactoredOffset(cfi, reg, cfi->stackFrameSizeAtFramePointer);
}

SYSBVM_API void sysbvm_dwarf_cfi_stackSizeAdvance(sysbvm_dwarf_cfi_builder_t *cfi, size_t pc, size_t increment)
{
    if(!cfi->isInPrologue) return;
    if(!increment) return;
    
    cfi->stackFrameSize += increment / sizeof(uintptr_t);
    if(!cfi->hasFramePointerRegister)
    {
        sysbvm_dwarf_cfi_setPC(cfi, pc);
        sysbvm_dwarf_cfi_cfaInRegisterWithFactoredOffset(cfi, cfi->stackPointerRegister, cfi->stackFrameSizeAtFramePointer);
    }
}

SYSBVM_API void sysbvm_dwarf_cfi_endPrologue(sysbvm_dwarf_cfi_builder_t *cfi)
{
    SYSBVM_ASSERT(cfi->isInPrologue);
    cfi->isInPrologue = false;
}

SYSBVM_API void sysbvm_dwarf_debugInfo_create(sysbvm_dwarf_debugInfo_builder_t *builder)
{
    memset(builder, 0, sizeof(sysbvm_dwarf_debugInfo_builder_t));
    builder->version = 4;
    builder->lineProgramHeader.minimumInstructionLength = 1;
    builder->lineProgramHeader.maximumOperationsPerInstruction = 1;
    builder->lineProgramHeader.opcodeBase = 13;
    builder->lineProgramHeader.defaultIsStatement = true;

    sysbvm_dynarray_initialize(&builder->locationExpression, 1, 256);

    sysbvm_dynarray_initialize(&builder->line, 1, 1024);
    sysbvm_dynarray_initialize(&builder->str, 1, 1024);
    sysbvm_dynarray_initialize(&builder->abbrev, 1, 1024);
    sysbvm_dynarray_initialize(&builder->info, 1, 1024);
    sysbvm_dynarray_initialize(&builder->lineTextAddresses, sizeof(uint32_t), 1024);
    sysbvm_dynarray_initialize(&builder->infoTextAddresses, sizeof(uint32_t), 32);

    // Null string.
    sysbvm_dwarf_encodeByte(&builder->str, 0);

    // Info header
    sysbvm_dwarf_encodeDwarfPointer(&builder->info, 0);
    sysbvm_dwarf_encodeWord(&builder->info, builder->version);
    sysbvm_dwarf_encodeDwarfPointer(&builder->info, 0); // Debug abbrev offset
    sysbvm_dwarf_encodeByte(&builder->info, sizeof(uintptr_t)); // Address size.
}

SYSBVM_API void sysbvm_dwarf_debugInfo_destroy(sysbvm_dwarf_debugInfo_builder_t *builder)
{
    sysbvm_dynarray_destroy(&builder->locationExpression);
    sysbvm_dynarray_destroy(&builder->line);
    sysbvm_dynarray_destroy(&builder->str);
    sysbvm_dynarray_destroy(&builder->abbrev);
    sysbvm_dynarray_destroy(&builder->info);
    sysbvm_dynarray_destroy(&builder->lineTextAddresses);
    sysbvm_dynarray_destroy(&builder->infoTextAddresses);
}

SYSBVM_API void sysbvm_dwarf_debugInfo_finish(sysbvm_dwarf_debugInfo_builder_t *builder)
{
    // End the abbreviations.
    sysbvm_dwarf_encodeByte(&builder->abbrev, 0);

    // Info initial length.
    {
        uint32_t infoInitialLength = (uint32_t)(builder->info.size - 4);
        memcpy(builder->info.data, &infoInitialLength, 4);
    }
}

SYSBVM_API void sysbvm_dwarf_debugInfo_patchTextAddressesRelativeTo(sysbvm_dwarf_debugInfo_builder_t *builder, uintptr_t baseAddress)
{
    uint32_t *lineOffsets = (uint32_t*)builder->lineTextAddresses.data;
    for(size_t i = 0; i < builder->lineTextAddresses.size; ++i)
    {
        uintptr_t *address = (uintptr_t *)(builder->line.data + lineOffsets[i]);
        *address += baseAddress;
    }

    uint32_t *infoOffsets = (uint32_t*)builder->infoTextAddresses.data;
    for(size_t i = 0; i < builder->infoTextAddresses.size; ++i)
    {
        uintptr_t *address = (uintptr_t *)(builder->info.data + infoOffsets[i]);
        *address += baseAddress;
    }
}

SYSBVM_API void sysbvm_dwarf_debugInfo_beginLineInformation(sysbvm_dwarf_debugInfo_builder_t *builder)
{
    sysbvm_dwarf_encodeDWord(&builder->line, 0);
    sysbvm_dwarf_encodeWord(&builder->line, builder->version);
    builder->lineHeaderLengthOffset = (uint32_t)sysbvm_dwarf_encodeDWord(&builder->line, 0); // Header length
    sysbvm_dwarf_encodeByte(&builder->line, builder->lineProgramHeader.minimumInstructionLength);
    sysbvm_dwarf_encodeByte(&builder->line, builder->lineProgramHeader.maximumOperationsPerInstruction);
    sysbvm_dwarf_encodeByte(&builder->line, builder->lineProgramHeader.defaultIsStatement);
    sysbvm_dwarf_encodeByte(&builder->line, builder->lineProgramHeader.lineBase);
    sysbvm_dwarf_encodeByte(&builder->line, builder->lineProgramHeader.lineRange);
    sysbvm_dwarf_encodeByte(&builder->line, builder->lineProgramHeader.opcodeBase);

    sysbvm_dwarf_encodeByte(&builder->line, 0); // DW_LNS_copy
    sysbvm_dwarf_encodeByte(&builder->line, 1); // DW_LNS_advance_pc
    sysbvm_dwarf_encodeByte(&builder->line, 1); // DW_LNS_advance_line
    sysbvm_dwarf_encodeByte(&builder->line, 1); // DW_LNS_set_file
    sysbvm_dwarf_encodeByte(&builder->line, 1); // DW_LNS_set_column
    sysbvm_dwarf_encodeByte(&builder->line, 0); // DW_LNS_negate_stmt
    sysbvm_dwarf_encodeByte(&builder->line, 0); // DW_LNS_set_basic_block
    sysbvm_dwarf_encodeByte(&builder->line, 0); // DW_LNS_const_add_pc
    sysbvm_dwarf_encodeByte(&builder->line, 1); // DW_LNS_fixed_advance_pc
    sysbvm_dwarf_encodeByte(&builder->line, 0); // DW_LNS_set_prologue_end
    sysbvm_dwarf_encodeByte(&builder->line, 0); // DW_LNS_set_epilogue_begin
    sysbvm_dwarf_encodeByte(&builder->line, 1); // DW_LNS_set_isa

    builder->lineProgramState.regAddress = 0;
    builder->lineProgramState.regOpIndex = 0;
    builder->lineProgramState.regFile = 1;
    builder->lineProgramState.regLine = 1;
    builder->lineProgramState.regColumn = 0;
    builder->lineProgramState.regIsStatement = builder->lineProgramHeader.defaultIsStatement;
    builder->lineProgramState.regBasicBlock = false;
    builder->lineProgramState.regEndSequence = false;
    builder->lineProgramState.regPrologueEnd = false;
    builder->lineProgramState.regEpilogueBegin = false;
    builder->lineProgramState.regISA = 0;
    builder->lineProgramState.regDiscriminator = false;
}

SYSBVM_API void sysbvm_dwarf_debugInfo_addDirectory(sysbvm_dwarf_debugInfo_builder_t *builder, sysbvm_tuple_t directoryName)
{
    sysbvm_dwarf_encodeStringTupleWithDefaultString(&builder->line, directoryName, ".");
}

SYSBVM_API void sysbvm_dwarf_debugInfo_endDirectoryList(sysbvm_dwarf_debugInfo_builder_t *builder)
{
    sysbvm_dwarf_encodeByte(&builder->line, 0);
}

SYSBVM_API void sysbvm_dwarf_debugInfo_addFile(sysbvm_dwarf_debugInfo_builder_t *builder, int directoryIndex, sysbvm_tuple_t name)
{
    sysbvm_dwarf_encodeStringTupleWithDefaultString(&builder->line, name, "<unknown>");
    sysbvm_dwarf_encodeULEB128(&builder->line, directoryIndex);
    sysbvm_dwarf_encodeULEB128(&builder->line, 0); // Last modification time.
    sysbvm_dwarf_encodeULEB128(&builder->line, 0); // Size in bytes.
}

SYSBVM_API void sysbvm_dwarf_debugInfo_endFileList(sysbvm_dwarf_debugInfo_builder_t *builder)
{
    sysbvm_dwarf_encodeByte(&builder->line, 0);
}

SYSBVM_API void sysbvm_dwarf_debugInfo_endLineInformationHeader(sysbvm_dwarf_debugInfo_builder_t *builder)
{
    uint32_t headerSize = (uint32_t)(builder->line.size - builder->lineHeaderLengthOffset - 4);
    memcpy(builder->line.data + builder->lineHeaderLengthOffset, &headerSize, 4);
}

SYSBVM_API void sysbvm_dwarf_debugInfo_line_setAddress(sysbvm_dwarf_debugInfo_builder_t *builder, uintptr_t value)
{
    sysbvm_dwarf_encodeByte(&builder->line, 0);
    sysbvm_dwarf_encodeULEB128(&builder->line, 1 + sizeof(value));
    sysbvm_dwarf_encodeByte(&builder->line, DW_LNE_set_address);
    uint32_t addressOffset = (uint32_t)sysbvm_dwarf_encodePointer(&builder->line, value);
    sysbvm_dynarray_add(&builder->lineTextAddresses, &addressOffset);
    builder->lineProgramState.regAddress = (uint32_t)value;
}

SYSBVM_API void sysbvm_dwarf_debugInfo_line_setFile(sysbvm_dwarf_debugInfo_builder_t *builder, uint32_t file)
{
    if(builder->lineProgramState.regFile == file)
        return;

    sysbvm_dwarf_encodeByte(&builder->line, DW_LNS_set_file);
    sysbvm_dwarf_encodeULEB128(&builder->line, file);
    builder->lineProgramState.regFile = file;
}

SYSBVM_API void sysbvm_dwarf_debugInfo_line_setColumn(sysbvm_dwarf_debugInfo_builder_t *builder, int column)
{
    if(builder->lineProgramState.regColumn == column)
        return;

    sysbvm_dwarf_encodeByte(&builder->line, DW_LNS_set_column);
    sysbvm_dwarf_encodeULEB128(&builder->line, column);
    builder->lineProgramState.regColumn = column;
}

SYSBVM_API void sysbvm_dwarf_debugInfo_line_advanceLine(sysbvm_dwarf_debugInfo_builder_t *builder, int deltaLine)
{
    if(deltaLine == 0)
        return;

    sysbvm_dwarf_encodeByte(&builder->line, DW_LNS_advance_line);
    sysbvm_dwarf_encodeSLEB128(&builder->line, deltaLine);
    builder->lineProgramState.regLine += deltaLine;
}

SYSBVM_API void sysbvm_dwarf_debugInfo_line_advancePC(sysbvm_dwarf_debugInfo_builder_t *builder, int deltaPC)
{
    if(deltaPC == 0)
        return;

    sysbvm_dwarf_encodeByte(&builder->line, DW_LNS_advance_pc);
    sysbvm_dwarf_encodeULEB128(&builder->line, deltaPC);
    builder->lineProgramState.regAddress += deltaPC;
}

SYSBVM_API void sysbvm_dwarf_debugInfo_line_copyRow(sysbvm_dwarf_debugInfo_builder_t *builder)
{
    sysbvm_dwarf_encodeByte(&builder->line, DW_LNS_copy);
    builder->lineProgramState.regDiscriminator = false;
    builder->lineProgramState.regBasicBlock = false;
    builder->lineProgramState.regPrologueEnd = false;
    builder->lineProgramState.regEpilogueBegin = false;
}

SYSBVM_API void sysbvm_dwarf_debugInfo_line_advanceLineAndPC(sysbvm_dwarf_debugInfo_builder_t *builder, int deltaLine, int deltaPC)
{
    int operationAdvance = deltaPC / builder->lineProgramHeader.minimumInstructionLength;

    int opcode = (deltaLine - builder->lineProgramHeader.lineBase) + (builder->lineProgramHeader.lineRange * operationAdvance) + builder->lineProgramHeader.opcodeBase;
    if( (0 <= opcode) && (opcode <= 255) 
        && (deltaLine - builder->lineProgramHeader.lineBase < builder->lineProgramHeader.lineRange)
        && (deltaLine >= builder->lineProgramHeader.lineBase) )
    {
        sysbvm_dwarf_encodeByte(&builder->line, opcode);
        builder->lineProgramState.regLine += deltaLine;
        builder->lineProgramState.regAddress += deltaPC;
    }
    else
    {
        sysbvm_dwarf_debugInfo_line_advanceLine(builder, deltaLine);
        sysbvm_dwarf_debugInfo_line_advancePC(builder, deltaPC);
        sysbvm_dwarf_debugInfo_line_copyRow(builder);
    }
}

SYSBVM_API void sysbvm_dwarf_debugInfo_line_endSequence(sysbvm_dwarf_debugInfo_builder_t *builder)
{
    sysbvm_dwarf_encodeByte(&builder->line, 0);
    sysbvm_dwarf_encodeULEB128(&builder->line, 1);
    sysbvm_dwarf_encodeByte(&builder->line, DW_LNE_end_sequence);
}

SYSBVM_API void sysbvm_dwarf_debugInfo_endLineInformation(sysbvm_dwarf_debugInfo_builder_t *builder)
{
    uint32_t lineInfoSize = (uint32_t)(builder->line.size - 4);
    memcpy(builder->line.data, &lineInfoSize, 4);
}

SYSBVM_API size_t sysbvm_dwarf_debugInfo_beginDIE(sysbvm_dwarf_debugInfo_builder_t *builder, uintptr_t tag, bool hasChildren)
{
    int abbreviationCode = ++builder->abbreviationCount;
    sysbvm_dwarf_encodeULEB128(&builder->abbrev, abbreviationCode);
    sysbvm_dwarf_encodeULEB128(&builder->abbrev, tag);
    sysbvm_dwarf_encodeByte(&builder->abbrev, hasChildren ? DW_CHILDREN_yes : DW_CHILDREN_no);

    return sysbvm_dwarf_encodeULEB128(&builder->info, abbreviationCode);
}

SYSBVM_API void sysbvm_dwarf_debugInfo_endDIE(sysbvm_dwarf_debugInfo_builder_t *builder)
{
    sysbvm_dwarf_encodeByte(&builder->abbrev, 0);
    sysbvm_dwarf_encodeByte(&builder->abbrev, 0);
}

SYSBVM_API void sysbvm_dwarf_debugInfo_endDIEChildren(sysbvm_dwarf_debugInfo_builder_t *builder)
{
    sysbvm_dwarf_encodeULEB128(&builder->info, 0);
}

SYSBVM_API void sysbvm_dwarf_debugInfo_attribute_uleb128(sysbvm_dwarf_debugInfo_builder_t *builder, uintptr_t attribute, uintptr_t value)
{
    sysbvm_dwarf_encodeULEB128(&builder->abbrev, attribute);
    sysbvm_dwarf_encodeULEB128(&builder->abbrev, DW_FORM_udata);

    sysbvm_dwarf_encodeULEB128(&builder->info, value);
}

SYSBVM_API void sysbvm_dwarf_debugInfo_attribute_secOffset(sysbvm_dwarf_debugInfo_builder_t *builder, uintptr_t attribute, uintptr_t value)
{
    sysbvm_dwarf_encodeULEB128(&builder->abbrev, attribute);
    sysbvm_dwarf_encodeULEB128(&builder->abbrev, DW_FORM_sec_offset);

    sysbvm_dwarf_encodeDWord(&builder->info, (uint32_t)value);
}

SYSBVM_API void sysbvm_dwarf_debugInfo_attribute_string(sysbvm_dwarf_debugInfo_builder_t *builder, uintptr_t attribute, const char *value)
{
    sysbvm_dwarf_encodeULEB128(&builder->abbrev, attribute);
    sysbvm_dwarf_encodeULEB128(&builder->abbrev, DW_FORM_strp);

    size_t stringOffset = sysbvm_dwarf_encodeCString(&builder->str, value);
    sysbvm_dwarf_encodeDWord(&builder->info, (uint32_t)stringOffset);
}

SYSBVM_API void sysbvm_dwarf_debugInfo_attribute_stringTupleWithDefaultString(sysbvm_dwarf_debugInfo_builder_t *builder, uintptr_t attribute, sysbvm_tuple_t value, const char *defaultString)
{
    sysbvm_dwarf_encodeULEB128(&builder->abbrev, attribute);
    sysbvm_dwarf_encodeULEB128(&builder->abbrev, DW_FORM_strp);

    size_t stringOffset = sysbvm_dwarf_encodeStringTupleWithDefaultString(&builder->str, value, defaultString);
    sysbvm_dwarf_encodeDWord(&builder->info, (uint32_t)stringOffset);
}

SYSBVM_API void sysbvm_dwarf_debugInfo_attribute_ref1(sysbvm_dwarf_debugInfo_builder_t *builder, uintptr_t attribute, uint8_t value)
{
    sysbvm_dwarf_encodeULEB128(&builder->abbrev, attribute);
    sysbvm_dwarf_encodeULEB128(&builder->abbrev, DW_FORM_ref1);
    sysbvm_dwarf_encodeByte(&builder->info, value);
}

SYSBVM_API void sysbvm_dwarf_debugInfo_attribute_textAddress(sysbvm_dwarf_debugInfo_builder_t *builder, uintptr_t attribute, uintptr_t value)
{
    sysbvm_dwarf_encodeULEB128(&builder->abbrev, attribute);
    sysbvm_dwarf_encodeULEB128(&builder->abbrev, DW_FORM_addr);

    uint32_t addressOffset = (uint32_t)sysbvm_dwarf_encodePointer(&builder->info, value);
    sysbvm_dynarray_add(&builder->infoTextAddresses, &addressOffset);
}

SYSBVM_API void sysbvm_dwarf_debugInfo_attribute_beginLocationExpression(sysbvm_dwarf_debugInfo_builder_t *builder, uintptr_t attribute)
{
    sysbvm_dwarf_encodeULEB128(&builder->abbrev, attribute);
    sysbvm_dwarf_encodeULEB128(&builder->abbrev, DW_FORM_exprloc);
}

SYSBVM_API void sysbvm_dwarf_debugInfo_attribute_endLocationExpression(sysbvm_dwarf_debugInfo_builder_t *builder)
{
    sysbvm_dwarf_encodeULEB128(&builder->info, builder->locationExpression.size);
    sysbvm_dynarray_addAll(&builder->info, builder->locationExpression.size, builder->locationExpression.data);
    builder->locationExpression.size = 0;
}

SYSBVM_API void sysbvm_dwarf_debugInfo_location_constUnsigned(sysbvm_dwarf_debugInfo_builder_t *builder, uintptr_t constant)
{
    if(constant <= 31)
    {
        sysbvm_dwarf_encodeByte(&builder->locationExpression, DW_OP_lit0 + (uint8_t)constant);
        return;
    }

    if(constant <= 0xFF)
    {
        sysbvm_dwarf_encodeByte(&builder->locationExpression, DW_OP_const1u);
        sysbvm_dwarf_encodeByte(&builder->locationExpression, (uint8_t)constant);
        return;
    }

    if(constant <= 0xFFFF)
    {
        sysbvm_dwarf_encodeByte(&builder->locationExpression, DW_OP_const2u);
        sysbvm_dwarf_encodeWord(&builder->locationExpression, (uint16_t)constant);
        return;
    }

    if(constant <= 0xFFFFFFFF)
    {
        sysbvm_dwarf_encodeByte(&builder->locationExpression, DW_OP_const4u);
        sysbvm_dwarf_encodeDWord(&builder->locationExpression, (uint32_t)constant);
        return;
    }

    sysbvm_dwarf_encodeByte(&builder->locationExpression, DW_OP_const8u);
    sysbvm_dwarf_encodeQWord(&builder->locationExpression, (uint64_t)constant);
}

SYSBVM_API void sysbvm_dwarf_debugInfo_location_deref(sysbvm_dwarf_debugInfo_builder_t *builder)
{
    sysbvm_dwarf_encodeByte(&builder->locationExpression, DW_OP_deref);
}

SYSBVM_API void sysbvm_dwarf_debugInfo_location_frameBaseOffset(sysbvm_dwarf_debugInfo_builder_t *builder, intptr_t offset)
{
    sysbvm_dwarf_encodeByte(&builder->locationExpression, DW_OP_fbreg);
    sysbvm_dwarf_encodeSLEB128(&builder->locationExpression, offset);
}

SYSBVM_API void sysbvm_dwarf_debugInfo_location_plus(sysbvm_dwarf_debugInfo_builder_t *builder)
{
    sysbvm_dwarf_encodeByte(&builder->locationExpression, DW_OP_plus);
}

SYSBVM_API void sysbvm_dwarf_debugInfo_location_register(sysbvm_dwarf_debugInfo_builder_t *builder, uintptr_t reg)
{
    if(reg <= 31)
    {
        sysbvm_dwarf_encodeByte(&builder->locationExpression, DW_OP_reg0 + (uint8_t)reg);
    }
    else
    {
        sysbvm_dwarf_encodeByte(&builder->locationExpression, DW_OP_regx);
        sysbvm_dwarf_encodeULEB128(&builder->locationExpression, reg);
    }
}
