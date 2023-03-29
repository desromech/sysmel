#include "tuuvm/bytecodeCompiler.h"
#include "tuuvm/array.h"
#include "tuuvm/arrayList.h"
#include "tuuvm/ast.h"
#include "tuuvm/dictionary.h"
#include "tuuvm/environment.h"
#include "tuuvm/errors.h"
#include "tuuvm/function.h"
#include "tuuvm/stackFrame.h"
#include "internal/context.h"
#include <stdlib.h>

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

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompilerInstructionVectorOperand_create(tuuvm_context_t *context, tuuvm_operandVectorName_t vectorName, int16_t vectorIndex)
{
    tuuvm_bytecodeCompilerInstructionVectorOperand_t *result = (tuuvm_bytecodeCompilerInstructionVectorOperand_t*)tuuvm_context_allocatePointerTuple(context, context->roots.bytecodeCompilerInstructionVectorOperandType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_bytecodeCompilerInstructionVectorOperand_t));
    result->vectorType = tuuvm_tuple_int16_encode(vectorName);
    result->index = tuuvm_tuple_int16_encode(vectorIndex);
    result->hasAllocaDestination = TUUVM_FALSE_TUPLE;
    result->hasNonAllocaDestination = TUUVM_FALSE_TUPLE;
    result->hasLoadStoreUsage = TUUVM_FALSE_TUPLE;
    result->hasNonLoadStoreUsage = TUUVM_FALSE_TUPLE;
    return (tuuvm_tuple_t)result;
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_create(tuuvm_context_t *context)
{
    tuuvm_bytecodeCompiler_t *result = (tuuvm_bytecodeCompiler_t*)tuuvm_context_allocatePointerTuple(context, context->roots.bytecodeCompilerType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_bytecodeCompiler_t));
    result->arguments = tuuvm_array_create(context, 0);
    result->captures = tuuvm_array_create(context, 0);
    result->literals = tuuvm_arrayList_create(context);
    result->literalDictionary = tuuvm_identityDictionary_create(context);
    result->temporaries = tuuvm_arrayList_create(context);
    result->usedTemporaryCount = tuuvm_tuple_size_encode(context, 0);
    result->bindingDictionary = tuuvm_identityDictionary_create(context);
    return (tuuvm_tuple_t)result;
}

TUUVM_API void tuuvm_bytecodeCompiler_setArgumentCount(tuuvm_context_t *context, tuuvm_tuple_t compiler, size_t argumentCount)
{
    tuuvm_tuple_t arguments = tuuvm_array_create(context, argumentCount);
    for(size_t i = 0; i < argumentCount; ++i )
    {
        tuuvm_tuple_t argumentOperand = tuuvm_bytecodeCompilerInstructionVectorOperand_create(context, TUUVM_OPERAND_VECTOR_ARGUMENTS, i);
        tuuvm_array_atPut(arguments, i, argumentOperand);
    }

    ((tuuvm_bytecodeCompiler_t*)compiler)->arguments = arguments;
}

TUUVM_API void tuuvm_bytecodeCompiler_setCaptureCount(tuuvm_context_t *context, tuuvm_tuple_t compiler, size_t argumentCount)
{
    tuuvm_tuple_t captures = tuuvm_array_create(context, argumentCount);
    for(size_t i = 0; i < argumentCount; ++i )
    {
        tuuvm_tuple_t captureOperand = tuuvm_bytecodeCompilerInstructionVectorOperand_create(context, TUUVM_OPERAND_VECTOR_CAPTURES, i);
        tuuvm_array_atPut(captures, i, captureOperand);
    }

    ((tuuvm_bytecodeCompiler_t*)compiler)->captures = captures;
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_addLiteral(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t literalValue)
{
    tuuvm_bytecodeCompiler_t *compilerObject = (tuuvm_bytecodeCompiler_t*)compiler;
    tuuvm_tuple_t existent = TUUVM_NULL_TUPLE;
    if(tuuvm_identityDictionary_find(compilerObject->literalDictionary, literalValue, &existent))
        return existent;

    size_t literalIndex = tuuvm_arrayList_getSize(compilerObject->literals);
    tuuvm_tuple_t newLiteralOperand = tuuvm_bytecodeCompilerInstructionVectorOperand_create(context, TUUVM_OPERAND_VECTOR_LITERAL, literalIndex);
    tuuvm_arrayList_add(context, compilerObject->literals, literalValue);
    tuuvm_identityDictionary_atPut(context, compilerObject->literalDictionary, literalValue, newLiteralOperand);
    return newLiteralOperand;
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

    instructionObject->sourceASTNode = compilerObject->sourceASTNode;
    instructionObject->sourceEnvironment = compilerObject->sourceEnvironment;
    instructionObject->sourcePosition = compilerObject->sourcePosition;
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_compileASTNode(tuuvm_context_t *context, tuuvm_tuple_t compiler_, tuuvm_tuple_t astNode_)
{
    struct {
        tuuvm_bytecodeCompiler_t *compilerObject;
        tuuvm_tuple_t astNode;
        tuuvm_tuple_t result;

        tuuvm_tuple_t oldSourcePosition;
        tuuvm_tuple_t oldSourceASTNode;
    } gcFrame = {
        .compilerObject = (tuuvm_bytecodeCompiler_t*)compiler_,
        .astNode = astNode_
    };

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.oldSourcePosition = gcFrame.compilerObject->sourcePosition;
    gcFrame.oldSourceASTNode = gcFrame.compilerObject->sourceASTNode;

    gcFrame.compilerObject->sourcePosition = tuuvm_astNode_getSourcePosition(gcFrame.astNode);
    gcFrame.compilerObject->sourceASTNode = gcFrame.astNode;

    gcFrame.result = tuuvm_tuple_send1(context, context->roots.astNodeCompileIntoBytecodeSelector, gcFrame.astNode, (tuuvm_tuple_t)gcFrame.compilerObject);

    gcFrame.compilerObject->sourcePosition = gcFrame.oldSourcePosition;
    gcFrame.compilerObject->sourceASTNode = gcFrame.oldSourceASTNode;

    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_compileASTNodeWithBreakAndContinue(tuuvm_context_t *context, tuuvm_tuple_t compiler_, tuuvm_tuple_t astNode, tuuvm_tuple_t breakLabel, tuuvm_tuple_t continueLabel)
{
    struct {
        tuuvm_bytecodeCompiler_t *compilerObject;
        tuuvm_tuple_t oldBreakLabel;
        tuuvm_tuple_t oldContinueLabel;
        tuuvm_tuple_t result;
    } gcFrame = {
        .compilerObject = (tuuvm_bytecodeCompiler_t*)compiler_,
    };

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.oldBreakLabel = gcFrame.compilerObject->breakLabel;
    gcFrame.oldContinueLabel = gcFrame.compilerObject->continueLabel;
    gcFrame.compilerObject->breakLabel = breakLabel;
    gcFrame.compilerObject->continueLabel = continueLabel;

    gcFrame.result = tuuvm_bytecodeCompiler_compileASTNode(context, (tuuvm_tuple_t)gcFrame.compilerObject, astNode);

    gcFrame.compilerObject->breakLabel = gcFrame.oldBreakLabel;
    gcFrame.compilerObject->continueLabel = gcFrame.oldContinueLabel;

    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_compileASTNodeWithEnvironment(tuuvm_context_t *context, tuuvm_tuple_t compiler_, tuuvm_tuple_t astNode, tuuvm_tuple_t environment)
{
    struct {
        tuuvm_bytecodeCompiler_t *compilerObject;
        tuuvm_tuple_t oldEnvironment;
        tuuvm_tuple_t result;
    } gcFrame = {
        .compilerObject = (tuuvm_bytecodeCompiler_t*)compiler_,
    };

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.oldEnvironment = gcFrame.compilerObject->sourceEnvironment;
    gcFrame.compilerObject->sourceEnvironment = environment;

    gcFrame.result = tuuvm_bytecodeCompiler_compileASTNode(context, (tuuvm_tuple_t)gcFrame.compilerObject, astNode);

    gcFrame.compilerObject->sourceEnvironment = gcFrame.oldEnvironment;

    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

TUUVM_API void tuuvm_bytecodeCompiler_setBindingValue(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t binding, tuuvm_tuple_t value)
{
    tuuvm_bytecodeCompiler_t *compilerObject = (tuuvm_bytecodeCompiler_t*)compiler;
    tuuvm_identityDictionary_atPut(context, compilerObject->bindingDictionary, binding, value);
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_getBindingValue(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t binding)
{
    (void)context;
    tuuvm_tuple_t value = TUUVM_NULL_TUPLE;
    tuuvm_bytecodeCompiler_t *compilerObject = (tuuvm_bytecodeCompiler_t*)compiler;
    if(!tuuvm_identityDictionary_find(compilerObject->bindingDictionary, binding, &value))
        tuuvm_error("Invalid value binding.");

    return value;
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_newTemporary(tuuvm_context_t *context, tuuvm_tuple_t compiler)
{
    tuuvm_bytecodeCompiler_t *compilerObject = (tuuvm_bytecodeCompiler_t*)compiler;
    size_t temporaryIndex = tuuvm_arrayList_getSize(compilerObject->temporaries);
    tuuvm_tuple_t temporaryOperand = tuuvm_bytecodeCompilerInstructionVectorOperand_create(context, TUUVM_OPERAND_VECTOR_LOCAL, temporaryIndex);
    tuuvm_arrayList_add(context, compilerObject->temporaries, temporaryIndex);
    return temporaryOperand;
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_allocaWithValue(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t result, tuuvm_tuple_t pointerLikeType, tuuvm_tuple_t value)
{
    tuuvm_tuple_t operands = tuuvm_array_create(context, 3);
    tuuvm_array_atPut(operands, 0, result);
    tuuvm_array_atPut(operands, 1, pointerLikeType);
    tuuvm_array_atPut(operands, 2, value);

    tuuvm_tuple_t instruction = tuuvm_bytecodeCompilerInstruction_create(context, TUUVM_OPCODE_ALLOCA_WITH_VALUE, operands);
    tuuvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_coerceValue(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t result, tuuvm_tuple_t type, tuuvm_tuple_t value)
{
    tuuvm_tuple_t operands = tuuvm_array_create(context, 3);
    tuuvm_array_atPut(operands, 0, result);
    tuuvm_array_atPut(operands, 1, type);
    tuuvm_array_atPut(operands, 2, value);

    tuuvm_tuple_t instruction = tuuvm_bytecodeCompilerInstruction_create(context, TUUVM_OPCODE_COERCE_VALUE, operands);
    tuuvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_jump(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t destination)
{
    tuuvm_tuple_t operands = tuuvm_array_create(context, 1);
    tuuvm_array_atPut(operands, 0, destination);

    tuuvm_tuple_t instruction = tuuvm_bytecodeCompilerInstruction_create(context, TUUVM_OPCODE_JUMP, operands);
    tuuvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_jumpIfFalse(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t condition, tuuvm_tuple_t destination)
{
    tuuvm_tuple_t operands = tuuvm_array_create(context, 2);
    tuuvm_array_atPut(operands, 0, condition);
    tuuvm_array_atPut(operands, 1, destination);

    tuuvm_tuple_t instruction = tuuvm_bytecodeCompilerInstruction_create(context, TUUVM_OPCODE_JUMP_IF_FALSE, operands);
    tuuvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_jumpIfTrue(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t condition, tuuvm_tuple_t destination)
{
    tuuvm_tuple_t operands = tuuvm_array_create(context, 2);
    tuuvm_array_atPut(operands, 0, condition);
    tuuvm_array_atPut(operands, 1, destination);

    tuuvm_tuple_t instruction = tuuvm_bytecodeCompilerInstruction_create(context, TUUVM_OPCODE_JUMP_IF_TRUE, operands);
    tuuvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_call(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t result, tuuvm_tuple_t function, tuuvm_tuple_t arguments)
{
    size_t argumentCount = tuuvm_array_getSize(arguments);
    tuuvm_tuple_t operands = tuuvm_array_create(context, 2 + argumentCount);
    tuuvm_array_atPut(operands, 0, result);
    tuuvm_array_atPut(operands, 1, function);
    for(size_t i = 0; i < argumentCount; ++i)
        tuuvm_array_atPut(operands, 2 + i, tuuvm_array_at(arguments, i));
    
    tuuvm_tuple_t instruction = tuuvm_bytecodeCompilerInstruction_create(context, TUUVM_OPCODE_CALL + (argumentCount & 0xF), operands);
    tuuvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_uncheckedCall(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t result, tuuvm_tuple_t function, tuuvm_tuple_t arguments)
{
    size_t argumentCount = tuuvm_array_getSize(arguments);
    tuuvm_tuple_t operands = tuuvm_array_create(context, 2 + argumentCount);
    tuuvm_array_atPut(operands, 0, result);
    tuuvm_array_atPut(operands, 1, function);
    for(size_t i = 0; i < argumentCount; ++i)
        tuuvm_array_atPut(operands, 2 + i, tuuvm_array_at(arguments, i));
    
    tuuvm_tuple_t instruction = tuuvm_bytecodeCompilerInstruction_create(context, TUUVM_OPCODE_UNCHECKED_CALL + (argumentCount & 0xF), operands);
    tuuvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_send(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t result, tuuvm_tuple_t selector, tuuvm_tuple_t receiver, tuuvm_tuple_t arguments)
{
    size_t argumentCount = tuuvm_array_getSize(arguments);
    tuuvm_tuple_t operands = tuuvm_array_create(context, 3 + argumentCount);
    tuuvm_array_atPut(operands, 0, result);
    tuuvm_array_atPut(operands, 1, selector);
    tuuvm_array_atPut(operands, 2, receiver);
    for(size_t i = 0; i < argumentCount; ++i)
        tuuvm_array_atPut(operands, 3 + i, tuuvm_array_at(arguments, i));
    
    tuuvm_tuple_t instruction = tuuvm_bytecodeCompilerInstruction_create(context, TUUVM_OPCODE_SEND + (argumentCount & 0xF), operands);
    tuuvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_sendWithLookupReceiverType(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t result, tuuvm_tuple_t receiverLookupType, tuuvm_tuple_t selector, tuuvm_tuple_t receiver, tuuvm_tuple_t arguments)
{
    size_t argumentCount = tuuvm_array_getSize(arguments);
    tuuvm_tuple_t operands = tuuvm_array_create(context, 4 + argumentCount);
    tuuvm_array_atPut(operands, 0, result);
    tuuvm_array_atPut(operands, 1, receiverLookupType);
    tuuvm_array_atPut(operands, 2, selector);
    tuuvm_array_atPut(operands, 3, receiver);
    for(size_t i = 0; i < argumentCount; ++i)
        tuuvm_array_atPut(operands, 4 + i, tuuvm_array_at(arguments, i));
    
    tuuvm_tuple_t instruction = tuuvm_bytecodeCompilerInstruction_create(context, TUUVM_OPCODE_SEND_WITH_LOOKUP + (argumentCount & 0xF), operands);
    tuuvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_makeArrayWithElements(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t result, tuuvm_tuple_t elements)
{
    size_t elementCount = tuuvm_array_getSize(elements);
    tuuvm_tuple_t operands = tuuvm_array_create(context, 1 + elementCount);
    tuuvm_array_atPut(operands, 0, result);
    for(size_t i = 0; i < elementCount; ++i)
        tuuvm_array_atPut(operands, 1 + i, tuuvm_array_at(elements, i));
    
    tuuvm_tuple_t instruction = tuuvm_bytecodeCompilerInstruction_create(context, TUUVM_OPCODE_MAKE_ARRAY_WITH_ELEMENTS + (elementCount & 0xF), operands);
    tuuvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_makeAssociation(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t result, tuuvm_tuple_t key, tuuvm_tuple_t value)
{
    tuuvm_tuple_t operands = tuuvm_array_create(context, 3);
    tuuvm_array_atPut(operands, 0, result);
    tuuvm_array_atPut(operands, 1, key);
    tuuvm_array_atPut(operands, 2, value);
    
    tuuvm_tuple_t instruction = tuuvm_bytecodeCompilerInstruction_create(context, TUUVM_OPCODE_MAKE_ASSOCIATION, operands);
    tuuvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_makeByteArrayWithElements(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t result, tuuvm_tuple_t elements)
{
    size_t elementCount = tuuvm_array_getSize(elements);
    tuuvm_tuple_t operands = tuuvm_array_create(context, 1 + elementCount);
    tuuvm_array_atPut(operands, 0, result);
    for(size_t i = 0; i < elementCount; ++i)
        tuuvm_array_atPut(operands, 1 + i, tuuvm_array_at(elements, i));
    
    tuuvm_tuple_t instruction = tuuvm_bytecodeCompilerInstruction_create(context, TUUVM_OPCODE_MAKE_BYTE_ARRAY_WITH_ELEMENTS + (elementCount & 0xF), operands);
    tuuvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_makeDictionaryWithElements(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t result, tuuvm_tuple_t elements)
{
    size_t elementCount = tuuvm_array_getSize(elements);
    tuuvm_tuple_t operands = tuuvm_array_create(context, 1 + elementCount);
    tuuvm_array_atPut(operands, 0, result);
    for(size_t i = 0; i < elementCount; ++i)
        tuuvm_array_atPut(operands, 1 + i, tuuvm_array_at(elements, i));
    
    tuuvm_tuple_t instruction = tuuvm_bytecodeCompilerInstruction_create(context, TUUVM_OPCODE_MAKE_DICTIONARY_WITH_ELEMENTS + (elementCount & 0xF), operands);
    tuuvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_makeClosureWithCaptures(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t result, tuuvm_tuple_t functionDefinition, tuuvm_tuple_t captures)
{
    size_t captureCount = tuuvm_array_getSize(captures);
    tuuvm_tuple_t operands = tuuvm_array_create(context, 2 + captureCount);
    tuuvm_array_atPut(operands, 0, result);
    tuuvm_array_atPut(operands, 1, functionDefinition);
    for(size_t i = 0; i < captureCount; ++i)
        tuuvm_array_atPut(operands, 2 + i, tuuvm_array_at(captures, i));
    
    tuuvm_tuple_t instruction = tuuvm_bytecodeCompilerInstruction_create(context, TUUVM_OPCODE_MAKE_CLOSURE_WITH_CAPTURES + (captureCount & 0xF), operands);
    tuuvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_move(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t destination, tuuvm_tuple_t value)
{
    tuuvm_tuple_t operands = tuuvm_array_create(context, 2);
    tuuvm_array_atPut(operands, 0, destination);
    tuuvm_array_atPut(operands, 1, value);

    tuuvm_tuple_t instruction = tuuvm_bytecodeCompilerInstruction_create(context, TUUVM_OPCODE_MOVE, operands);
    tuuvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeCompiler_return(tuuvm_context_t *context, tuuvm_tuple_t compiler, tuuvm_tuple_t value)
{
    tuuvm_tuple_t operands = tuuvm_array_create(context, 1);
    tuuvm_array_atPut(operands, 0, value);

    tuuvm_tuple_t instruction = tuuvm_bytecodeCompilerInstruction_create(context, TUUVM_OPCODE_RETURN, operands);
    tuuvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

static size_t tuuvm_bytecodeCompilerInstruction_computeAssembledSize(tuuvm_bytecodeCompilerInstruction_t *instruction)
{
    // Is this a label?
    if(!instruction->operands || !instruction->opcode)
        return 0;

    return 1 + tuuvm_array_getSize(instruction->operands) * 2;
}

static size_t tuuvm_bytecodeCompilerInstructionOperandFor_assembleInto(tuuvm_context_t *context, tuuvm_tuple_t operand, uint8_t *destination, tuuvm_bytecodeCompilerInstruction_t *instruction)
{
    tuuvm_tuple_t operandType = tuuvm_tuple_getType(context, operand);
    if(operandType == context->roots.bytecodeCompilerInstructionType)
    {
        tuuvm_bytecodeCompilerInstruction_t *destinationInstruction = (tuuvm_bytecodeCompilerInstruction_t*)operand;
        size_t destPC = tuuvm_tuple_size_decode(destinationInstruction->pc);
        size_t sourcePC = tuuvm_tuple_size_decode(instruction->endPC);
        int16_t pcDelta = (int16_t) (destPC - sourcePC);
        *destination++ = pcDelta & 0xFF;
        *destination++ = (pcDelta >> 8) & 0xFF;
        return 2;
    }
    else if(operandType == context->roots.bytecodeCompilerInstructionVectorOperandType)
    {
        tuuvm_bytecodeCompilerInstructionVectorOperand_t *vectorOperand = (tuuvm_bytecodeCompilerInstructionVectorOperand_t*)operand;
        int16_t vectorType = tuuvm_tuple_int16_decode(vectorOperand->vectorType);
        int16_t vectorIndex = tuuvm_tuple_int16_decode(vectorOperand->index);
        int16_t encodedOperand = (vectorIndex << TUUVM_OPERAND_VECTOR_BITS) | vectorType;
        *destination++ = encodedOperand & 0xFF;
        *destination++ = (encodedOperand >> 8) & 0xFF;
        return 2;
    }
    else
    {
        abort();
        return 0;
    }
}

static size_t tuuvm_bytecodeCompilerInstruction_assembleInto(tuuvm_context_t *context, tuuvm_bytecodeCompilerInstruction_t *instruction, uint8_t *destination)
{
    // Is this a label?
    if(!instruction->operands || !instruction->opcode)
        return 0;

    size_t offset = 0;
    destination[offset++] = tuuvm_tuple_uint8_decode(instruction->opcode);

    size_t operandCount = tuuvm_array_getSize(instruction->operands);
    for(size_t i = 0; i < operandCount; ++i)
    {
        tuuvm_tuple_t operand = tuuvm_array_at(instruction->operands, i);
        offset += tuuvm_bytecodeCompilerInstructionOperandFor_assembleInto(context, operand, destination + offset, instruction);
    }

    return offset;
}

TUUVM_API void tuuvm_bytecodeCompiler_compileFunctionDefinition(tuuvm_context_t *context, tuuvm_functionDefinition_t *definition_)
{
    struct {
        tuuvm_functionDefinition_t *definition;
        tuuvm_bytecodeCompiler_t *compiler;
        tuuvm_tuple_t bodyResult;
        tuuvm_functionBytecode_t *bytecode;
        tuuvm_bytecodeCompilerInstruction_t *instruction;
    } gcFrame = {
        .definition = (tuuvm_functionDefinition_t*)definition_,
    };

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.compiler = (tuuvm_bytecodeCompiler_t*)tuuvm_bytecodeCompiler_create(context);
    gcFrame.compiler->sourceEnvironment = gcFrame.definition->analysisEnvironment;
    gcFrame.compiler->sourcePosition = gcFrame.definition->sourcePosition;

    {
        // Make the argument operands.
        size_t argumentCount = tuuvm_array_getSize(gcFrame.definition->analyzedArguments);
        tuuvm_bytecodeCompiler_setArgumentCount(context, (tuuvm_tuple_t)gcFrame.compiler, argumentCount);
        for(size_t i = 0; i < argumentCount; ++i)
        {
            tuuvm_tuple_t binding = tuuvm_array_at(gcFrame.definition->analyzedArguments, i);
            tuuvm_tuple_t value = tuuvm_array_at(gcFrame.compiler->arguments, i);
            tuuvm_bytecodeCompiler_setBindingValue(context, (tuuvm_tuple_t)gcFrame.compiler, binding, value);
        }

        // Make the binding operands.
        size_t captureCount = tuuvm_array_getSize(gcFrame.definition->analyzedCaptures);
        tuuvm_bytecodeCompiler_setCaptureCount(context, (tuuvm_tuple_t)gcFrame.compiler, captureCount);
        for(size_t i = 0; i < captureCount; ++i)
        {
            tuuvm_tuple_t binding = tuuvm_array_at(gcFrame.definition->analyzedCaptures, i);
            tuuvm_tuple_t value = tuuvm_array_at(gcFrame.compiler->captures, i);
            tuuvm_bytecodeCompiler_setBindingValue(context, (tuuvm_tuple_t)gcFrame.compiler, binding, value);
        }
    }

    // Compile the body.
    gcFrame.bodyResult = gcFrame.definition->analyzedBodyNode
        ? tuuvm_bytecodeCompiler_compileASTNode(context, (tuuvm_tuple_t)gcFrame.compiler, gcFrame.definition->analyzedBodyNode)
        : tuuvm_bytecodeCompiler_addLiteral(context, (tuuvm_tuple_t)gcFrame.compiler, TUUVM_VOID_TUPLE);

    if(gcFrame.bodyResult)
        tuuvm_bytecodeCompiler_return(context, (tuuvm_tuple_t)gcFrame.compiler, gcFrame.bodyResult);

    
    // TODO: Optimize the temporaries.
    // TODO: Optimize the jumps onto the next instruction.
    gcFrame.compiler->usedTemporaryCount = tuuvm_tuple_size_encode(context, tuuvm_arrayList_getSize(gcFrame.compiler->temporaries));

    gcFrame.bytecode = (tuuvm_functionBytecode_t*)tuuvm_context_allocatePointerTuple(context, context->roots.functionBytecodeType, TUUVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(tuuvm_functionBytecode_t));
    gcFrame.bytecode->argumentCount = tuuvm_tuple_size_encode(context, tuuvm_array_getSize(gcFrame.compiler->arguments));
    gcFrame.bytecode->captureVectorSize = tuuvm_tuple_size_encode(context, tuuvm_array_getSize(gcFrame.compiler->captures));
    gcFrame.bytecode->localVectorSize = gcFrame.compiler->usedTemporaryCount;
    gcFrame.bytecode->literalVector = tuuvm_arrayList_asArray(context, gcFrame.compiler->literals);

    // Assemble the instructions.
    size_t instructionsOffset = 0;
    for(gcFrame.instruction = gcFrame.compiler->firstInstruction; gcFrame.instruction; gcFrame.instruction = gcFrame.instruction->next)
    {
        gcFrame.instruction->pc = tuuvm_tuple_size_encode(context, instructionsOffset);
        instructionsOffset += tuuvm_bytecodeCompilerInstruction_computeAssembledSize(gcFrame.instruction);
        gcFrame.instruction->endPC = tuuvm_tuple_size_encode(context, instructionsOffset);
    }

    gcFrame.bytecode->instructions = tuuvm_byteArray_create(context, instructionsOffset);
    instructionsOffset = 0;
    uint8_t *destInstructions = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(gcFrame.bytecode->instructions)->bytes;
    for(gcFrame.instruction = gcFrame.compiler->firstInstruction; gcFrame.instruction; gcFrame.instruction = gcFrame.instruction->next)
        instructionsOffset += tuuvm_bytecodeCompilerInstruction_assembleInto(context, gcFrame.instruction, destInstructions + instructionsOffset);

    // Finish by installing it on the definition.
    gcFrame.definition->bytecode = (tuuvm_tuple_t)gcFrame.bytecode;
}

static tuuvm_tuple_t tuuvm_astCoerceValueNode_primitiveCompileIntoBytecode(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *compiler = &arguments[1];

    tuuvm_astCoerceValueNode_t **coerceValueNode = (tuuvm_astCoerceValueNode_t**)node;
    struct {
        tuuvm_tuple_t typeOperand;
        tuuvm_tuple_t valueOperand;
        tuuvm_tuple_t result;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.typeOperand = tuuvm_bytecodeCompiler_compileASTNode(context, *compiler, (*coerceValueNode)->typeExpression);
    gcFrame.valueOperand = tuuvm_bytecodeCompiler_compileASTNode(context, *compiler, (*coerceValueNode)->valueExpression);
    gcFrame.result = tuuvm_bytecodeCompiler_newTemporary(context, *compiler);
    tuuvm_bytecodeCompiler_coerceValue(context, *compiler, gcFrame.result, gcFrame.typeOperand, gcFrame.valueOperand);

    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_astFunctionApplicationNode_primitiveCompileIntoBytecode(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *compiler = &arguments[1];

    tuuvm_astFunctionApplicationNode_t **applicationNode = (tuuvm_astFunctionApplicationNode_t**)node;
    struct {
        tuuvm_tuple_t function;
        tuuvm_tuple_t arguments;
        tuuvm_tuple_t argumentNode;
        tuuvm_tuple_t argumentOperand;
        tuuvm_tuple_t result;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    size_t applicationArgumentCount = tuuvm_array_getSize((*applicationNode)->arguments);

    // TODO: Inline some special functions here.

    gcFrame.function = tuuvm_bytecodeCompiler_compileASTNode(context, *compiler, (*applicationNode)->functionExpression);
    gcFrame.arguments = tuuvm_array_create(context, applicationArgumentCount);

    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        gcFrame.argumentNode = tuuvm_array_at((*applicationNode)->arguments, i);
        gcFrame.argumentOperand = tuuvm_bytecodeCompiler_compileASTNode(context, *compiler, gcFrame.argumentNode);
        tuuvm_array_atPut(gcFrame.arguments, i, gcFrame.argumentOperand);
    }

    gcFrame.result = tuuvm_bytecodeCompiler_newTemporary(context, *compiler);

    tuuvm_bitflags_t applicationFlags = tuuvm_tuple_bitflags_decode((*applicationNode)->applicationFlags);
    bool isNotypecheck = (applicationFlags & TUUVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK) != 0;

    if(isNotypecheck)
        tuuvm_bytecodeCompiler_uncheckedCall(context, *compiler, gcFrame.result, gcFrame.function, gcFrame.arguments);
    else
        tuuvm_bytecodeCompiler_call(context, *compiler, gcFrame.result, gcFrame.function, gcFrame.arguments);

    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_astIdentifierReferenceNode_primitiveCompileIntoBytecode(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *compiler = &arguments[1];

    tuuvm_astIdentifierReferenceNode_t **referenceNode = (tuuvm_astIdentifierReferenceNode_t**)node;
    return tuuvm_bytecodeCompiler_getBindingValue(context, *compiler, (*referenceNode)->binding);
}

static tuuvm_tuple_t tuuvm_astIfNode_primitiveCompileIntoBytecode(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *compiler = &arguments[1];

    tuuvm_astIfNode_t **ifNode = (tuuvm_astIfNode_t**)node;
    struct {
        tuuvm_tuple_t falseLabel;
        tuuvm_tuple_t mergeLabel;
        tuuvm_tuple_t trueResult;
        tuuvm_tuple_t falseResult;
        tuuvm_tuple_t result;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.falseLabel = tuuvm_bytecodeCompilerInstruction_createLabel(context);
    gcFrame.mergeLabel = tuuvm_bytecodeCompilerInstruction_createLabel(context);

    // Emit the condition
    {
        tuuvm_tuple_t condition = tuuvm_bytecodeCompiler_compileASTNode(context, *compiler, (*ifNode)->conditionExpression);
        tuuvm_bytecodeCompiler_jumpIfFalse(context, *compiler, condition, gcFrame.falseLabel);
    }

    gcFrame.result = tuuvm_bytecodeCompiler_newTemporary(context, *compiler);

    // True branch.
    if((*ifNode)->trueExpression)
        gcFrame.trueResult = tuuvm_bytecodeCompiler_compileASTNode(context, *compiler, (*ifNode)->trueExpression);
    else
        gcFrame.trueResult = tuuvm_bytecodeCompiler_addLiteral(context, *compiler, TUUVM_NULL_TUPLE);
    tuuvm_bytecodeCompiler_move(context, *compiler, gcFrame.result, gcFrame.trueResult);
    tuuvm_bytecodeCompiler_jump(context, *compiler, gcFrame.mergeLabel);

    // False branch.
    tuuvm_bytecodeCompiler_addInstruction(*compiler, gcFrame.falseLabel);
    if((*ifNode)->falseExpression)
        gcFrame.falseResult = tuuvm_bytecodeCompiler_compileASTNode(context, *compiler, (*ifNode)->falseExpression);
    else
        gcFrame.falseResult = tuuvm_bytecodeCompiler_addLiteral(context, *compiler, TUUVM_NULL_TUPLE);
    tuuvm_bytecodeCompiler_move(context, *compiler, gcFrame.result, gcFrame.falseResult);

    tuuvm_bytecodeCompiler_addInstruction(*compiler, gcFrame.mergeLabel);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_astLambdaNode_primitiveCompileIntoBytecode(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *compiler = &arguments[1];

    tuuvm_astLambdaNode_t **lambdaNode = (tuuvm_astLambdaNode_t**)node;

    tuuvm_functionDefinition_t *functionDefinition = (tuuvm_functionDefinition_t*)(*lambdaNode)->functionDefinition;
    tuuvm_tuple_t functionDefinitionOperand = tuuvm_bytecodeCompiler_addLiteral(context, *compiler, (tuuvm_tuple_t)functionDefinition);
    size_t captureVectorSize = tuuvm_array_getSize(functionDefinition->analyzedCaptures);
    tuuvm_tuple_t captureVector = tuuvm_array_create(context, captureVectorSize);
    
    for(size_t i = 0; i < captureVectorSize; ++i)
    {
        tuuvm_tuple_t captureBinding = tuuvm_symbolCaptureBinding_getSourceBinding(tuuvm_array_at(functionDefinition->analyzedCaptures, i));
        tuuvm_tuple_t captureValue = tuuvm_bytecodeCompiler_getBindingValue(context, *compiler, captureBinding);

        tuuvm_array_atPut(captureVector, i, captureValue);
    }

    tuuvm_tuple_t result = tuuvm_bytecodeCompiler_newTemporary(context, *compiler);
    tuuvm_bytecodeCompiler_makeClosureWithCaptures(context, *compiler, result, functionDefinitionOperand, captureVector);
    return result;
}

static tuuvm_tuple_t tuuvm_astLexicalBlockNode_primitiveCompileIntoBytecode(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *compiler = &arguments[1];

    tuuvm_astLexicalBlockNode_t **lexicalBlockNode = (tuuvm_astLexicalBlockNode_t**)node;

    return tuuvm_bytecodeCompiler_compileASTNodeWithEnvironment(context, *compiler, (*lexicalBlockNode)->body, (*lexicalBlockNode)->bodyEnvironment);
}

static tuuvm_tuple_t tuuvm_astLiteralNode_primitiveCompileIntoBytecode(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *compiler = &arguments[1];

    tuuvm_astLiteralNode_t **literalNode = (tuuvm_astLiteralNode_t**)node;
    return tuuvm_bytecodeCompiler_addLiteral(context, *compiler, (*literalNode)->value);
}

static tuuvm_tuple_t tuuvm_astLocalDefinitionNode_primitiveCompileIntoBytecode(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *compiler = &arguments[1];

    tuuvm_astLocalDefinitionNode_t **localDefinitionNode = (tuuvm_astLocalDefinitionNode_t**)node;

    tuuvm_tuple_t value = (*localDefinitionNode)->valueExpression
        ? tuuvm_bytecodeCompiler_compileASTNode(context, *compiler, (*localDefinitionNode)->valueExpression)
        : tuuvm_bytecodeCompiler_addLiteral(context, *compiler, TUUVM_NULL_TUPLE);

    bool isMutable = tuuvm_tuple_boolean_decode((*localDefinitionNode)->isMutable);
    if(isMutable)
    {
        tuuvm_tuple_t result = tuuvm_bytecodeCompiler_newTemporary(context, *compiler);
        tuuvm_bytecodeCompiler_allocaWithValue(context, *compiler, result,
            tuuvm_bytecodeCompiler_addLiteral(context, *compiler, (*localDefinitionNode)->super.analyzedType),
            value);
        value = result;
    }

    tuuvm_bytecodeCompiler_setBindingValue(context, *compiler, (*localDefinitionNode)->binding, value);
    return value;
}

static tuuvm_tuple_t tuuvm_astMakeAssociationNode_primitiveCompileIntoBytecode(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *compiler = &arguments[1];

    tuuvm_astMakeAssociationNode_t **associationNode = (tuuvm_astMakeAssociationNode_t**)node;
    struct {
        tuuvm_tuple_t key;
        tuuvm_tuple_t value;
        tuuvm_tuple_t result;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.key = tuuvm_bytecodeCompiler_compileASTNode(context, *compiler, (*associationNode)->key);
    if((*associationNode)->value)
        gcFrame.value = tuuvm_bytecodeCompiler_compileASTNode(context, *compiler, (*associationNode)->value);
    else
        gcFrame.value = tuuvm_bytecodeCompiler_addLiteral(context, *compiler, TUUVM_NULL_TUPLE);

    gcFrame.result = tuuvm_bytecodeCompiler_newTemporary(context, *compiler);
    tuuvm_bytecodeCompiler_makeAssociation(context, *compiler, gcFrame.result, gcFrame.key, gcFrame.value);

    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_astMakeArrayNode_primitiveCompileIntoBytecode(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *compiler = &arguments[1];

    tuuvm_astMakeArrayNode_t **arrayNode = (tuuvm_astMakeArrayNode_t**)node;
    struct {
        tuuvm_tuple_t elements;
        tuuvm_tuple_t element;
        tuuvm_tuple_t result;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    size_t arrayElementCountCount = tuuvm_array_getSize((*arrayNode)->elements);
    gcFrame.elements = tuuvm_array_create(context, arrayElementCountCount);
    for(size_t i = 0; i < arrayElementCountCount; ++i)
    {
        gcFrame.element = tuuvm_bytecodeCompiler_compileASTNode(context, *compiler, tuuvm_array_at((*arrayNode)->elements, i));
        tuuvm_array_atPut(gcFrame.elements, i, gcFrame.element);
    }

    gcFrame.result = tuuvm_bytecodeCompiler_newTemporary(context, *compiler);
    tuuvm_bytecodeCompiler_makeArrayWithElements(context, *compiler, gcFrame.result, gcFrame.elements);

    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_astMakeByteArrayNode_primitiveCompileIntoBytecode(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *compiler = &arguments[1];

    tuuvm_astMakeByteArrayNode_t **arrayNode = (tuuvm_astMakeByteArrayNode_t**)node;
    struct {
        tuuvm_tuple_t elements;
        tuuvm_tuple_t element;
        tuuvm_tuple_t result;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    size_t arrayElementCountCount = tuuvm_array_getSize((*arrayNode)->elements);
    gcFrame.elements = tuuvm_array_create(context, arrayElementCountCount);
    for(size_t i = 0; i < arrayElementCountCount; ++i)
    {
        gcFrame.element = tuuvm_bytecodeCompiler_compileASTNode(context, *compiler, tuuvm_array_at((*arrayNode)->elements, i));
        tuuvm_array_atPut(gcFrame.elements, i, gcFrame.element);
    }

    gcFrame.result = tuuvm_bytecodeCompiler_newTemporary(context, *compiler);
    tuuvm_bytecodeCompiler_makeByteArrayWithElements(context, *compiler, gcFrame.result, gcFrame.elements);

    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_astMakeDictionaryNode_primitiveCompileIntoBytecode(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *compiler = &arguments[1];

    tuuvm_astMakeDictionaryNode_t **dictionaryNode = (tuuvm_astMakeDictionaryNode_t**)node;
    struct {
        tuuvm_tuple_t elements;
        tuuvm_tuple_t element;
        tuuvm_tuple_t result;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    size_t arrayElementCountCount = tuuvm_array_getSize((*dictionaryNode)->elements);
    gcFrame.elements = tuuvm_array_create(context, arrayElementCountCount);
    for(size_t i = 0; i < arrayElementCountCount; ++i)
    {
        gcFrame.element = tuuvm_bytecodeCompiler_compileASTNode(context, *compiler, tuuvm_array_at((*dictionaryNode)->elements, i));
        tuuvm_array_atPut(gcFrame.elements, i, gcFrame.element);
    }

    gcFrame.result = tuuvm_bytecodeCompiler_newTemporary(context, *compiler);
    tuuvm_bytecodeCompiler_makeDictionaryWithElements(context, *compiler, gcFrame.result, gcFrame.elements);

    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_astMessageSendNode_primitiveCompileIntoBytecode(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *compiler = &arguments[1];

    tuuvm_astMessageSendNode_t **sendNode = (tuuvm_astMessageSendNode_t**)node;
    struct {
        tuuvm_tuple_t receiver;
        tuuvm_tuple_t receiverLookupType;
        tuuvm_tuple_t selector;
        tuuvm_tuple_t arguments;
        tuuvm_tuple_t argument;
        tuuvm_tuple_t result;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.receiver = tuuvm_bytecodeCompiler_compileASTNode(context, *compiler, (*sendNode)->receiver);
    if((*sendNode)->receiverLookupType)
        gcFrame.receiverLookupType = tuuvm_bytecodeCompiler_compileASTNode(context, *compiler, (*sendNode)->receiverLookupType);
    gcFrame.selector = tuuvm_bytecodeCompiler_compileASTNode(context, *compiler, (*sendNode)->selector);

    size_t sendArgumentCount = tuuvm_array_getSize((*sendNode)->arguments);
    gcFrame.arguments = tuuvm_array_create(context, sendArgumentCount);
    for(size_t i = 0; i < sendArgumentCount; ++i)
    {
        gcFrame.argument = tuuvm_bytecodeCompiler_compileASTNode(context, *compiler, tuuvm_array_at((*sendNode)->arguments, i));
        tuuvm_array_atPut(gcFrame.arguments, i, gcFrame.argument);
    }

    gcFrame.result = tuuvm_bytecodeCompiler_newTemporary(context, *compiler);
    if((*sendNode)->receiverLookupType)
        tuuvm_bytecodeCompiler_sendWithLookupReceiverType(context, *compiler, gcFrame.result, gcFrame.receiverLookupType, gcFrame.selector, gcFrame.receiver, gcFrame.arguments);
    else
        tuuvm_bytecodeCompiler_send(context, *compiler, gcFrame.result, gcFrame.selector, gcFrame.receiver, gcFrame.arguments);

    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_astSequenceNode_primitiveCompileIntoBytecode(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *compiler = &arguments[1];

    tuuvm_astSequenceNode_t **sequenceNode = (tuuvm_astSequenceNode_t**)node;
    struct {
        tuuvm_tuple_t expressionNode;
        tuuvm_tuple_t result;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    size_t expressionCount = tuuvm_array_getSize((*sequenceNode)->expressions);
    gcFrame.result = TUUVM_NULL_TUPLE;
    if(expressionCount == 0)
        gcFrame.result = tuuvm_bytecodeCompiler_addLiteral(context, *compiler, TUUVM_VOID_TUPLE);

    for(size_t i = 0; i < expressionCount; ++i)
    {
        gcFrame.expressionNode = tuuvm_array_at((*sequenceNode)->expressions, i);
        gcFrame.result = tuuvm_bytecodeCompiler_compileASTNode(context, *compiler, gcFrame.expressionNode);
    }

    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static tuuvm_tuple_t tuuvm_astWhileContinueNode_primitiveCompileIntoBytecode(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    tuuvm_tuple_t *node = &arguments[0];
    tuuvm_tuple_t *compiler = &arguments[1];

    tuuvm_astWhileContinueWithNode_t **whileNode = (tuuvm_astWhileContinueWithNode_t**)node;
    struct {
        tuuvm_tuple_t whileEntryLabel;
        tuuvm_tuple_t whileBodyLabel;
        tuuvm_tuple_t whileContinue;
        tuuvm_tuple_t whileMergeLabel;
        tuuvm_tuple_t result;
    } gcFrame = {0};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.whileEntryLabel = tuuvm_bytecodeCompilerInstruction_createLabel(context);
    gcFrame.whileBodyLabel = tuuvm_bytecodeCompilerInstruction_createLabel(context);
    gcFrame.whileContinue = tuuvm_bytecodeCompilerInstruction_createLabel(context);
    gcFrame.whileMergeLabel = tuuvm_bytecodeCompilerInstruction_createLabel(context);

    // While condition block.
    tuuvm_bytecodeCompiler_addInstruction(*compiler, gcFrame.whileEntryLabel);
    if((*whileNode)->conditionExpression)
    {
        tuuvm_tuple_t condition = tuuvm_bytecodeCompiler_compileASTNode(context, *compiler, (*whileNode)->conditionExpression);
        tuuvm_bytecodeCompiler_jumpIfFalse(context, *compiler, condition, gcFrame.whileMergeLabel);
    }
    else
    {
        tuuvm_bytecodeCompiler_jump(context, *compiler, gcFrame.whileBodyLabel);
    }

    // While body.
    tuuvm_bytecodeCompiler_addInstruction(*compiler, gcFrame.whileBodyLabel);
    if((*whileNode)->bodyExpression)
        tuuvm_bytecodeCompiler_compileASTNodeWithBreakAndContinue(context, *compiler, (*whileNode)->bodyExpression, gcFrame.whileMergeLabel, gcFrame.whileContinue);
    tuuvm_bytecodeCompiler_jump(context, *compiler, gcFrame.whileContinue);

    // While continue
    tuuvm_bytecodeCompiler_addInstruction(*compiler, gcFrame.whileContinue);
    if((*whileNode)->continueExpression)
        tuuvm_bytecodeCompiler_compileASTNode(context, *compiler, (*whileNode)->continueExpression);

    tuuvm_bytecodeCompiler_jump(context, *compiler, gcFrame.whileEntryLabel);

    // While merge
    tuuvm_bytecodeCompiler_addInstruction(*compiler, gcFrame.whileMergeLabel);

    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return tuuvm_bytecodeCompiler_addLiteral(context, *compiler, TUUVM_VOID_TUPLE);
}

static void tuuvm_bytecodeCompiler_setupNodeCompilationFunction(tuuvm_context_t *context, tuuvm_tuple_t astNodeType, tuuvm_functionEntryPoint_t compilationFunction)
{
    tuuvm_type_setMethodWithSelector(context, astNodeType, context->roots.astNodeCompileIntoBytecodeSelector, tuuvm_function_createPrimitive(context, 2, TUUVM_FUNCTION_FLAGS_NONE, NULL, compilationFunction));
}

void tuuvm_bytecodeCompiler_registerPrimitives(void)
{
    tuuvm_primitiveTable_registerFunction(tuuvm_astCoerceValueNode_primitiveCompileIntoBytecode, "ASTCoerceValueNode::compileIntoBytecodeWith:");
    tuuvm_primitiveTable_registerFunction(tuuvm_astFunctionApplicationNode_primitiveCompileIntoBytecode, "ASTFunctionApplicationNode::compileIntoBytecodeWith:");
    tuuvm_primitiveTable_registerFunction(tuuvm_astIdentifierReferenceNode_primitiveCompileIntoBytecode, "ASTIdentifierReferenceNode::compileIntoBytecodeWith:");
    tuuvm_primitiveTable_registerFunction(tuuvm_astIfNode_primitiveCompileIntoBytecode, "ASTIfNode::compileIntoBytecodeWith:");
    tuuvm_primitiveTable_registerFunction(tuuvm_astLambdaNode_primitiveCompileIntoBytecode, "ASTLambdaNode::compileIntoBytecodeWith:");
    tuuvm_primitiveTable_registerFunction(tuuvm_astLexicalBlockNode_primitiveCompileIntoBytecode, "ASTLexicalBlockNode::compileIntoBytecodeWith:");
    tuuvm_primitiveTable_registerFunction(tuuvm_astLiteralNode_primitiveCompileIntoBytecode, "ASTLiteralNode::compileIntoBytecodeWith:");
    tuuvm_primitiveTable_registerFunction(tuuvm_astLocalDefinitionNode_primitiveCompileIntoBytecode, "ASTLocalDefinitionNode::compileIntoBytecodeWith:");
    tuuvm_primitiveTable_registerFunction(tuuvm_astMakeArrayNode_primitiveCompileIntoBytecode, "ASTMakeArrayNode::compileIntoBytecodeWith:");
    tuuvm_primitiveTable_registerFunction(tuuvm_astMakeAssociationNode_primitiveCompileIntoBytecode, "ASTMakeAssociationNode::compileIntoBytecodeWith:");
    tuuvm_primitiveTable_registerFunction(tuuvm_astMakeByteArrayNode_primitiveCompileIntoBytecode, "ASTMakeByteArrayNode::compileIntoBytecodeWith:");
    tuuvm_primitiveTable_registerFunction(tuuvm_astMakeDictionaryNode_primitiveCompileIntoBytecode, "ASTMakeDictionaryNode::compileIntoBytecodeWith:");
    tuuvm_primitiveTable_registerFunction(tuuvm_astMessageSendNode_primitiveCompileIntoBytecode, "ASTMessageSendNode::compileIntoBytecodeWith:");
    tuuvm_primitiveTable_registerFunction(tuuvm_astSequenceNode_primitiveCompileIntoBytecode, "ASTSequenceNode::compileIntoBytecodeWith:");
    tuuvm_primitiveTable_registerFunction(tuuvm_astWhileContinueNode_primitiveCompileIntoBytecode, "ASTWhileNodeNode::compileIntoBytecodeWith:");
}

void tuuvm_bytecodeCompiler_setupPrimitives(tuuvm_context_t *context)
{
    tuuvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astCoerceValueNodeType, &tuuvm_astCoerceValueNode_primitiveCompileIntoBytecode);
    tuuvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astFunctionApplicationNodeType, &tuuvm_astFunctionApplicationNode_primitiveCompileIntoBytecode);
    tuuvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astIdentifierReferenceNodeType, &tuuvm_astIdentifierReferenceNode_primitiveCompileIntoBytecode);
    tuuvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astIfNodeType, &tuuvm_astIfNode_primitiveCompileIntoBytecode);
    tuuvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astLambdaNodeType, &tuuvm_astLambdaNode_primitiveCompileIntoBytecode);
    tuuvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astLexicalBlockNodeType, &tuuvm_astLexicalBlockNode_primitiveCompileIntoBytecode);
    tuuvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astLiteralNodeType, &tuuvm_astLiteralNode_primitiveCompileIntoBytecode);
    tuuvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astLocalDefinitionNodeType, &tuuvm_astLocalDefinitionNode_primitiveCompileIntoBytecode);
    tuuvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astMakeArrayNodeType, &tuuvm_astMakeArrayNode_primitiveCompileIntoBytecode);
    tuuvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astMakeAssociationNodeType, &tuuvm_astMakeAssociationNode_primitiveCompileIntoBytecode);
    tuuvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astMakeByteArrayNodeType, &tuuvm_astMakeByteArrayNode_primitiveCompileIntoBytecode);
    tuuvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astMakeDictionaryNodeType, &tuuvm_astMakeDictionaryNode_primitiveCompileIntoBytecode);
    tuuvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astMessageSendNodeType, &tuuvm_astMessageSendNode_primitiveCompileIntoBytecode);
    tuuvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astSequenceNodeType, &tuuvm_astSequenceNode_primitiveCompileIntoBytecode);
    tuuvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astWhileContinueWithNodeType, &tuuvm_astWhileContinueNode_primitiveCompileIntoBytecode);
}
