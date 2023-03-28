#ifndef TUUVM_BYTECODE_COMPILER_H
#define TUUVM_BYTECODE_COMPILER_H

#pragma once

#include "bytecode.h"

typedef struct tuuvm_bytecodeCompilerInstructionOperand_s
{
    tuuvm_tuple_header_t header;
} tuuvm_bytecodeCompilerInstructionOperand_t;

typedef struct tuuvm_bytecodeCompilerInstruction_s
{
    tuuvm_bytecodeCompilerInstructionOperand_t super;

    struct tuuvm_bytecodeCompilerInstruction_s *previous;
    struct tuuvm_bytecodeCompilerInstruction_s *next;

    tuuvm_tuple_t pc;
    tuuvm_tuple_t opcode;
    tuuvm_tuple_t operands;
} tuuvm_bytecodeCompilerInstruction_t;

typedef struct tuuvm_bytecodeCompilerInstructionVectorOperand_s
{
    tuuvm_bytecodeCompilerInstructionOperand_t super;
    tuuvm_tuple_t index;
    tuuvm_tuple_t vectorType;

    tuuvm_tuple_t hasAllocaDestination;
    tuuvm_tuple_t hasNonAllocaDestination;
    tuuvm_tuple_t hasLoadStoreUsage;
    tuuvm_tuple_t hasNonLoadStoreUsage;
} tuuvm_bytecodeCompilerInstructionVectorOperand_t;

typedef struct tuuvm_bytecodeCompiler_s
{
    tuuvm_tuple_header_t header;

    tuuvm_bytecodeCompilerInstruction_t *firstInstruction;
    tuuvm_bytecodeCompilerInstruction_t *lastInstruction;
} tuuvm_bytecodeCompiler_t;

/**
 * Creates an instance of the bytecode compiler.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_create(tuuvm_context_t *context);

/**
 * Creates a bytecode instruction.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompilerInstruction_create(tuuvm_context_t *context, uint8_t opcode, tuuvm_tuple_t operands);

/**
 * Creates a bytecode instruction.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompilerInstruction_createLabel(tuuvm_context_t *context);

/**
 * Creates an instruction to the bytecode compiler.
 */
TUUVM_API void tuuvm_bytecodeCompiler_addInstruction(tuuvm_tuple_t compiler, tuuvm_tuple_t instruction);

#endif //TUUVM_BYTECODE_COMPILER_H
