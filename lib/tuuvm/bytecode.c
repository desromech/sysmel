#include "tuuvm/bytecode.h"
#include "tuuvm/context.h"
#include <stdlib.h>

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeInterpreter_apply(tuuvm_context_t *context, tuuvm_tuple_t *function, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    abort();
    return TUUVM_NULL_TUPLE;
}

void tuuvm_bytecode_registerPrimitives(void)
{
}

void tuuvm_bytecode_setupPrimitives(tuuvm_context_t *context)
{
    // Export the function opcodes.

    // Zero operands.
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::Nop", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_NOP));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::Trap", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_TRAP));

    // One operands.
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::Return", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_RETURN));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::Jump", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_JUMP));

    // Two operands.
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::Alloca", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_ALLOCA));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::Move", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_MOVE));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::Load", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_LOAD));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::Store", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_STORE));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::JumpIfTrue", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_JUMP_IF_TRUE));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::JumpIfFalse", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_JUMP_IF_FALSE));

    // Three operands.
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::MakeAssociation", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_MAKE_ASSOCIATION));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::MakeClosureWithVector", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_MAKE_CLOSURE_WITH_VECTOR));

    // Variable operand count.
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::Call", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_CALL));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::UncheckedCall", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_UNCHECKED_CALL));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::Send", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_SEND));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::SendWithLookup", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_SEND_WITH_LOOKUP));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::MakeArrayWithElements", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_MAKE_ARRAY_WITH_ELEMENTS));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::MakeByteArrayWithElements", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_MAKE_BYTE_ARRAY_WITH_ELEMENTS));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::MakeClosureWithCaptures", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_MAKE_CLOSURE_WITH_CAPTURES));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::MakeDictionaryWithElements", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_MAKE_DICTIONARY_WITH_KEY_VALUES));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::MakeTupleWithElements", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_MAKE_TUPLE_WITH_ELEMENTS));

    // Export the operand vector names.
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::OperandVectorName::Arguments", tuuvm_tuple_uint8_encode(TUUVM_OPERAND_VECTOR_ARGUMENTS));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::OperandVectorName::Captures", tuuvm_tuple_uint8_encode(TUUVM_OPERAND_VECTOR_CAPTURES));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::OperandVectorName::Literal", tuuvm_tuple_uint8_encode(TUUVM_OPERAND_VECTOR_LITERAL));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::OperandVectorName::Local", tuuvm_tuple_uint8_encode(TUUVM_OPERAND_VECTOR_LOCAL));
}
