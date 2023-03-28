#include "tuuvm/bytecodeCompiler.h"
#include "internal/context.h"

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_create(tuuvm_context_t *context)
{
    tuuvm_bytecodeCompiler_t *result = (tuuvm_bytecodeCompiler_t*)tuuvm_context_allocatePointerTuple(context, context->roots.bytecodeCompilerType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_bytecodeCompiler_t));
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompilerInstruction_create(tuuvm_context_t *context, uint8_t opcode, tuuvm_tuple_t operands)
{
    tuuvm_bytecodeCompilerInstruction_t *result = (tuuvm_bytecodeCompilerInstruction_t*)tuuvm_context_allocatePointerTuple(context, context->roots.bytecodeCompilerInstructionType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_bytecodeCompilerInstruction_t));
    result->opcode = tuuvm_tuple_uint8_encode(opcode);
    result->operands = operands;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompilerInstruction_createLabel(tuuvm_context_t *context)
{
    tuuvm_bytecodeCompilerInstruction_t *result = (tuuvm_bytecodeCompilerInstruction_t*)tuuvm_context_allocatePointerTuple(context, context->roots.bytecodeCompilerInstructionType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_bytecodeCompilerInstruction_t));
    return (tuuvm_tuple_t)result;
}

TUUVM_API void tuuvm_bytecodeCompiler_addInstruction(tuuvm_tuple_t compiler, tuuvm_tuple_t instruction)
{
    tuuvm_bytecodeCompiler_t *compilerObject = (tuuvm_bytecodeCompiler_t*)compiler;
    tuuvm_bytecodeCompilerInstruction_t *instructionObject = (tuuvm_bytecodeCompilerInstruction_t*)instruction;

    instructionObject->previous = (tuuvm_bytecodeCompilerInstruction_t*)compilerObject->lastInstruction;

    if(compilerObject->lastInstruction)
    {
        ((tuuvm_bytecodeCompilerInstruction_t*)compilerObject->lastInstruction)->next = instructionObject;
    }
    else
    {
        compilerObject->firstInstruction = instructionObject;
    }

    compilerObject->lastInstruction = instructionObject;
}

void tuuvm_bytecodeCompiler_registerPrimitives(void)
{
}

void tuuvm_bytecodeCompiler_setupPrimitives(tuuvm_context_t *context)
{
    (void)context;
}
