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
    TUUVM_OPCODE_CALL = 0x40, /// <Result> := call <Function> <Arguments>...
    TUUVM_OPCODE_UNCHECKED_CALL = 0x50, /// <Result> := call <Function> <Arguments>...
    TUUVM_OPCODE_SEND = 0x60, /// <Result> := send <Selector> <Receiver> <Arguments>...
    TUUVM_OPCODE_SEND_WITH_LOOKUP = 0x70, /// <Result> := sendWithLookup <ReceiverType> <Selector> <Receiver> <Arguments>....

    TUUVM_OPCODE_MAKE_ARRAY_WITH_ELEMENTS = 0x80, /// <Result> := makeArrayWithElements <Elements>...
    TUUVM_OPCODE_MAKE_BYTE_ARRAY_WITH_ELEMENTS = 0x90, /// <Result> := makeByteArrayWithElements <Elements>...
    TUUVM_OPCODE_MAKE_CLOSURE_WITH_CAPTURES = 0xA0, /// <Result> := makeClosureWithCapture <FunctionDefinition> <Captures>..
    TUUVM_OPCODE_MAKE_DICTIONARY_WITH_ELEMENTS = 0xB0, /// <Result> := makeDictionaryWithElements <Elements>...
    TUUVM_OPCODE_MAKE_TUPLE_WITH_ELEMENTS = 0xC0, /// <Result> := makeTupleWithElements <Elements>...

    TUUVM_OPCODE_FIRST_VARIABLE = TUUVM_OPCODE_CALL
} tuuvm_opcode_t;

typedef enum tuuvm_operandVectorName_e
{
    TUUVM_OPERAND_VECTOR_ARGUMENTS = 0,
    TUUVM_OPERAND_VECTOR_CAPTURES = 1,
    TUUVM_OPERAND_VECTOR_LITERAL = 2,
    TUUVM_OPERAND_VECTOR_LOCAL = 3,

    TUUVM_OPERAND_VECTOR_BITS = 2,
    TUUVM_OPERAND_VECTOR_BITMASK = 3,
} tuuvm_operandVectorName_t;

typedef struct tuuvm_functionBytecode_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t argumentCount;
    tuuvm_tuple_t captureVectorSize;
    tuuvm_tuple_t localVectorSize;
    tuuvm_tuple_t literalVector;
    tuuvm_tuple_t instructions;

    tuuvm_tuple_t definition;
    tuuvm_tuple_t pcToDebugListTable;
    tuuvm_tuple_t debugSourceASTNodes;
    tuuvm_tuple_t debugSourcePositions;
    tuuvm_tuple_t debugSourceEnvironments;

    tuuvm_tuple_t jittedCode;
    tuuvm_tuple_t jittedCodeSessionToken;
    tuuvm_tuple_t jittedCodeTrampoline;
    tuuvm_tuple_t jittedCodeTrampolineSessionToken;
} tuuvm_functionBytecode_t;

typedef struct tuuvm_stackFrameBytecodeFunctionActivationRecord_s tuuvm_stackFrameBytecodeFunctionActivationRecord_t;
typedef struct tuuvm_stackFrameBytecodeFunctionJitActivationRecord_s tuuvm_stackFrameBytecodeFunctionJitActivationRecord_t;

TUUVM_API uint8_t tuuvm_bytecodeInterpreter_destinationOperandCountForOpcode(uint8_t opcode);

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeInterpreter_apply(tuuvm_context_t *context, tuuvm_tuple_t function, size_t argumentCount, tuuvm_tuple_t *arguments);
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeInterpreter_applyJitTrampolineDestination(tuuvm_context_t *context, tuuvm_tuple_t function, size_t argumentCount, tuuvm_tuple_t *arguments);
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeInterpreter_getSourcePositionForActivationRecord(tuuvm_context_t *context, tuuvm_stackFrameBytecodeFunctionActivationRecord_t *activationRecord);
TUUVM_API tuuvm_tuple_t tuuvm_bytecodeInterpreter_getSourcePositionForJitActivationRecord(tuuvm_context_t *context, tuuvm_stackFrameBytecodeFunctionJitActivationRecord_t *activationRecord);

#endif //TUUVM_BYTECODE_H
