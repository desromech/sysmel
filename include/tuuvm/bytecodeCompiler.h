#ifndef TUUVM_BYTECODE_COMPILER_H
#define TUUVM_BYTECODE_COMPILER_H

#pragma once

#include "bytecode.h"

typedef struct tuuvm_functionDefinition_s tuuvm_functionDefinition_t;

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
    tuuvm_tuple_t endPC;
    tuuvm_tuple_t opcode;
    tuuvm_tuple_t operands;

    tuuvm_tuple_t sourcePosition;
    tuuvm_tuple_t sourceEnvironment;
    tuuvm_tuple_t sourceASTNode;
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

    tuuvm_tuple_t arguments;
    tuuvm_tuple_t captures;
    tuuvm_tuple_t literals;
    tuuvm_tuple_t literalDictionary;
    tuuvm_tuple_t temporaries;
    tuuvm_tuple_t usedTemporaryCount;

    tuuvm_bytecodeCompilerInstruction_t *firstInstruction;
    tuuvm_bytecodeCompilerInstruction_t *lastInstruction;

    tuuvm_tuple_t breakLabel;
    tuuvm_tuple_t continueLabel;

    tuuvm_tuple_t sourcePosition;
    tuuvm_tuple_t sourceEnvironment;
    tuuvm_tuple_t sourceASTNode;

    tuuvm_tuple_t bindingDictionary;
} tuuvm_bytecodeCompiler_t;

/**
 * Creates an instance of the bytecode compiler.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_create(tuuvm_context_t *context);

/**
 * Set the argument count.
 */
TUUVM_API void tuuvm_bytecodeCompiler_setArgumentCount(tuuvm_context_t *context, tuuvm_tuple_t compiler, size_t argumentCount);

/**
 * Set the capture count.
 */
TUUVM_API void tuuvm_bytecodeCompiler_setCaptureCount(tuuvm_context_t *context, tuuvm_tuple_t compiler, size_t argumentCount);

/**
 * Adds a literal
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_addLiteral(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t literalValue);

/**
 * Creates an instruction to the bytecode compiler.
 */
TUUVM_API void tuuvm_bytecodeCompiler_addInstruction(tuuvm_tuple_t compiler, tuuvm_tuple_t instruction);

/**
 * Compiles the AST node with the bytecode compiler.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_compileASTNode(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t astNode);

/**
 * Compiles the AST node with the bytecode compiler and the given break and continue labels.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_compileASTNodeWithBreakAndContinue(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t astNode, tuuvm_tuple_t breakLabel, tuuvm_tuple_t continueLabel);

/**
 * Compiles the AST node with the bytecode compiler and the given environment.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_compileASTNodeWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t astNode, tuuvm_tuple_t environment);


/**
 * Compiles sets the value for the given binding.
 */
TUUVM_API void tuuvm_bytecodeCompiler_setBindingValue(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t binding, tuuvm_tuple_t value);

/**
 * Compiles gets the value for the given binding.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_getBindingValue(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t binding);

/**
 * Compiles the given function definition.
 */
TUUVM_API void tuuvm_bytecodeCompiler_compileFunctionDefinition(tuuvm_context_t *context, tuuvm_functionDefinition_t *definition);

/**
 * Makes a temporary.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_newTemporary(tuuvm_context_t *context, tuuvm_tuple_t compiler);

/**
 * Alloca with value.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_allocaWithValue(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t result, tuuvm_tuple_t pointerLikeType, tuuvm_tuple_t value);

/**
 * Call instruction.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_call(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t result, tuuvm_tuple_t function, tuuvm_tuple_t arguments);

/**
 * Unchecked call instruction.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_uncheckedCall(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t result, tuuvm_tuple_t function, tuuvm_tuple_t arguments);

/**
 * Coerce value.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_coerceValue(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t result, tuuvm_tuple_t type, tuuvm_tuple_t value);

/**
 * Jump instruction.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_jump(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t destination);

/**
 * Jump if true instruction.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_jumpIfTrue(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t condition, tuuvm_tuple_t destination);

/**
 * Jump if false instruction.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_jumpIfFalse(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t condition, tuuvm_tuple_t destination);

/**
 * Load instruction.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_load(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t destination, tuuvm_tuple_t pointer);

/**
 * Make array with elements.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_makeArrayWithElements(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t result, tuuvm_tuple_t elements);

/**
 * Make an association.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_makeAssociation(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t result, tuuvm_tuple_t key, tuuvm_tuple_t value);

/**
 * Make a byte array with elements.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_makeByteArrayWithElements(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t result, tuuvm_tuple_t elements);

/**
 * Make a dictionary with elements.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_makeDictionaryWithElements(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t result, tuuvm_tuple_t elements);

/**
 * Make closure with captures
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_makeClosureWithCaptures(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t result, tuuvm_tuple_t functionDefinition, tuuvm_tuple_t captures);

/**
 * Move instruction.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_move(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t destination, tuuvm_tuple_t value);

/**
 * Return.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_return(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t value);

/**
 * Send instruction.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_send(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t result, tuuvm_tuple_t selector, tuuvm_tuple_t receiver, tuuvm_tuple_t arguments);

/**
 * Send with lookup receiver type instruction.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_sendWithLookupReceiverType(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t result, tuuvm_tuple_t receiverLookupType, tuuvm_tuple_t selector, tuuvm_tuple_t receiver, tuuvm_tuple_t arguments);

/**
 * Store instruction.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_store(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t pointer, tuuvm_tuple_t value);

/**
 * Typecheck instruction.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_typecheck(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t expectedType, tuuvm_tuple_t value);

/**
 * Creates a bytecode instruction.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompilerInstruction_create(tuuvm_context_t *context, uint8_t opcode, tuuvm_tuple_t operands);

/**
 * Creates a bytecode instruction.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompilerInstruction_createLabel(tuuvm_context_t *context);

/**
 * Creates a bytecode instruction vector operand.
 */
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompilerInstructionVectorOperand_create(tuuvm_context_t *context, tuuvm_operandVectorName_t vectorName, int16_t vectorIndex);

#endif //TUUVM_BYTECODE_COMPILER_H
