#ifndef SYSBVM_BYTECODE_COMPILER_H
#define SYSBVM_BYTECODE_COMPILER_H

#pragma once

#include "bytecode.h"

typedef struct sysbvm_functionDefinition_s sysbvm_functionDefinition_t;

typedef struct sysbvm_functionBytecodeAssemblerAbstractOperand_s
{
    sysbvm_tuple_header_t header;
} sysbvm_functionBytecodeAssemblerAbstractOperand_t;

typedef struct sysbvm_functionBytecodeAssemblerAbstractInstruction_s
{
    sysbvm_functionBytecodeAssemblerAbstractOperand_t super;

    sysbvm_tuple_t pc;
    sysbvm_tuple_t endPC;
    struct sysbvm_functionBytecodeAssemblerAbstractInstruction_s *previous;
    struct sysbvm_functionBytecodeAssemblerAbstractInstruction_s *next;

    sysbvm_tuple_t sourcePosition;
    sysbvm_tuple_t sourceEnvironment;
    sysbvm_tuple_t sourceASTNode;
} sysbvm_functionBytecodeAssemblerAbstractInstruction_t;

typedef struct sysbvm_functionBytecodeAssemblerInstruction_s
{
    sysbvm_functionBytecodeAssemblerAbstractInstruction_t super;

    sysbvm_tuple_t standardOpcode;
    sysbvm_tuple_t operands;
} sysbvm_functionBytecodeAssemblerInstruction_t;

typedef struct sysbvm_functionBytecodeAssemblerLabel_s
{
    sysbvm_functionBytecodeAssemblerAbstractInstruction_t super;
} sysbvm_functionBytecodeAssemblerLabel_t;

typedef struct sysbvm_functionBytecodeAssemblerVectorOperand_s
{
    sysbvm_functionBytecodeAssemblerAbstractOperand_t super;

    sysbvm_tuple_t index;
    sysbvm_tuple_t vectorType;

    sysbvm_tuple_t hasAllocaDestination;
    sysbvm_tuple_t hasNonAllocaDestination;
    sysbvm_tuple_t hasSlotReferenceAtDestination;
    sysbvm_tuple_t hasNonSlotReferenceAtDestination;

    sysbvm_tuple_t hasLoadStoreUsage;
    sysbvm_tuple_t hasNonLoadStoreUsage;

    sysbvm_tuple_t optimizationTupleOperand;
    sysbvm_tuple_t optimizationTypeSlotOperand;
} sysbvm_functionBytecodeAssemblerVectorOperand_t;

typedef struct sysbvm_functionBytecodeAssembler_s
{
    sysbvm_tuple_header_t header;

    sysbvm_tuple_t arguments;
    sysbvm_tuple_t captures;
    sysbvm_tuple_t literals;
    sysbvm_tuple_t literalDictionary;
    sysbvm_tuple_t temporaries;
    sysbvm_tuple_t temporaryTypes;
    sysbvm_tuple_t usedTemporaryCount;

    sysbvm_functionBytecodeAssemblerAbstractInstruction_t *firstInstruction;
    sysbvm_functionBytecodeAssemblerAbstractInstruction_t *lastInstruction;

    sysbvm_tuple_t sourcePosition;
    sysbvm_tuple_t sourceEnvironment;
    sysbvm_tuple_t sourceASTNode;
} sysbvm_functionBytecodeAssembler_t;

typedef struct sysbvm_functionBytecodeDirectCompiler_s
{
    sysbvm_tuple_header_t header;

    sysbvm_functionBytecodeAssembler_t *assembler;
    sysbvm_tuple_t bindingDictionary;

    sysbvm_tuple_t breakLabel;
    sysbvm_tuple_t continueLabel;
} sysbvm_functionBytecodeDirectCompiler_t;

/**
 * Creates an instance of the bytecode assembler.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_create(sysbvm_context_t *context);

/**
 * Creates an instance of the bytecode compiler.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeDirectCompiler_create(sysbvm_context_t *context);

/**
 * Set the argument count.
 */
SYSBVM_API void sysbvm_functionBytecodeAssembler_setArgumentCount(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, size_t argumentCount);

/**
 * Set the capture count.
 */
SYSBVM_API void sysbvm_functionBytecodeAssembler_setCaptureCount(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, size_t argumentCount);

/**
 * Adds a literal
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_addLiteral(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t literalValue);

/**
 * Creates an instruction to the bytecode compiler.
 */
SYSBVM_API void sysbvm_functionBytecodeAssembler_addInstruction(sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t instruction);

/**
 * Compiles the AST node with the bytecode compiler.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeDirectCompiler_compileASTNode(sysbvm_context_t *context, sysbvm_functionBytecodeDirectCompiler_t *compiler, sysbvm_tuple_t astNode);

/**
 * Compiles the AST node with the bytecode compiler and the given break and continue labels.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeDirectCompiler_compileASTNodeWithBreakAndContinue(sysbvm_context_t *context, sysbvm_functionBytecodeDirectCompiler_t *compiler, sysbvm_tuple_t astNode, sysbvm_tuple_t breakLabel, sysbvm_tuple_t continueLabel);

/**
 * Compiles the AST node with the bytecode compiler and the given environment.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeDirectCompiler_compileASTNodeWithEnvironment(sysbvm_context_t *context, sysbvm_functionBytecodeDirectCompiler_t *compiler, sysbvm_tuple_t astNode, sysbvm_tuple_t environment);


/**
 * Compiles sets the value for the given binding.
 */
SYSBVM_API void sysbvm_functionBytecodeDirectCompiler_setBindingValue(sysbvm_context_t *context, sysbvm_functionBytecodeDirectCompiler_t *compiler, sysbvm_tuple_t binding, sysbvm_tuple_t value);

/**
 * Compiles gets the value for the given binding.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeDirectCompiler_getBindingValue(sysbvm_context_t *context, sysbvm_functionBytecodeDirectCompiler_t *compiler, sysbvm_tuple_t binding);

/**
 * Compiles the given function definition.
 */
SYSBVM_API void sysbvm_functionBytecodeDirectCompiler_compileFunctionDefinition(sysbvm_context_t *context, sysbvm_functionDefinition_t *definition);

/**
 * Makes a temporary.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_newTemporary(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t type);

/**
 * Alloca with value.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_allocaWithValue(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t pointerLikeType, sysbvm_tuple_t value);

/**
 * Call instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_call(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t function, sysbvm_tuple_t arguments);

/**
 * Unchecked call instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_uncheckedCall(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t function, sysbvm_tuple_t arguments);

/**
 * Coerce value.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_coerceValue(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t type, sysbvm_tuple_t value);

/**
 * Down cast value.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_downCastValue(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t type, sysbvm_tuple_t value);

/**
 * Unchecked downcast cast value.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_uncheckedDownCastValue(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t type, sysbvm_tuple_t value);

/**
 * Jump instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_jump(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t destination);

/**
 * Jump if true instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_jumpIfTrue(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t condition, sysbvm_tuple_t destination);

/**
 * Jump if false instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_jumpIfFalse(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t condition, sysbvm_tuple_t destination);

/**
 * Load instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_load(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t destination, sysbvm_tuple_t pointer);

/**
 * Make array with elements.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_makeArrayWithElements(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t elements);

/**
 * Make an association.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_makeAssociation(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t key, sysbvm_tuple_t value);

/**
 * Make a byte array with elements.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_makeByteArrayWithElements(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t elements);

/**
 * Make a dictionary with elements.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_makeDictionaryWithElements(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t elements);

/**
 * Make closure with captures
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_makeClosureWithCaptures(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t functionDefinition, sysbvm_tuple_t captures);

/**
 * Move instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_move(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t destination, sysbvm_tuple_t value);

/**
 * Return.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_return(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t value);

/**
 * Send instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_send(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t selector, sysbvm_tuple_t receiver, sysbvm_tuple_t arguments);

/**
 * Send with lookup receiver type instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_sendWithLookupReceiverType(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t receiverLookupType, sysbvm_tuple_t selector, sysbvm_tuple_t receiver, sysbvm_tuple_t arguments);

/**
 * Store instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_store(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t pointer, sysbvm_tuple_t value);

/**
 * Slot at instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_slotAt(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot);

/**
 * Slot reference at instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_slotReferenceAt(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot);

/**
 * Reference slot at put instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_refSlotAtPut(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot, sysbvm_tuple_t value);

/**
 * Reference slot at instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_refSlotAt(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot);

/**
 * Reference slot reference at instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_refSlotReferenceAt(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot);

/**
 * Slot at put instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_slotAtPut(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot, sysbvm_tuple_t value);

/**
 * Creates a bytecode instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssemblerInstruction_create(sysbvm_context_t *context, uint8_t opcode, sysbvm_tuple_t operands);

/**
 * Creates a bytecode instruction.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssemblerInstruction_createLabel(sysbvm_context_t *context);

/**
 * Creates a bytecode instruction vector operand.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssemblerVectorOperand_create(sysbvm_context_t *context, sysbvm_operandVectorName_t vectorName, int16_t vectorIndex);

#endif //SYSBVM_BYTECODE_COMPILER_H
