#ifndef SYSBVM_BYTECODE_H
#define SYSBVM_BYTECODE_H

#pragma once

#include "tuple.h"

typedef struct sysbvm_context_s sysbvm_context_t;
typedef struct sysbvm_pic_s sysbvm_pic_t;

typedef enum sysbvm_opcode_e
{
    // Zero operand instructions
    SYSBVM_OPCODE_NOP,
    SYSBVM_OPCODE_BREAKPOINT,
    SYSBVM_OPCODE_UNREACHABLE,

    // One operand instructions
    SYSBVM_OPCODE_RETURN = 0x10, /// return <Result>
    SYSBVM_OPCODE_JUMP, /// jump <Destination Relative PC>
    SYSBVM_OPCODE_COUNT_EXTENSION = 0x1F, /// countExt <Amount>

    // Two operand instructions
    SYSBVM_OPCODE_ALLOCA = 0x20, /// <Result> := alloca <Pointer Type>
    SYSBVM_OPCODE_MOVE, /// <Destination> := move <Source>
    SYSBVM_OPCODE_LOAD, /// <Destination> := load <PointerLikeValue>
    SYSBVM_OPCODE_LOAD_SYMBOL_VALUE_BINDING, /// <Destination> := loadSymbolValueBinding <SymbolValueBinding>
    SYSBVM_OPCODE_STORE, /// store <PointerLikeValue> <Value>
    SYSBVM_OPCODE_JUMP_IF_TRUE, /// jumpIfTrue <Condition> <Destination Relative PC>
    SYSBVM_OPCODE_JUMP_IF_FALSE, /// jumpIfFalse <Condition> <Destination Relative PC>
    SYSBVM_OPCODE_SET_DEBUG_VALUE, /// setDebugValue <Value> <Binding>

    // Three operand instructions
    SYSBVM_OPCODE_ALLOCA_WITH_VALUE = 0x30, /// <Result> := alloca <Pointer Type> <Value>
    SYSBVM_OPCODE_COERCE_VALUE, /// <Result> = coerceValue <Type> <Value>
    SYSBVM_OPCODE_DOWNCAST_VALUE, /// <Result> = downCast <Type> <Value>
    SYSBVM_OPCODE_UNCHECKED_DOWNCAST_VALUE, /// <Result> = uncheckedDownCast <Type> <Value>
    SYSBVM_OPCODE_MAKE_ASSOCIATION, /// <Result> = makeAssociation <Key> <Value>
    SYSBVM_OPCODE_MAKE_CLOSURE_WITH_VECTOR, /// <Result> = makeClosureWithVector <Function Definition> <Capture vector>
    SYSBVM_OPCODE_SLOT_AT, /// <Result> = slotAt <Tuple> <TypeSlot>
    SYSBVM_OPCODE_SLOT_REFERENCE_AT, /// <Result> = slotAt <Tuple> <TypeSlot>
    SYSBVM_OPCODE_SLOT_AT_PUT, /// slotAt <Tuple> <TypeSlot> <Value>
    SYSBVM_OPCODE_REF_SLOT_AT, /// <Result> = refSlotAt <Tuple> <TypeSlot>
    SYSBVM_OPCODE_REF_SLOT_REFERENCE_AT, /// <Result> = refSlotReferenceAt <Tuple> <TypeSlot>
    SYSBVM_OPCODE_REF_SLOT_AT_PUT, /// refSlotAtPut <Tuple> <TypeSlot> <Value>

    // Variable operand instructions
    SYSBVM_OPCODE_CALL = 0x40, /// <Result> := call <Function> <Arguments>...
    SYSBVM_OPCODE_UNCHECKED_CALL = 0x50, /// <Result> := call <Function> <Arguments>...
    SYSBVM_OPCODE_SEND = 0x60, /// <Result> := send <Selector> <Receiver> <Arguments>...
    SYSBVM_OPCODE_SEND_WITH_LOOKUP = 0x70, /// <Result> := sendWithLookup <ReceiverType> <Selector> <Receiver> <Arguments>....

    SYSBVM_OPCODE_MAKE_ARRAY_WITH_ELEMENTS = 0x80, /// <Result> := makeArrayWithElements <Elements>...
    SYSBVM_OPCODE_MAKE_BYTE_ARRAY_WITH_ELEMENTS = 0x90, /// <Result> := makeByteArrayWithElements <Elements>...
    SYSBVM_OPCODE_MAKE_CLOSURE_WITH_CAPTURES = 0xA0, /// <Result> := makeClosureWithCapture <FunctionDefinition> <Captures>..
    SYSBVM_OPCODE_MAKE_DICTIONARY_WITH_ELEMENTS = 0xB0, /// <Result> := makeDictionaryWithElements <Elements>...
    SYSBVM_OPCODE_MAKE_TUPLE_WITH_ELEMENTS = 0xC0, /// <Result> := makeTupleWithElements <Elements>...

    SYSBVM_OPCODE_CASE_JUMP = 0xE0, /// caseJump <Value> <Key>... <Destination>... <DefaultDestination>

    SYSBVM_OPCODE_FIRST_VARIABLE = SYSBVM_OPCODE_CALL
} sysbvm_opcode_t;

typedef enum sysbvm_operandVectorName_e
{
    SYSBVM_OPERAND_VECTOR_ARGUMENTS = 0,
    SYSBVM_OPERAND_VECTOR_CAPTURES = 1,
    SYSBVM_OPERAND_VECTOR_LITERAL = 2,
    SYSBVM_OPERAND_VECTOR_LOCAL = 3,

    SYSBVM_OPERAND_VECTOR_BITS = 2,
    SYSBVM_OPERAND_VECTOR_BITMASK = 3,
} sysbvm_operandVectorName_t;

typedef struct sysbvm_functionBytecode_s
{
    sysbvm_tuple_header_t header;
    sysbvm_tuple_t argumentCount;
    sysbvm_tuple_t captureVectorSize;
    sysbvm_tuple_t localVectorSize;
    sysbvm_tuple_t literalVector;
    sysbvm_tuple_t instructions;

    sysbvm_tuple_t definition;

    sysbvm_tuple_t arguments;
    sysbvm_tuple_t captures;
    sysbvm_tuple_t temporaryTypes;
    
    sysbvm_tuple_t sourcePosition;
    sysbvm_tuple_t debugSourcePositions;
    sysbvm_tuple_t debugSourceEnvironments;

    sysbvm_tuple_t jittedCode;
    sysbvm_tuple_t jittedCodeWritePointer;
    sysbvm_tuple_t jittedCodeSessionToken;

    sysbvm_tuple_t jittedCodeTrampoline;
    sysbvm_tuple_t jittedCodeTrampolineWritePointer;
    sysbvm_tuple_t jittedCodeTrampolineSessionToken;
} sysbvm_functionBytecode_t;

typedef struct sysbvm_stackFrameBytecodeFunctionActivationRecord_s sysbvm_stackFrameBytecodeFunctionActivationRecord_t;
typedef struct sysbvm_stackFrameBytecodeFunctionJitActivationRecord_s sysbvm_stackFrameBytecodeFunctionJitActivationRecord_t;

SYSBVM_API uint8_t sysbvm_bytecodeInterpreter_destinationOperandCountForOpcode(uint8_t opcode);
SYSBVM_API uint8_t sysbvm_bytecodeInterpreter_offsetOperandCountForOpcode(uint8_t opcode);

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeInterpreter_apply(sysbvm_context_t *context, sysbvm_tuple_t function, size_t argumentCount, sysbvm_tuple_t *arguments);
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeInterpreter_applyJitTrampolineDestination(sysbvm_context_t *context, sysbvm_tuple_t function, size_t argumentCount, sysbvm_tuple_t *arguments);
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeInterpreter_getSourcePositionForActivationRecord(sysbvm_context_t *context, sysbvm_stackFrameBytecodeFunctionActivationRecord_t *activationRecord);
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeInterpreter_getSourcePositionForJitActivationRecord(sysbvm_context_t *context, sysbvm_stackFrameBytecodeFunctionJitActivationRecord_t *activationRecord);

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeInterpreter_functionApplyNoCopyArguments(sysbvm_context_t *context, sysbvm_tuple_t function, size_t argumentCount, sysbvm_tuple_t *arguments, sysbvm_bitflags_t applicationFlags);
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeInterpreter_interpretSendNoCopyArguments(sysbvm_context_t *context, sysbvm_pic_t *pic, sysbvm_tuple_t selector, size_t argumentCount, sysbvm_tuple_t *receiverAndArguments, sysbvm_bitflags_t applicationFlags);
SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeInterpreter_interpretSendWithReceiverTypeNoCopyArguments(sysbvm_context_t *context, sysbvm_pic_t *pic, sysbvm_tuple_t receiverType, sysbvm_tuple_t selector, size_t argumentCount, sysbvm_tuple_t *receiverAndArguments, sysbvm_bitflags_t applicationFlags);

#endif //SYSBVM_BYTECODE_H
