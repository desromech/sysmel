#ifndef SYSBVM_BYTECODE_COMPILER_H
#define SYSBVM_BYTECODE_COMPILER_H

#pragma once

#include "bytecode.h"

typedef struct sysbvm_functionDefinition_s sysbvm_functionDefinition_t;

typedef struct sysbvm_bytecodeCompilerInstructionOperand_s
{
    sysbvm_tuple_header_t header;
} sysbvm_bytecodeCompilerInstructionOperand_t;

typedef struct sysbvm_bytecodeCompilerInstruction_s
{
    sysbvm_bytecodeCompilerInstructionOperand_t super;

    struct sysbvm_bytecodeCompilerInstruction_s *previous;
    struct sysbvm_bytecodeCompilerInstruction_s *next;

    sysbvm_tuple_t pc;
    sysbvm_tuple_t endPC;
    sysbvm_tuple_t opcode;
    sysbvm_tuple_t operands;

    sysbvm_tuple_t sourcePosition;
    sysbvm_tuple_t sourceEnvironment;
    sysbvm_tuple_t sourceASTNode;
} sysbvm_bytecodeCompilerInstruction_t;

typedef struct sysbvm_bytecodeCompilerInstructionVectorOperand_s
{
    sysbvm_bytecodeCompilerInstructionOperand_t super;

    sysbvm_tuple_t index;
    sysbvm_tuple_t vectorType;

    sysbvm_tuple_t hasAllocaDestination;
    sysbvm_tuple_t hasNonAllocaDestination;
    sysbvm_tuple_t hasLoadStoreUsage;
    sysbvm_tuple_t hasNonLoadStoreUsage;
} sysbvm_bytecodeCompilerInstructionVectorOperand_t;

typedef struct sysbvm_bytecodeCompiler_s
{
    sysbvm_tuple_header_t header;

    sysbvm_tuple_t arguments;
    sysbvm_tuple_t captures;
    sysbvm_tuple_t literals;
    sysbvm_tuple_t literalDictionary;
    sysbvm_tuple_t temporaries;
    sysbvm_tuple_t usedTemporaryCount;

    sysbvm_bytecodeCompilerInstruction_t *firstInstruction;
    sysbvm_bytecodeCompilerInstruction_t *lastInstruction;

    sysbvm_tuple_t breakLabel;
    sysbvm_tuple_t continueLabel;

    sysbvm_tuple_t sourcePosition;
    sysbvm_tuple_t sourceEnvironment;
    sysbvm_tuple_t sourceASTNode;

    sysbvm_tuple_t bindingDictionary;
} sysbvm_bytecodeCompiler_t;

/**
 * Creates an instance of the bytecode compiler.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_create(sysbvm_context_t *context);

/**
 * Set the argument count.
 */
SYSBVM_API void sysbvm_bytecodeCompiler_setArgumentCount(sysbvm_context_t *context, sysbvm_tuple_t compiler, size_t argumentCount);

/**
 * Set the capture count.
 */
SYSBVM_API void sysbvm_bytecodeCompiler_setCaptureCount(sysbvm_context_t *context, sysbvm_tuple_t compiler, size_t argumentCount);

/**
 * Adds a literal
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_addLiteral(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t literalValue);

/**
 * Creates an instruction to the bytecode compiler.
 */
SYSBVM_API void sysbvm_bytecodeCompiler_addInstruction(sysbvm_tuple_t compiler, sysbvm_tuple_t instruction);

/**
 * Compiles the AST node with the bytecode compiler.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_compileASTNode(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t astNode);

/**
 * Compiles the AST node with the bytecode compiler and the given break and continue labels.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_compileASTNodeWithBreakAndContinue(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t astNode, sysbvm_tuple_t breakLabel, sysbvm_tuple_t continueLabel);

/**
 * Compiles the AST node with the bytecode compiler and the given environment.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_compileASTNodeWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t astNode, sysbvm_tuple_t environment);


/**
 * Compiles sets the value for the given binding.
 */
SYSBVM_API void sysbvm_bytecodeCompiler_setBindingValue(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t binding, sysbvm_tuple_t value);

/**
 * Compiles gets the value for the given binding.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_getBindingValue(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t binding);

/**
 * Compiles the given function definition.
 */
SYSBVM_API void sysbvm_bytecodeCompiler_compileFunctionDefinition(sysbvm_context_t *context, sysbvm_functionDefinition_t *definition);

/**
 * Makes a temporary.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_newTemporary(sysbvm_context_t *context, sysbvm_tuple_t compiler);

/**
 * Alloca with value.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_allocaWithValue(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t result, sysbvm_tuple_t pointerLikeType, sysbvm_tuple_t value);

/**
 * Call instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_call(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t result, sysbvm_tuple_t function, sysbvm_tuple_t arguments);

/**
 * Unchecked call instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_uncheckedCall(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t result, sysbvm_tuple_t function, sysbvm_tuple_t arguments);

/**
 * Coerce value.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_coerceValue(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t result, sysbvm_tuple_t type, sysbvm_tuple_t value);

/**
 * Jump instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_jump(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t destination);

/**
 * Jump if true instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_jumpIfTrue(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t condition, sysbvm_tuple_t destination);

/**
 * Jump if false instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_jumpIfFalse(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t condition, sysbvm_tuple_t destination);

/**
 * Load instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_load(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t destination, sysbvm_tuple_t pointer);

/**
 * Make array with elements.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_makeArrayWithElements(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t result, sysbvm_tuple_t elements);

/**
 * Make an association.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_makeAssociation(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t result, sysbvm_tuple_t key, sysbvm_tuple_t value);

/**
 * Make a byte array with elements.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_makeByteArrayWithElements(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t result, sysbvm_tuple_t elements);

/**
 * Make a dictionary with elements.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_makeDictionaryWithElements(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t result, sysbvm_tuple_t elements);

/**
 * Make closure with captures
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_makeClosureWithCaptures(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t result, sysbvm_tuple_t functionDefinition, sysbvm_tuple_t captures);

/**
 * Move instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_move(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t destination, sysbvm_tuple_t value);

/**
 * Return.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_return(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t value);

/**
 * Send instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_send(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t result, sysbvm_tuple_t selector, sysbvm_tuple_t receiver, sysbvm_tuple_t arguments);

/**
 * Send with lookup receiver type instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_sendWithLookupReceiverType(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t result, sysbvm_tuple_t receiverLookupType, sysbvm_tuple_t selector, sysbvm_tuple_t receiver, sysbvm_tuple_t arguments);

/**
 * Store instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_store(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t pointer, sysbvm_tuple_t value);

/**
 * Slot at instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_slotAt(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t result, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot);

/**
 * Slot reference at instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_slotReferenceAt(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t result, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot);

/**
 * Slot at put instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_slotAtPut(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot, sysbvm_tuple_t value);

/**
 * Typecheck instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_typecheck(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t expectedType, sysbvm_tuple_t value);

/**
 * Creates a bytecode instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompilerInstruction_create(sysbvm_context_t *context, uint8_t opcode, sysbvm_tuple_t operands);

/**
 * Creates a bytecode instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompilerInstruction_createLabel(sysbvm_context_t *context);

/**
 * Creates a bytecode instruction vector operand.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompilerInstructionVectorOperand_create(sysbvm_context_t *context, sysbvm_operandVectorName_t vectorName, int16_t vectorIndex);

#endif //SYSBVM_BYTECODE_COMPILER_H
