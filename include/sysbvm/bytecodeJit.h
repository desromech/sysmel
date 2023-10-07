#ifndef SYSBVM_BYTECODE_JIT_H
#define SYSBVM_BYTECODE_JIT_H

#pragma once

#include "bytecode.h"
#include "dwarf.h"

#if defined(__x86_64__) || defined(_M_X64)
#   define SYSBVM_ARCH_X86_64 1
#endif

#if defined(__aarch64__)
#   define SYSBVM_ARCH_AARCH64 1
#endif

#if defined(SYSBVM_ARCH_X86_64)
#   define SYSBVM_JIT_SUPPORTED
#endif

#ifdef SYSBVM_JIT_SUPPORTED

#include "dynarray.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 
#define NOMINMAX
#include <windows.h>
#endif

#define SYSBVM_JIT_DWARF_LINE_INFO_EMISSION_STATE_MAX_DIRECTORIES 8
#define SYSBVM_JIT_DWARF_LINE_INFO_EMISSION_STATE_MAX_FILES 8

typedef sysbvm_tuple_t (*sysbvm_bytecodeJit_entryPoint) (sysbvm_context_t *context, sysbvm_tuple_t function, size_t argumentCount, sysbvm_tuple_t *arguments);

typedef enum sysbvm_bytecodeJitRelocationType_e
{
    SYSBVM_BYTECODE_JIT_RELOCATION_RELATIVE32,
    SYSBVM_BYTECODE_JIT_RELOCATION_RELATIVE64,
} sysbvm_bytecodeJitRelocationType_t;

typedef struct sysbvm_bytecodeJitRelocation_s
{
    size_t offset;
    sysbvm_bytecodeJitRelocationType_t type;
    intptr_t value;
    intptr_t addend;
} sysbvm_bytecodeJitRelocation_t;

typedef struct sysbvm_bytecodeJitPCRelocation_s
{
    size_t offset;
    size_t targetPC;
    intptr_t addend;
} sysbvm_bytecodeJitPCRelocation_t;

typedef struct sysbvm_bytecodeJitSourcePositionRecord_s
{
    size_t pc;
    sysbvm_tuple_t sourcePosition;
    uint32_t line;
    uint32_t column;
} sysbvm_bytecodeJitSourcePositionRecord_t;

typedef struct sysbvm_jit_dwarfLineInfoEmissionState_s
{
    int directoryCount;
    sysbvm_tuple_t directories[SYSBVM_JIT_DWARF_LINE_INFO_EMISSION_STATE_MAX_DIRECTORIES];

    int fileCount;
    sysbvm_tuple_t files[SYSBVM_JIT_DWARF_LINE_INFO_EMISSION_STATE_MAX_DIRECTORIES];

    int32_t previousLine;
    int32_t minLineAdvance;
    int32_t maxLineAdvance;
    int32_t lineBase;
    int32_t lineRange;

    sysbvm_tuple_t currentFile;
    size_t pc;
} sysbvm_jit_dwarfLineInfoEmissionState_t;

typedef struct sysbvm_bytecodeJit_s
{
    sysbvm_context_t *context;
    sysbvm_tuple_t sourcePosition;
    sysbvm_tuple_t compiledProgramEntity;

    size_t argumentCount;
    size_t captureVectorSize;
    size_t literalCount;
    size_t localVectorSize;

    int32_t contextPointerOffset;
    int32_t localVectorOffset;
    int32_t argumentVectorOffset;
    int32_t literalVectorOffset;
    int32_t captureVectorOffset;
    int32_t pcOffset;
    int32_t stackFrameRecordOffset;
    int32_t callArgumentVectorSizeOffset;
    int32_t callArgumentVectorOffset;
    int32_t stackFrameSize;
    int32_t stackCallReservationSize;
    int32_t cfiFrameOffset;

    sysbvm_dynarray_t objectFileHeader;
    sysbvm_dynarray_t instructions;
    sysbvm_dynarray_t constants;
    sysbvm_dynarray_t relocations;
    sysbvm_dynarray_t pcRelocations;
    sysbvm_dynarray_t sourcePositions;
    sysbvm_dynarray_t unwindInfo;
    sysbvm_dynarray_t unwindInfoBytecode;
    sysbvm_dwarf_cfi_builder_t dwarfEhBuilder;
    sysbvm_dwarf_debugInfo_builder_t dwarfDebugInfoBuilder;
    sysbvm_dynarray_t objectFileContent;
    size_t objectFileContentJittedFunctionNameOffset;
    size_t prologueSize;

    sysbvm_jit_dwarfLineInfoEmissionState_t dwarfLineEmissionState;

    intptr_t *pcDestinations;

    sysbvm_tuple_t *literalVectorGCRoot;
} sysbvm_bytecodeJit_t;

static inline size_t sysbvm_sizeAlignedTo(size_t pointer, size_t alignment)
{
    return (pointer + alignment - 1) & (~(alignment - 1));
}

SYSBVM_API int sysbvm_jit_dwarfLineInfoEmissionState_indexOfDirectory(sysbvm_jit_dwarfLineInfoEmissionState_t *state, sysbvm_tuple_t directory);
SYSBVM_API int sysbvm_jit_dwarfLineInfoEmissionState_indexOfFile(sysbvm_jit_dwarfLineInfoEmissionState_t *state, sysbvm_tuple_t file);

SYSBVM_API void sysbvm_bytecodeJit_initialize(sysbvm_bytecodeJit_t *jit, sysbvm_context_t *context);
SYSBVM_API size_t sysbvm_bytecodeJit_addBytes(sysbvm_bytecodeJit_t *jit, size_t byteCount, uint8_t *bytes);
SYSBVM_API size_t sysbvm_bytecodeJit_addByte(sysbvm_bytecodeJit_t *jit, uint8_t byte);
SYSBVM_API size_t sysbvm_bytecodeJit_addConstantsBytes(sysbvm_bytecodeJit_t *jit, size_t byteCount, uint8_t *bytes);
SYSBVM_API size_t sysbvm_bytecodeJit_addUnwindInfoBytes(sysbvm_bytecodeJit_t *jit, size_t byteCount, uint8_t *bytes);
SYSBVM_API size_t sysbvm_bytecodeJit_addUnwindInfoByte(sysbvm_bytecodeJit_t *jit, uint8_t byte);

#ifdef _WIN32
SYSBVM_API void sysbvm_bytecodeJit_uwop(sysbvm_bytecodeJit_t *jit, uint8_t opcode, uint8_t operationInfo);
SYSBVM_API void sysbvm_bytecodeJit_uwop_pushNonVol(sysbvm_bytecodeJit_t *jit, uint8_t reg);
SYSBVM_API void sysbvm_bytecodeJit_uwop_setFPReg(sysbvm_bytecodeJit_t *jit);
SYSBVM_API void sysbvm_bytecodeJit_uwop_alloc(sysbvm_bytecodeJit_t *jit, size_t amount);
#endif

SYSBVM_API void sysbvm_bytecodeJit_addPCRelocation(sysbvm_bytecodeJit_t *jit, sysbvm_bytecodeJitPCRelocation_t relocation);
SYSBVM_API void sysbvm_bytecodeJit_addRelocation(sysbvm_bytecodeJit_t *jit, sysbvm_bytecodeJitRelocation_t relocation);
SYSBVM_API void sysbvm_bytecodeJit_jitFree(sysbvm_bytecodeJit_t *jit);
SYSBVM_API bool sysbvm_bytecodeJit_getLiteralValueForOperand(sysbvm_bytecodeJit_t *jit, int16_t operand, sysbvm_tuple_t *outLiteralValue);

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeJit_slotAt(sysbvm_context_t *context, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot);
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeJit_slotReferenceAt(sysbvm_context_t *context, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot);
SYSBVM_API void sysbvm_bytecodeJit_slotAtPut(sysbvm_context_t *context, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot, sysbvm_tuple_t value);
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeJit_refSlotAt(sysbvm_context_t *context, sysbvm_tuple_t tupleReference, sysbvm_tuple_t typeSlot);
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeJit_refSlotReferenceAt(sysbvm_context_t *context, sysbvm_tuple_t tupleReference, sysbvm_tuple_t typeSlot);
SYSBVM_API void sysbvm_bytecodeJit_refSlotAtPut(sysbvm_context_t *context, sysbvm_tuple_t tupleReference, sysbvm_tuple_t typeSlot, sysbvm_tuple_t value);
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeJit_symbolValueBinding_getValue(sysbvm_context_t *context, sysbvm_tuple_t valueBinding);

SYSBVM_API void sysbvm_bytecodeJit_jit(sysbvm_context_t *context, sysbvm_functionBytecode_t *functionBytecode);

// Backend specific methods.
SYSBVM_API void sysbvm_jit_prologue(sysbvm_bytecodeJit_t *jit);
SYSBVM_API bool sysbvm_jit_emitDebugLineInfo(sysbvm_bytecodeJit_t *jit);
SYSBVM_API void sysbvm_jit_finish(sysbvm_bytecodeJit_t *jit);
SYSBVM_API uint8_t *sysbvm_jit_installIn(sysbvm_bytecodeJit_t *jit, uint8_t *codeZonePointer);

SYSBVM_API void sysbvm_jit_breakpoint(sysbvm_bytecodeJit_t *jit);
SYSBVM_API void sysbvm_jit_unreachable(sysbvm_bytecodeJit_t *jit);

SYSBVM_API void sysbvm_jit_callNoResult0(sysbvm_bytecodeJit_t *jit, void *functionPointer);
SYSBVM_API void sysbvm_jit_callWithContextNoResult0(sysbvm_bytecodeJit_t *jit, void *functionPointer);
SYSBVM_API void sysbvm_jit_callWithContext1(sysbvm_bytecodeJit_t *jit, void *functionPointer, int16_t resultOperand, int16_t argumentOperand0);
SYSBVM_API void sysbvm_jit_callWithContext2(sysbvm_bytecodeJit_t *jit, void *functionPointer, int16_t resultOperand, int16_t argumentOperand0, int16_t argumentOperand1);
SYSBVM_API void sysbvm_jit_callWithContextNoResult2(sysbvm_bytecodeJit_t *jit, void *functionPointer, int16_t argumentOperand0, int16_t argumentOperand1);
SYSBVM_API void sysbvm_jit_callWithContextNoResult3(sysbvm_bytecodeJit_t *jit, void *functionPointer, int16_t argumentOperand0, int16_t argumentOperand1, int16_t argumentOperand2);

SYSBVM_API void sysbvm_jit_patchTrampolineWithRealEntryPoint(sysbvm_bytecodeJit_t *jit, sysbvm_functionBytecode_t *bytecode);
SYSBVM_API void sysbvm_jit_functionApply(sysbvm_bytecodeJit_t *jit, int16_t resultOperand, int16_t functionOperand, size_t argumentCount, int16_t *argumentOperands, int applicationFlags);
SYSBVM_API void sysbvm_jit_send(sysbvm_bytecodeJit_t *jit, int16_t resultOperand, int16_t selectorOperand, size_t argumentCount, int16_t *argumentOperands, int applicationFlags);
SYSBVM_API void sysbvm_jit_sendWithReceiverType(sysbvm_bytecodeJit_t *jit, int16_t resultOperand, int16_t receiverTypeOperand, int16_t selectorOperand, size_t argumentCount, int16_t *argumentOperands, int applicationFlags);
SYSBVM_API void sysbvm_jit_makeArray(sysbvm_bytecodeJit_t *jit, int16_t resultOperand, size_t elementCount, int16_t *elementOperands);
SYSBVM_API void sysbvm_jit_makeByteArray(sysbvm_bytecodeJit_t *jit, int16_t resultOperand, size_t elementCount, int16_t *elementOperands);
SYSBVM_API void sysbvm_jit_makeDictionary(sysbvm_bytecodeJit_t *jit, int16_t resultOperand, size_t elementCount, int16_t *elementOperands);
SYSBVM_API void sysbvm_jit_makeClosureWithCaptures(sysbvm_bytecodeJit_t *jit, int16_t resultOperand, int16_t functionDefinitionOperand, size_t captureCount, int16_t *elementOperands);
SYSBVM_API void sysbvm_jit_jumpRelative(sysbvm_bytecodeJit_t *jit, size_t targetPC);
SYSBVM_API void sysbvm_jit_jumpRelativeIfTrue(sysbvm_bytecodeJit_t *jit, int16_t conditionOperand, size_t targetPC);
SYSBVM_API void sysbvm_jit_jumpRelativeIfFalse(sysbvm_bytecodeJit_t *jit, int16_t conditionOperand, size_t targetPC);

SYSBVM_API void sysbvm_jit_moveOperandToOperand(sysbvm_bytecodeJit_t *jit, int16_t destinationOperand, int16_t sourceOperand);
SYSBVM_API void sysbvm_jit_return(sysbvm_bytecodeJit_t *jit, int16_t operand);
SYSBVM_API void sysbvm_jit_storePC(sysbvm_bytecodeJit_t *jit, uint16_t pc);

#endif //SYSBVM_JIT_SUPPORTED
#endif //SYSBVM_BYTECODE_JIT_H
