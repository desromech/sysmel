#ifndef TUUVM_BYTECODE_H
#define TUUVM_BYTECODE_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef enum tuuvm_opcode_e
{
    // Zero operand instructions
    TUUVM_OPCODE_NOP,
    TUUVM_OPCODE_TRAP,

    // One operand instructions
    TUUVM_OPCODE_RETURN = 0x10, /// return <Result>
    TUUVM_OPCODE_JUMP, /// jump <Destination Relative PC>

    // Two operand instructions
    TUUVM_OPCODE_ALLOCA = 0x20, /// <Result> := alloca <Pointer Type>
    TUUVM_OPCODE_MOVE, /// <Destination> := move <Source>
    TUUVM_OPCODE_LOAD, /// <Destination> := load <PointerLikeValue>
    TUUVM_OPCODE_STORE, /// store <PointerLikeValue> <Value>
    TUUVM_OPCODE_JUMP_IF_TRUE, /// jumpIfTrue <Condition> <Destination Relative PC>
    TUUVM_OPCODE_JUMP_IF_FALSE, /// jumpIfFalse <Condition> <Destination Relative PC>

    // Three operand instructions
    TUUVM_OPCODE_ALLOCA_WITH_VALUE = 0x30, /// <Result> := alloca <Pointer Type> <Value>
    TUUVM_OPCODE_COERCE_VALUE, /// <Result> = coerceValue <Type> <Value>
    TUUVM_OPCODE_MAKE_ASSOCIATION, /// <Result> = makeAssociation <Key> <Value>
    TUUVM_OPCODE_MAKE_CLOSURE_WITH_VECTOR, /// <Result> = makeClosureWithVector <Function Definition> <Capture vector>

    // Variable operand instructions
    TUUVM_OPCODE_CALL = 0x40,
    TUUVM_OPCODE_UNCHECKED_CALL = 0x50,
    TUUVM_OPCODE_SEND = 0x60,
    TUUVM_OPCODE_SEND_WITH_LOOKUP = 0x70,

    TUUVM_OPCODE_MAKE_ARRAY_WITH_ELEMENTS = 0x80,
    TUUVM_OPCODE_MAKE_BYTE_ARRAY_WITH_ELEMENTS = 0x90,
    TUUVM_OPCODE_MAKE_CLOSURE_WITH_CAPTURES = 0xA0,
    TUUVM_OPCODE_MAKE_DICTIONARY_WITH_KEY_VALUES = 0xB0,
    TUUVM_OPCODE_MAKE_TUPLE_WITH_ELEMENTS = 0xC0,
} tuuvm_opcode_t;

typedef enum tuuvm_operandVectorName_e
{
    TUUVM_OPERAND_VECTOR_ARGUMENTS = 0,
    TUUVM_OPERAND_VECTOR_CAPTURES = 1,
    TUUVM_OPERAND_VECTOR_LITERAL = 2,
    TUUVM_OPERAND_VECTOR_LOCAL = 3,
} tuuvm_operandVectorName_t;

typedef struct tuuvm_functionBytecode_s
{
    tuuvm_tuple_t argumentCount;
    tuuvm_tuple_t captureVectorSize;
    tuuvm_tuple_t localVectorSize;
    tuuvm_tuple_t literalVector;
    tuuvm_tuple_t instructions;

    tuuvm_tuple_t definition;
    tuuvm_tuple_t pcToDebugListTable;
    tuuvm_tuple_t debugSourceASTNodes;
    tuuvm_tuple_t debugSourcePosition;
    tuuvm_tuple_t debugSourceEnvironment;
} tuuvm_functionBytecode_t;

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeInterpreter_apply(tuuvm_context_t *context, tuuvm_tuple_t *function, size_t argumentCount, tuuvm_tuple_t *arguments);

#endif //TUUVM_BYTECODE_H
