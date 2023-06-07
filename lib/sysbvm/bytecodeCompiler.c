#include "sysbvm/bytecodeCompiler.h"
#include "sysbvm/array.h"
#include "sysbvm/orderedCollection.h"
#include "sysbvm/ast.h"
#include "sysbvm/dictionary.h"
#include "sysbvm/environment.h"
#include "sysbvm/errors.h"
#include "sysbvm/function.h"
#include "sysbvm/stackFrame.h"
#include "internal/context.h"
#include <stdlib.h>

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompilerInstruction_create(sysbvm_context_t *context, uint8_t opcode, sysbvm_tuple_t operands)
{
    sysbvm_bytecodeCompilerInstruction_t *result = (sysbvm_bytecodeCompilerInstruction_t*)sysbvm_context_allocatePointerTuple(context, context->roots.bytecodeCompilerInstructionType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_bytecodeCompilerInstruction_t));
    result->opcode = sysbvm_tuple_uint8_encode(opcode);
    result->operands = operands;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompilerInstruction_createLabel(sysbvm_context_t *context)
{
    sysbvm_bytecodeCompilerInstruction_t *result = (sysbvm_bytecodeCompilerInstruction_t*)sysbvm_context_allocatePointerTuple(context, context->roots.bytecodeCompilerInstructionType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_bytecodeCompilerInstruction_t));
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompilerInstructionVectorOperand_create(sysbvm_context_t *context, sysbvm_operandVectorName_t vectorName, int16_t vectorIndex)
{
    sysbvm_bytecodeCompilerInstructionVectorOperand_t *result = (sysbvm_bytecodeCompilerInstructionVectorOperand_t*)sysbvm_context_allocatePointerTuple(context, context->roots.bytecodeCompilerInstructionVectorOperandType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_bytecodeCompilerInstructionVectorOperand_t));
    result->vectorType = sysbvm_tuple_int16_encode(vectorName);
    result->index = sysbvm_tuple_int16_encode(vectorIndex);
    result->hasAllocaDestination = SYSBVM_FALSE_TUPLE;
    result->hasNonAllocaDestination = SYSBVM_FALSE_TUPLE;
    result->hasSlotReferenceAtDestination = SYSBVM_FALSE_TUPLE;
    result->hasNonSlotReferenceAtDestination = SYSBVM_FALSE_TUPLE;
    result->hasLoadStoreUsage = SYSBVM_FALSE_TUPLE;
    result->hasNonLoadStoreUsage = SYSBVM_FALSE_TUPLE;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_create(sysbvm_context_t *context)
{
    sysbvm_bytecodeCompiler_t *result = (sysbvm_bytecodeCompiler_t*)sysbvm_context_allocatePointerTuple(context, context->roots.bytecodeCompilerType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_bytecodeCompiler_t));
    result->arguments = sysbvm_array_create(context, 0);
    result->captures = sysbvm_array_create(context, 0);
    result->literals = sysbvm_orderedCollection_create(context);
    result->literalDictionary = sysbvm_identityDictionary_create(context);
    result->temporaries = sysbvm_orderedCollection_create(context);
    result->usedTemporaryCount = sysbvm_tuple_size_encode(context, 0);
    result->bindingDictionary = sysbvm_identityDictionary_create(context);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API void sysbvm_bytecodeCompiler_setArgumentCount(sysbvm_context_t *context, sysbvm_tuple_t compiler, size_t argumentCount)
{
    sysbvm_tuple_t arguments = sysbvm_array_create(context, argumentCount);
    for(size_t i = 0; i < argumentCount; ++i )
    {
        sysbvm_tuple_t argumentOperand = sysbvm_bytecodeCompilerInstructionVectorOperand_create(context, SYSBVM_OPERAND_VECTOR_ARGUMENTS, (int16_t)i);
        sysbvm_array_atPut(arguments, i, argumentOperand);
    }

    ((sysbvm_bytecodeCompiler_t*)compiler)->arguments = arguments;
}

SYSBVM_API void sysbvm_bytecodeCompiler_setCaptureCount(sysbvm_context_t *context, sysbvm_tuple_t compiler, size_t argumentCount)
{
    sysbvm_tuple_t captures = sysbvm_array_create(context, argumentCount);
    for(size_t i = 0; i < argumentCount; ++i )
    {
        sysbvm_tuple_t captureOperand = sysbvm_bytecodeCompilerInstructionVectorOperand_create(context, SYSBVM_OPERAND_VECTOR_CAPTURES, (int16_t)i);
        sysbvm_array_atPut(captures, i, captureOperand);
    }

    ((sysbvm_bytecodeCompiler_t*)compiler)->captures = captures;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_addLiteral(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t literalValue)
{
    sysbvm_bytecodeCompiler_t *compilerObject = (sysbvm_bytecodeCompiler_t*)compiler;
    sysbvm_tuple_t existent = SYSBVM_NULL_TUPLE;
    if(sysbvm_identityDictionary_find(compilerObject->literalDictionary, literalValue, &existent))
        return existent;

    size_t literalIndex = sysbvm_orderedCollection_getSize(compilerObject->literals);
    sysbvm_tuple_t newLiteralOperand = sysbvm_bytecodeCompilerInstructionVectorOperand_create(context, SYSBVM_OPERAND_VECTOR_LITERAL, (int16_t)literalIndex);
    sysbvm_orderedCollection_add(context, compilerObject->literals, literalValue);
    sysbvm_identityDictionary_atPut(context, compilerObject->literalDictionary, literalValue, newLiteralOperand);
    return newLiteralOperand;
}

SYSBVM_API void sysbvm_bytecodeCompiler_addInstruction(sysbvm_tuple_t compiler, sysbvm_tuple_t instruction)
{
    sysbvm_bytecodeCompiler_t *compilerObject = (sysbvm_bytecodeCompiler_t*)compiler;
    sysbvm_bytecodeCompilerInstruction_t *instructionObject = (sysbvm_bytecodeCompilerInstruction_t*)instruction;

    instructionObject->previous = (sysbvm_bytecodeCompilerInstruction_t*)compilerObject->lastInstruction;

    if(compilerObject->lastInstruction)
    {
        ((sysbvm_bytecodeCompilerInstruction_t*)compilerObject->lastInstruction)->next = instructionObject;
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

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_compileASTNode(sysbvm_context_t *context, sysbvm_tuple_t compiler_, sysbvm_tuple_t astNode_)
{
    struct {
        sysbvm_bytecodeCompiler_t *compilerObject;
        sysbvm_tuple_t astNode;
        sysbvm_tuple_t result;

        sysbvm_tuple_t oldSourcePosition;
        sysbvm_tuple_t oldSourceASTNode;
    } gcFrame = {
        .compilerObject = (sysbvm_bytecodeCompiler_t*)compiler_,
        .astNode = astNode_
    };

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.oldSourcePosition = gcFrame.compilerObject->sourcePosition;
    gcFrame.oldSourceASTNode = gcFrame.compilerObject->sourceASTNode;

    gcFrame.compilerObject->sourcePosition = sysbvm_astNode_getSourcePosition(gcFrame.astNode);
    gcFrame.compilerObject->sourceASTNode = gcFrame.astNode;

    gcFrame.result = sysbvm_tuple_send1(context, context->roots.astNodeCompileIntoBytecodeSelector, gcFrame.astNode, (sysbvm_tuple_t)gcFrame.compilerObject);

    gcFrame.compilerObject->sourcePosition = gcFrame.oldSourcePosition;
    gcFrame.compilerObject->sourceASTNode = gcFrame.oldSourceASTNode;

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_compileASTNodeWithBreakAndContinue(sysbvm_context_t *context, sysbvm_tuple_t compiler_, sysbvm_tuple_t astNode, sysbvm_tuple_t breakLabel, sysbvm_tuple_t continueLabel)
{
    struct {
        sysbvm_bytecodeCompiler_t *compilerObject;
        sysbvm_tuple_t oldBreakLabel;
        sysbvm_tuple_t oldContinueLabel;
        sysbvm_tuple_t result;
    } gcFrame = {
        .compilerObject = (sysbvm_bytecodeCompiler_t*)compiler_,
    };

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.oldBreakLabel = gcFrame.compilerObject->breakLabel;
    gcFrame.oldContinueLabel = gcFrame.compilerObject->continueLabel;
    gcFrame.compilerObject->breakLabel = breakLabel;
    gcFrame.compilerObject->continueLabel = continueLabel;

    gcFrame.result = sysbvm_bytecodeCompiler_compileASTNode(context, (sysbvm_tuple_t)gcFrame.compilerObject, astNode);

    gcFrame.compilerObject->breakLabel = gcFrame.oldBreakLabel;
    gcFrame.compilerObject->continueLabel = gcFrame.oldContinueLabel;

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_compileASTNodeWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t compiler_, sysbvm_tuple_t astNode, sysbvm_tuple_t environment)
{
    struct {
        sysbvm_bytecodeCompiler_t *compilerObject;
        sysbvm_tuple_t oldEnvironment;
        sysbvm_tuple_t result;
    } gcFrame = {
        .compilerObject = (sysbvm_bytecodeCompiler_t*)compiler_,
    };

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.oldEnvironment = gcFrame.compilerObject->sourceEnvironment;
    gcFrame.compilerObject->sourceEnvironment = environment;

    gcFrame.result = sysbvm_bytecodeCompiler_compileASTNode(context, (sysbvm_tuple_t)gcFrame.compilerObject, astNode);

    gcFrame.compilerObject->sourceEnvironment = gcFrame.oldEnvironment;

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

SYSBVM_API void sysbvm_bytecodeCompiler_setBindingValue(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t binding, sysbvm_tuple_t value)
{
    sysbvm_bytecodeCompiler_t *compilerObject = (sysbvm_bytecodeCompiler_t*)compiler;
    sysbvm_identityDictionary_atPut(context, compilerObject->bindingDictionary, binding, value);
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_getBindingValue(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t binding)
{
    (void)context;
    if(sysbvm_tuple_isKindOf(context, binding, context->roots.symbolTupleSlotBindingType))
    {
        sysbvm_symbolTupleSlotBinding_t *bindingObject = (sysbvm_symbolTupleSlotBinding_t*)binding;
        sysbvm_tuple_t tupleValue = sysbvm_bytecodeCompiler_getBindingValue(context, compiler, bindingObject->tupleBinding);
        sysbvm_tuple_t reference = sysbvm_bytecodeCompiler_newTemporary(context, compiler);

        sysbvm_symbolBinding_t *tupleBindingObject = (sysbvm_symbolBinding_t*)bindingObject->tupleBinding;

        if(sysbvm_type_isPointerLikeType(tupleBindingObject->type))
            sysbvm_bytecodeCompiler_refSlotReferenceAt(context, compiler, reference, tupleValue, sysbvm_bytecodeCompiler_addLiteral(context, compiler, bindingObject->typeSlot));
        else
            sysbvm_bytecodeCompiler_slotReferenceAt(context, compiler, reference, tupleValue, sysbvm_bytecodeCompiler_addLiteral(context, compiler, bindingObject->typeSlot));
        return reference;
    }

    sysbvm_tuple_t value = SYSBVM_NULL_TUPLE;
    sysbvm_bytecodeCompiler_t *compilerObject = (sysbvm_bytecodeCompiler_t*)compiler;
    if(!sysbvm_identityDictionary_find(compilerObject->bindingDictionary, binding, &value))
        sysbvm_error("Invalid value binding.");

    return value;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_newTemporary(sysbvm_context_t *context, sysbvm_tuple_t compiler)
{
    sysbvm_bytecodeCompiler_t *compilerObject = (sysbvm_bytecodeCompiler_t*)compiler;
    size_t temporaryIndex = sysbvm_orderedCollection_getSize(compilerObject->temporaries);
    sysbvm_tuple_t temporaryOperand = sysbvm_bytecodeCompilerInstructionVectorOperand_create(context, SYSBVM_OPERAND_VECTOR_LOCAL, (int16_t)temporaryIndex);
    sysbvm_orderedCollection_add(context, compilerObject->temporaries, temporaryOperand);
    return temporaryOperand;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_allocaWithValue(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t result, sysbvm_tuple_t pointerLikeType, sysbvm_tuple_t value)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 3);
    sysbvm_array_atPut(operands, 0, result);
    sysbvm_array_atPut(operands, 1, pointerLikeType);
    sysbvm_array_atPut(operands, 2, value);

    sysbvm_tuple_t instruction = sysbvm_bytecodeCompilerInstruction_create(context, SYSBVM_OPCODE_ALLOCA_WITH_VALUE, operands);
    sysbvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_coerceValue(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t result, sysbvm_tuple_t type, sysbvm_tuple_t value)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 3);
    sysbvm_array_atPut(operands, 0, result);
    sysbvm_array_atPut(operands, 1, type);
    sysbvm_array_atPut(operands, 2, value);

    sysbvm_tuple_t instruction = sysbvm_bytecodeCompilerInstruction_create(context, SYSBVM_OPCODE_COERCE_VALUE, operands);
    sysbvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_jump(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t destination)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 1);
    sysbvm_array_atPut(operands, 0, destination);

    sysbvm_tuple_t instruction = sysbvm_bytecodeCompilerInstruction_create(context, SYSBVM_OPCODE_JUMP, operands);
    sysbvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_jumpIfFalse(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t condition, sysbvm_tuple_t destination)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 2);
    sysbvm_array_atPut(operands, 0, condition);
    sysbvm_array_atPut(operands, 1, destination);

    sysbvm_tuple_t instruction = sysbvm_bytecodeCompilerInstruction_create(context, SYSBVM_OPCODE_JUMP_IF_FALSE, operands);
    sysbvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_load(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t destination, sysbvm_tuple_t pointer)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 2);
    sysbvm_array_atPut(operands, 0, destination);
    sysbvm_array_atPut(operands, 1, pointer);

    sysbvm_tuple_t instruction = sysbvm_bytecodeCompilerInstruction_create(context, SYSBVM_OPCODE_LOAD, operands);
    sysbvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_store(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t pointer, sysbvm_tuple_t value)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 2);
    sysbvm_array_atPut(operands, 0, pointer);
    sysbvm_array_atPut(operands, 1, value);

    sysbvm_tuple_t instruction = sysbvm_bytecodeCompilerInstruction_create(context, SYSBVM_OPCODE_STORE, operands);
    sysbvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_slotAt(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t result, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 3);
    sysbvm_array_atPut(operands, 0, result);
    sysbvm_array_atPut(operands, 1, tuple);
    sysbvm_array_atPut(operands, 2, typeSlot);

    sysbvm_tuple_t instruction = sysbvm_bytecodeCompilerInstruction_create(context, SYSBVM_OPCODE_SLOT_AT, operands);
    sysbvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_slotReferenceAt(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t result, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 3);
    sysbvm_array_atPut(operands, 0, result);
    sysbvm_array_atPut(operands, 1, tuple);
    sysbvm_array_atPut(operands, 2, typeSlot);

    sysbvm_tuple_t instruction = sysbvm_bytecodeCompilerInstruction_create(context, SYSBVM_OPCODE_SLOT_REFERENCE_AT, operands);
    sysbvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_slotAtPut(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot, sysbvm_tuple_t value)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 3);
    sysbvm_array_atPut(operands, 0, tuple);
    sysbvm_array_atPut(operands, 1, typeSlot);
    sysbvm_array_atPut(operands, 2, value);

    sysbvm_tuple_t instruction = sysbvm_bytecodeCompilerInstruction_create(context, SYSBVM_OPCODE_SLOT_AT_PUT, operands);
    sysbvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_refSlotAt(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t result, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 3);
    sysbvm_array_atPut(operands, 0, result);
    sysbvm_array_atPut(operands, 1, tuple);
    sysbvm_array_atPut(operands, 2, typeSlot);

    sysbvm_tuple_t instruction = sysbvm_bytecodeCompilerInstruction_create(context, SYSBVM_OPCODE_REF_SLOT_AT, operands);
    sysbvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_refSlotReferenceAt(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t result, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 3);
    sysbvm_array_atPut(operands, 0, result);
    sysbvm_array_atPut(operands, 1, tuple);
    sysbvm_array_atPut(operands, 2, typeSlot);

    sysbvm_tuple_t instruction = sysbvm_bytecodeCompilerInstruction_create(context, SYSBVM_OPCODE_REF_SLOT_REFERENCE_AT, operands);
    sysbvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_refSlotAtPut(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot, sysbvm_tuple_t value)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 3);
    sysbvm_array_atPut(operands, 0, tuple);
    sysbvm_array_atPut(operands, 1, typeSlot);
    sysbvm_array_atPut(operands, 2, value);

    sysbvm_tuple_t instruction = sysbvm_bytecodeCompilerInstruction_create(context, SYSBVM_OPCODE_REF_SLOT_AT_PUT, operands);
    sysbvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_typecheck(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t expectedType, sysbvm_tuple_t value)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 2);
    sysbvm_array_atPut(operands, 0, expectedType);
    sysbvm_array_atPut(operands, 1, value);

    sysbvm_tuple_t instruction = sysbvm_bytecodeCompilerInstruction_create(context, SYSBVM_OPCODE_TYPECHECK, operands);
    sysbvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_jumpIfTrue(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t condition, sysbvm_tuple_t destination)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 2);
    sysbvm_array_atPut(operands, 0, condition);
    sysbvm_array_atPut(operands, 1, destination);

    sysbvm_tuple_t instruction = sysbvm_bytecodeCompilerInstruction_create(context, SYSBVM_OPCODE_JUMP_IF_TRUE, operands);
    sysbvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_call(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t result, sysbvm_tuple_t function, sysbvm_tuple_t arguments)
{
    size_t argumentCount = sysbvm_array_getSize(arguments);
    sysbvm_tuple_t operands = sysbvm_array_create(context, 2 + argumentCount);
    sysbvm_array_atPut(operands, 0, result);
    sysbvm_array_atPut(operands, 1, function);
    for(size_t i = 0; i < argumentCount; ++i)
        sysbvm_array_atPut(operands, 2 + i, sysbvm_array_at(arguments, i));
    
    sysbvm_tuple_t instruction = sysbvm_bytecodeCompilerInstruction_create(context, SYSBVM_OPCODE_CALL + (argumentCount & 0xF), operands);
    sysbvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_uncheckedCall(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t result, sysbvm_tuple_t function, sysbvm_tuple_t arguments)
{
    size_t argumentCount = sysbvm_array_getSize(arguments);
    sysbvm_tuple_t operands = sysbvm_array_create(context, 2 + argumentCount);
    sysbvm_array_atPut(operands, 0, result);
    sysbvm_array_atPut(operands, 1, function);
    for(size_t i = 0; i < argumentCount; ++i)
        sysbvm_array_atPut(operands, 2 + i, sysbvm_array_at(arguments, i));
    
    sysbvm_tuple_t instruction = sysbvm_bytecodeCompilerInstruction_create(context, SYSBVM_OPCODE_UNCHECKED_CALL + (argumentCount & 0xF), operands);
    sysbvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_send(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t result, sysbvm_tuple_t selector, sysbvm_tuple_t receiver, sysbvm_tuple_t arguments)
{
    size_t argumentCount = sysbvm_array_getSize(arguments);
    sysbvm_tuple_t operands = sysbvm_array_create(context, 3 + argumentCount);
    sysbvm_array_atPut(operands, 0, result);
    sysbvm_array_atPut(operands, 1, selector);
    sysbvm_array_atPut(operands, 2, receiver);
    for(size_t i = 0; i < argumentCount; ++i)
        sysbvm_array_atPut(operands, 3 + i, sysbvm_array_at(arguments, i));
    
    sysbvm_tuple_t instruction = sysbvm_bytecodeCompilerInstruction_create(context, SYSBVM_OPCODE_SEND + (argumentCount & 0xF), operands);
    sysbvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_sendWithLookupReceiverType(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t result, sysbvm_tuple_t receiverLookupType, sysbvm_tuple_t selector, sysbvm_tuple_t receiver, sysbvm_tuple_t arguments)
{
    size_t argumentCount = sysbvm_array_getSize(arguments);
    sysbvm_tuple_t operands = sysbvm_array_create(context, 4 + argumentCount);
    sysbvm_array_atPut(operands, 0, result);
    sysbvm_array_atPut(operands, 1, receiverLookupType);
    sysbvm_array_atPut(operands, 2, selector);
    sysbvm_array_atPut(operands, 3, receiver);
    for(size_t i = 0; i < argumentCount; ++i)
        sysbvm_array_atPut(operands, 4 + i, sysbvm_array_at(arguments, i));
    
    sysbvm_tuple_t instruction = sysbvm_bytecodeCompilerInstruction_create(context, SYSBVM_OPCODE_SEND_WITH_LOOKUP + (argumentCount & 0xF), operands);
    sysbvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_makeArrayWithElements(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t result, sysbvm_tuple_t elements)
{
    size_t elementCount = sysbvm_array_getSize(elements);
    sysbvm_tuple_t operands = sysbvm_array_create(context, 1 + elementCount);
    sysbvm_array_atPut(operands, 0, result);
    for(size_t i = 0; i < elementCount; ++i)
        sysbvm_array_atPut(operands, 1 + i, sysbvm_array_at(elements, i));
    
    sysbvm_tuple_t instruction = sysbvm_bytecodeCompilerInstruction_create(context, SYSBVM_OPCODE_MAKE_ARRAY_WITH_ELEMENTS + (elementCount & 0xF), operands);
    sysbvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_makeAssociation(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t result, sysbvm_tuple_t key, sysbvm_tuple_t value)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 3);
    sysbvm_array_atPut(operands, 0, result);
    sysbvm_array_atPut(operands, 1, key);
    sysbvm_array_atPut(operands, 2, value);
    
    sysbvm_tuple_t instruction = sysbvm_bytecodeCompilerInstruction_create(context, SYSBVM_OPCODE_MAKE_ASSOCIATION, operands);
    sysbvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_makeByteArrayWithElements(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t result, sysbvm_tuple_t elements)
{
    size_t elementCount = sysbvm_array_getSize(elements);
    sysbvm_tuple_t operands = sysbvm_array_create(context, 1 + elementCount);
    sysbvm_array_atPut(operands, 0, result);
    for(size_t i = 0; i < elementCount; ++i)
        sysbvm_array_atPut(operands, 1 + i, sysbvm_array_at(elements, i));
    
    sysbvm_tuple_t instruction = sysbvm_bytecodeCompilerInstruction_create(context, SYSBVM_OPCODE_MAKE_BYTE_ARRAY_WITH_ELEMENTS + (elementCount & 0xF), operands);
    sysbvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_makeDictionaryWithElements(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t result, sysbvm_tuple_t elements)
{
    size_t elementCount = sysbvm_array_getSize(elements);
    sysbvm_tuple_t operands = sysbvm_array_create(context, 1 + elementCount);
    sysbvm_array_atPut(operands, 0, result);
    for(size_t i = 0; i < elementCount; ++i)
        sysbvm_array_atPut(operands, 1 + i, sysbvm_array_at(elements, i));
    
    sysbvm_tuple_t instruction = sysbvm_bytecodeCompilerInstruction_create(context, SYSBVM_OPCODE_MAKE_DICTIONARY_WITH_ELEMENTS + (elementCount & 0xF), operands);
    sysbvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_makeClosureWithCaptures(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t result, sysbvm_tuple_t functionDefinition, sysbvm_tuple_t captures)
{
    size_t captureCount = sysbvm_array_getSize(captures);
    sysbvm_tuple_t operands = sysbvm_array_create(context, 2 + captureCount);
    sysbvm_array_atPut(operands, 0, result);
    sysbvm_array_atPut(operands, 1, functionDefinition);
    for(size_t i = 0; i < captureCount; ++i)
        sysbvm_array_atPut(operands, 2 + i, sysbvm_array_at(captures, i));
    
    sysbvm_tuple_t instruction = sysbvm_bytecodeCompilerInstruction_create(context, SYSBVM_OPCODE_MAKE_CLOSURE_WITH_CAPTURES + (captureCount & 0xF), operands);
    sysbvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_move(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t destination, sysbvm_tuple_t value)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 2);
    sysbvm_array_atPut(operands, 0, destination);
    sysbvm_array_atPut(operands, 1, value);

    sysbvm_tuple_t instruction = sysbvm_bytecodeCompilerInstruction_create(context, SYSBVM_OPCODE_MOVE, operands);
    sysbvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeCompiler_return(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t value)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 1);
    sysbvm_array_atPut(operands, 0, value);

    sysbvm_tuple_t instruction = sysbvm_bytecodeCompilerInstruction_create(context, SYSBVM_OPCODE_RETURN, operands);
    sysbvm_bytecodeCompiler_addInstruction(compiler, instruction);
    return instruction;
}

static size_t sysbvm_bytecodeCompilerInstruction_computeAssembledSize(sysbvm_bytecodeCompilerInstruction_t *instruction)
{
    // Is this a label?
    if(!instruction->operands || !instruction->opcode)
        return 0;

    return 1 + sysbvm_array_getSize(instruction->operands) * 2;
}

static size_t sysbvm_bytecodeCompilerInstructionOperandFor_assembleInto(sysbvm_context_t *context, sysbvm_tuple_t operand, uint8_t *destination, sysbvm_bytecodeCompilerInstruction_t *instruction)
{
    sysbvm_tuple_t operandType = sysbvm_tuple_getType(context, operand);
    if(operandType == context->roots.bytecodeCompilerInstructionType)
    {
        sysbvm_bytecodeCompilerInstruction_t *destinationInstruction = (sysbvm_bytecodeCompilerInstruction_t*)operand;
        size_t destPC = sysbvm_tuple_size_decode(destinationInstruction->pc);
        size_t sourcePC = sysbvm_tuple_size_decode(instruction->endPC);
        int16_t pcDelta = (int16_t) (destPC - sourcePC);
        *destination++ = pcDelta & 0xFF;
        *destination++ = (pcDelta >> 8) & 0xFF;
        return 2;
    }
    else if(operandType == context->roots.bytecodeCompilerInstructionVectorOperandType)
    {
        sysbvm_bytecodeCompilerInstructionVectorOperand_t *vectorOperand = (sysbvm_bytecodeCompilerInstructionVectorOperand_t*)operand;
        int16_t vectorType = sysbvm_tuple_int16_decode(vectorOperand->vectorType);
        int16_t vectorIndex = sysbvm_tuple_int16_decode(vectorOperand->index);
        int16_t encodedOperand = (vectorIndex << SYSBVM_OPERAND_VECTOR_BITS) | vectorType;
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

static size_t sysbvm_bytecodeCompilerInstruction_assembleInto(sysbvm_context_t *context, sysbvm_bytecodeCompilerInstruction_t *instruction, uint8_t *destination)
{
    // Is this a label?
    if(!instruction->operands || !instruction->opcode)
        return 0;

    size_t offset = 0;
    destination[offset++] = sysbvm_tuple_uint8_decode(instruction->opcode);

    size_t operandCount = sysbvm_array_getSize(instruction->operands);
    for(size_t i = 0; i < operandCount; ++i)
    {
        sysbvm_tuple_t operand = sysbvm_array_at(instruction->operands, i);
        offset += sysbvm_bytecodeCompilerInstructionOperandFor_assembleInto(context, operand, destination + offset, instruction);
    }

    return offset;
}

static void sysbvm_bytecodeCompiler_removeInstruction(sysbvm_bytecodeCompiler_t *compiler, sysbvm_bytecodeCompilerInstruction_t *instruction)
{
    sysbvm_bytecodeCompilerInstruction_t *previous = instruction->previous;
    sysbvm_bytecodeCompilerInstruction_t *next = instruction->next;

    if(previous)
        previous->next = next;
    else
        compiler->firstInstruction = next;

    if(next)
        next->previous = previous;
    else
        compiler->lastInstruction = previous;
}

static void sysbvm_bytecodeCompiler_optimizeJumps(sysbvm_bytecodeCompiler_t *compiler)
{
    sysbvm_bytecodeCompilerInstruction_t *instruction = compiler->firstInstruction;
    while(instruction)
    {
        sysbvm_bytecodeCompilerInstruction_t *nextInstruction = instruction->next;
        uint8_t opcode = sysbvm_tuple_uint8_decode(instruction->opcode);
        if(opcode == SYSBVM_OPCODE_JUMP || opcode == SYSBVM_OPCODE_JUMP_IF_TRUE || opcode == SYSBVM_OPCODE_JUMP_IF_FALSE)
        {
            sysbvm_tuple_t lastOperand = sysbvm_array_at(instruction->operands, sysbvm_array_getSize(instruction->operands) - 1);
            if(lastOperand == (sysbvm_tuple_t)nextInstruction)
                sysbvm_bytecodeCompiler_removeInstruction(compiler, instruction);

        }

        instruction = nextInstruction;
    }

}

static void sysbvm_bytecodeCompiler_markInstructionOperandUsages(sysbvm_context_t *context, sysbvm_bytecodeCompilerInstruction_t *instruction)
{
    (void)context;
    // Ignore labels.
    if(!instruction->opcode || !instruction->operands)
        return;

    uint8_t opcode = sysbvm_tuple_uint8_decode(instruction->opcode);
    uint8_t destinationOperandCount = sysbvm_bytecodeInterpreter_destinationOperandCountForOpcode(opcode);

    size_t operandCount = sysbvm_array_getSize(instruction->operands);
    if(opcode == SYSBVM_OPCODE_JUMP || opcode == SYSBVM_OPCODE_JUMP_IF_TRUE || opcode == SYSBVM_OPCODE_JUMP_IF_FALSE)
        --operandCount;
    
    // Mark the destination operands.
    for(size_t i = 0; i < destinationOperandCount; ++i)
    {
        sysbvm_bytecodeCompilerInstructionVectorOperand_t *operand = (sysbvm_bytecodeCompilerInstructionVectorOperand_t*)sysbvm_array_at(instruction->operands, i);
        if(opcode == SYSBVM_OPCODE_ALLOCA || opcode == SYSBVM_OPCODE_ALLOCA_WITH_VALUE)
            operand->hasAllocaDestination = SYSBVM_TRUE_TUPLE;
        else
            operand->hasNonAllocaDestination = SYSBVM_TRUE_TUPLE;

        if(opcode == SYSBVM_OPCODE_SLOT_REFERENCE_AT)
            operand->hasSlotReferenceAtDestination = SYSBVM_TRUE_TUPLE;
        else
            operand->hasNonSlotReferenceAtDestination = SYSBVM_TRUE_TUPLE;
    }

    // Mark the used operands.
    for(size_t i = destinationOperandCount; i < operandCount; ++i)
    {
        sysbvm_bytecodeCompilerInstructionVectorOperand_t *operand = (sysbvm_bytecodeCompilerInstructionVectorOperand_t*)sysbvm_array_at(instruction->operands, i);
        if(opcode == SYSBVM_OPCODE_LOAD)
            operand->hasLoadStoreUsage = SYSBVM_TRUE_TUPLE;
        else if(opcode == SYSBVM_OPCODE_STORE && i == 0)
            operand->hasLoadStoreUsage = SYSBVM_TRUE_TUPLE;
        else
            operand->hasNonLoadStoreUsage = SYSBVM_TRUE_TUPLE;
    }
}

static bool sysbvm_bytecodeCompiler_isLocalOnlyAlloca(sysbvm_tuple_t operand)
{
    sysbvm_bytecodeCompilerInstructionVectorOperand_t *vectorOperand = (sysbvm_bytecodeCompilerInstructionVectorOperand_t*)operand;

    bool hasAllocaDestination = sysbvm_tuple_boolean_decode(vectorOperand->hasAllocaDestination);
    bool hasNonAllocaDestination = sysbvm_tuple_boolean_decode(vectorOperand->hasNonAllocaDestination);
    bool hasLoadStoreUsage = sysbvm_tuple_boolean_decode(vectorOperand->hasLoadStoreUsage);
    bool hasNonLoadStoreUsage = sysbvm_tuple_boolean_decode(vectorOperand->hasNonLoadStoreUsage);
    
    return (hasAllocaDestination && !hasNonAllocaDestination) &&
        (hasLoadStoreUsage && !hasNonLoadStoreUsage);
}

static bool sysbvm_bytecodeCompiler_isLocalOnlyReferenceAt(sysbvm_tuple_t operand)
{
    sysbvm_bytecodeCompilerInstructionVectorOperand_t *vectorOperand = (sysbvm_bytecodeCompilerInstructionVectorOperand_t*)operand;

    bool hasSlotReferenceAtDestination = sysbvm_tuple_boolean_decode(vectorOperand->hasSlotReferenceAtDestination);
    bool hasNonSlotReferenceAtDestination = sysbvm_tuple_boolean_decode(vectorOperand->hasNonSlotReferenceAtDestination);
    bool hasLoadStoreUsage = sysbvm_tuple_boolean_decode(vectorOperand->hasLoadStoreUsage);
    bool hasNonLoadStoreUsage = sysbvm_tuple_boolean_decode(vectorOperand->hasNonLoadStoreUsage);
    
    return (hasSlotReferenceAtDestination && !hasNonSlotReferenceAtDestination) &&
        (hasLoadStoreUsage && !hasNonLoadStoreUsage);
}

static void sysbvm_bytecodeCompiler_optimizeLocalOnlyAllocaAndSlotReferences(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_bytecodeCompilerInstruction_t *instruction)
{
    // Ignore labels.
    if(!instruction->opcode || !instruction->operands)
        return;

    uint8_t opcode = sysbvm_tuple_uint8_decode(instruction->opcode);
    if(opcode == SYSBVM_OPCODE_ALLOCA
        && sysbvm_bytecodeCompiler_isLocalOnlyAlloca(sysbvm_array_at(instruction->operands, 0)))
    {
        instruction->opcode = sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_MOVE);
        sysbvm_array_atPut(instruction->operands, 1, sysbvm_bytecodeCompiler_addLiteral(context, compiler, SYSBVM_NULL_TUPLE));
    }
    else if(opcode == SYSBVM_OPCODE_ALLOCA_WITH_VALUE
        && sysbvm_bytecodeCompiler_isLocalOnlyAlloca(sysbvm_array_at(instruction->operands, 0)))
    {
        sysbvm_tuple_t newOperands = sysbvm_array_create(context, 2);
        sysbvm_array_atPut(newOperands, 0, sysbvm_array_at(instruction->operands, 0));
        sysbvm_array_atPut(newOperands, 1, sysbvm_array_at(instruction->operands, 2));
        instruction->opcode = sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_MOVE);
        instruction->operands = newOperands;
    }
    else if(opcode == SYSBVM_OPCODE_SLOT_REFERENCE_AT
        && sysbvm_bytecodeCompiler_isLocalOnlyReferenceAt(sysbvm_array_at(instruction->operands, 0)))
    {
        sysbvm_bytecodeCompilerInstructionVectorOperand_t *referenceOperand = (sysbvm_bytecodeCompilerInstructionVectorOperand_t*)sysbvm_array_at(instruction->operands, 0);
        referenceOperand->optimizationTupleOperand = sysbvm_array_at(instruction->operands, 1);
        referenceOperand->optimizationTypeSlotOperand = sysbvm_array_at(instruction->operands, 2);

        sysbvm_bytecodeCompiler_removeInstruction((sysbvm_bytecodeCompiler_t*)compiler, instruction);
    }
    else if(opcode == SYSBVM_OPCODE_LOAD
        && sysbvm_bytecodeCompiler_isLocalOnlyAlloca(sysbvm_array_at(instruction->operands, 1)))
    {
        instruction->opcode = sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_MOVE);
    }
    else if(opcode == SYSBVM_OPCODE_LOAD
        && sysbvm_bytecodeCompiler_isLocalOnlyReferenceAt(sysbvm_array_at(instruction->operands, 1)))
    {
        sysbvm_bytecodeCompilerInstructionVectorOperand_t *referenceOperand = (sysbvm_bytecodeCompilerInstructionVectorOperand_t*)sysbvm_array_at(instruction->operands, 1);

        sysbvm_tuple_t newOperands = sysbvm_array_create(context, 3);
        sysbvm_array_atPut(newOperands, 0, sysbvm_array_at(instruction->operands, 0));
        sysbvm_array_atPut(newOperands, 1, referenceOperand->optimizationTupleOperand);
        sysbvm_array_atPut(newOperands, 2, referenceOperand->optimizationTypeSlotOperand);
        instruction->opcode = sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_SLOT_AT);
        instruction->operands = newOperands;
    }
    else if(opcode == SYSBVM_OPCODE_STORE
        && sysbvm_bytecodeCompiler_isLocalOnlyAlloca(sysbvm_array_at(instruction->operands, 0)))
    {
        instruction->opcode = sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_MOVE);
    }
    else if(opcode == SYSBVM_OPCODE_STORE
        && sysbvm_bytecodeCompiler_isLocalOnlyReferenceAt(sysbvm_array_at(instruction->operands, 0)))
    {
        sysbvm_bytecodeCompilerInstructionVectorOperand_t *referenceOperand = (sysbvm_bytecodeCompilerInstructionVectorOperand_t*)sysbvm_array_at(instruction->operands, 0);

        sysbvm_tuple_t newOperands = sysbvm_array_create(context, 3);
        sysbvm_array_atPut(newOperands, 0, referenceOperand->optimizationTupleOperand);
        sysbvm_array_atPut(newOperands, 1, referenceOperand->optimizationTypeSlotOperand);
        sysbvm_array_atPut(newOperands, 2, sysbvm_array_at(instruction->operands, 1));
        instruction->opcode = sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_SLOT_AT_PUT);
        instruction->operands = newOperands;
    }
}

static void sysbvm_bytecodeCompiler_optimizeTemporaries(sysbvm_context_t *context, sysbvm_bytecodeCompiler_t *compiler)
{
    // Clear the temporaries usage flags.
    size_t temporaryCount = sysbvm_orderedCollection_getSize(compiler->temporaries);
    for(size_t i = 0; i < temporaryCount; ++i)
    {
        sysbvm_bytecodeCompilerInstructionVectorOperand_t *temporary = (sysbvm_bytecodeCompilerInstructionVectorOperand_t*)sysbvm_orderedCollection_at(compiler->temporaries, i);
        temporary->hasAllocaDestination = SYSBVM_FALSE_TUPLE;
        temporary->hasNonAllocaDestination = SYSBVM_FALSE_TUPLE;
        temporary->hasLoadStoreUsage = SYSBVM_FALSE_TUPLE;
        temporary->hasNonLoadStoreUsage = SYSBVM_FALSE_TUPLE;
    }

    // Mark the temporaries per instruction usage.
    sysbvm_bytecodeCompilerInstruction_t *instruction = compiler->firstInstruction;
    while(instruction)
    {
        sysbvm_bytecodeCompiler_markInstructionOperandUsages(context, instruction);
        instruction = instruction->next;
    }

    // Optimize the local only allocas.
    instruction = compiler->firstInstruction;
    while(instruction)
    {
        sysbvm_bytecodeCompilerInstruction_t *nextInstruction = instruction->next;
        sysbvm_bytecodeCompiler_optimizeLocalOnlyAllocaAndSlotReferences(context, (sysbvm_tuple_t)compiler, instruction);
        instruction = nextInstruction;
    }
}

SYSBVM_API void sysbvm_bytecodeCompiler_compileFunctionDefinition(sysbvm_context_t *context, sysbvm_functionDefinition_t *definition_)
{
    struct {
        sysbvm_functionDefinition_t *definition;
        sysbvm_bytecodeCompiler_t *compiler;
        sysbvm_tuple_t bodyResult;
        sysbvm_functionBytecode_t *bytecode;
        sysbvm_bytecodeCompilerInstruction_t *instruction;

        sysbvm_tuple_t pcToDebugListTable;
        sysbvm_tuple_t debugSourceTuple;
        sysbvm_tuple_t debugSourceASTNodes;
        sysbvm_tuple_t debugSourcePositions;
        sysbvm_tuple_t debugSourceEnvironments;
        sysbvm_tuple_t debugListsDictionary;
        sysbvm_tuple_t tableEntry;

        sysbvm_tuple_t debugPC;
        sysbvm_tuple_t lastDebugPC;
        sysbvm_tuple_t lastTableEntity;
    } gcFrame = {
        .definition = (sysbvm_functionDefinition_t*)definition_,
    };

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.compiler = (sysbvm_bytecodeCompiler_t*)sysbvm_bytecodeCompiler_create(context);
    gcFrame.compiler->sourceEnvironment = gcFrame.definition->analysisEnvironment;
    gcFrame.compiler->sourcePosition = gcFrame.definition->sourcePosition;

    {
        // Make the argument operands.
        size_t argumentCount = sysbvm_array_getSize(gcFrame.definition->analyzedArguments);
        sysbvm_bytecodeCompiler_setArgumentCount(context, (sysbvm_tuple_t)gcFrame.compiler, argumentCount);
        for(size_t i = 0; i < argumentCount; ++i)
        {
            sysbvm_tuple_t binding = sysbvm_array_at(gcFrame.definition->analyzedArguments, i);
            sysbvm_tuple_t value = sysbvm_array_at(gcFrame.compiler->arguments, i);
            sysbvm_bytecodeCompiler_setBindingValue(context, (sysbvm_tuple_t)gcFrame.compiler, binding, value);
        }

        // Make the binding operands.
        size_t captureCount = sysbvm_array_getSize(gcFrame.definition->analyzedCaptures);
        sysbvm_bytecodeCompiler_setCaptureCount(context, (sysbvm_tuple_t)gcFrame.compiler, captureCount);
        for(size_t i = 0; i < captureCount; ++i)
        {
            sysbvm_tuple_t binding = sysbvm_array_at(gcFrame.definition->analyzedCaptures, i);
            sysbvm_tuple_t value = sysbvm_array_at(gcFrame.compiler->captures, i);
            sysbvm_bytecodeCompiler_setBindingValue(context, (sysbvm_tuple_t)gcFrame.compiler, binding, value);
        }
    }

    // Compile the body.
    gcFrame.bodyResult = gcFrame.definition->analyzedBodyNode
        ? sysbvm_bytecodeCompiler_compileASTNode(context, (sysbvm_tuple_t)gcFrame.compiler, gcFrame.definition->analyzedBodyNode)
        : sysbvm_bytecodeCompiler_addLiteral(context, (sysbvm_tuple_t)gcFrame.compiler, SYSBVM_VOID_TUPLE);

    if(gcFrame.bodyResult)
        sysbvm_bytecodeCompiler_return(context, (sysbvm_tuple_t)gcFrame.compiler, gcFrame.bodyResult);

    // Perform some optimizations.
    sysbvm_bytecodeCompiler_optimizeJumps(gcFrame.compiler);
    sysbvm_bytecodeCompiler_optimizeTemporaries(context, gcFrame.compiler);
    
    // Assemble the instructions.
    gcFrame.compiler->usedTemporaryCount = sysbvm_tuple_size_encode(context, sysbvm_orderedCollection_getSize(gcFrame.compiler->temporaries));

    gcFrame.bytecode = (sysbvm_functionBytecode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.functionBytecodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_functionBytecode_t));
    gcFrame.bytecode->argumentCount = sysbvm_tuple_size_encode(context, sysbvm_array_getSize(gcFrame.compiler->arguments));
    gcFrame.bytecode->captureVectorSize = sysbvm_tuple_size_encode(context, sysbvm_array_getSize(gcFrame.compiler->captures));
    gcFrame.bytecode->localVectorSize = gcFrame.compiler->usedTemporaryCount;
    gcFrame.bytecode->literalVector = sysbvm_orderedCollection_asArray(context, gcFrame.compiler->literals);

    // Tables for the debug information.
    gcFrame.pcToDebugListTable = sysbvm_orderedCollection_create(context);
    gcFrame.debugSourceASTNodes = sysbvm_orderedCollection_create(context);
    gcFrame.debugSourcePositions = sysbvm_orderedCollection_create(context);
    gcFrame.debugSourceEnvironments = sysbvm_orderedCollection_create(context);
    gcFrame.debugListsDictionary = sysbvm_dictionary_create(context);

    // Assemble the instructions.
    size_t instructionsOffset = 0;
    for(gcFrame.instruction = gcFrame.compiler->firstInstruction; gcFrame.instruction; gcFrame.instruction = gcFrame.instruction->next)
    {
        size_t pc = instructionsOffset;
        gcFrame.instruction->pc = sysbvm_tuple_size_encode(context, instructionsOffset);
        instructionsOffset += sysbvm_bytecodeCompilerInstruction_computeAssembledSize(gcFrame.instruction);
        gcFrame.instruction->endPC = sysbvm_tuple_size_encode(context, instructionsOffset);

        // Ensure the debug source tuple is on their respective tables.
        gcFrame.debugSourceTuple = sysbvm_array_create(context, 3);
        sysbvm_array_atPut(gcFrame.debugSourceTuple, 0, gcFrame.instruction->sourceASTNode);
        sysbvm_array_atPut(gcFrame.debugSourceTuple, 1, gcFrame.instruction->sourcePosition);
        sysbvm_array_atPut(gcFrame.debugSourceTuple, 2, gcFrame.instruction->sourceEnvironment);
        if(!sysbvm_dictionary_find(context, gcFrame.debugListsDictionary, gcFrame.debugSourceTuple, &gcFrame.tableEntry))
        {
            gcFrame.tableEntry = sysbvm_tuple_size_encode(context, sysbvm_orderedCollection_getSize(gcFrame.debugSourceASTNodes));
            sysbvm_dictionary_atPut(context, gcFrame.debugListsDictionary, gcFrame.debugSourceTuple, gcFrame.tableEntry);

            sysbvm_orderedCollection_add(context, gcFrame.debugSourceASTNodes, gcFrame.instruction->sourceASTNode);
            sysbvm_orderedCollection_add(context, gcFrame.debugSourcePositions, gcFrame.instruction->sourcePosition);
            sysbvm_orderedCollection_add(context, gcFrame.debugSourceEnvironments, gcFrame.instruction->sourceEnvironment);
        }

        gcFrame.debugPC = sysbvm_tuple_size_encode(context, pc);
        if(gcFrame.debugPC != gcFrame.lastDebugPC || gcFrame.tableEntry != gcFrame.lastTableEntity)
        {
            sysbvm_orderedCollection_add(context, gcFrame.pcToDebugListTable, gcFrame.debugPC);
            sysbvm_orderedCollection_add(context, gcFrame.pcToDebugListTable, gcFrame.tableEntry);
        }

        gcFrame.lastDebugPC = gcFrame.debugPC;
        gcFrame.lastTableEntity = gcFrame.tableEntry;
    }

    gcFrame.bytecode->instructions = sysbvm_byteArray_create(context, instructionsOffset);
    instructionsOffset = 0;
    uint8_t *destInstructions = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(gcFrame.bytecode->instructions)->bytes;
    for(gcFrame.instruction = gcFrame.compiler->firstInstruction; gcFrame.instruction; gcFrame.instruction = gcFrame.instruction->next)
        instructionsOffset += sysbvm_bytecodeCompilerInstruction_assembleInto(context, gcFrame.instruction, destInstructions + instructionsOffset);

    // Finish by installing it on the definition.
    gcFrame.definition->bytecode = (sysbvm_tuple_t)gcFrame.bytecode;

    gcFrame.bytecode->pcToDebugListTable = sysbvm_orderedCollection_asArray(context, gcFrame.pcToDebugListTable);
    gcFrame.bytecode->debugSourceASTNodes = sysbvm_orderedCollection_asArray(context, gcFrame.debugSourceASTNodes);
    gcFrame.bytecode->debugSourcePositions = sysbvm_orderedCollection_asArray(context, gcFrame.debugSourcePositions);
    gcFrame.bytecode->debugSourceEnvironments = sysbvm_orderedCollection_asArray(context, gcFrame.debugSourceEnvironments);
}

static sysbvm_tuple_t sysbvm_astBreakNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    //sysbvm_tuple_t *node = &arguments[0];
    sysbvm_bytecodeCompiler_t **compiler = (sysbvm_bytecodeCompiler_t **)&arguments[1];
    if(!(*compiler)->breakLabel)
        sysbvm_error("Break statement in wrong location.");
    sysbvm_bytecodeCompiler_jump(context, (sysbvm_tuple_t)*compiler, (*compiler)->breakLabel);
     
    return sysbvm_bytecodeCompiler_addLiteral(context, (sysbvm_tuple_t)*compiler, SYSBVM_VOID_TUPLE);
}

static sysbvm_tuple_t sysbvm_astCoerceValueNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *compiler = &arguments[1];

    sysbvm_astCoerceValueNode_t **coerceValueNode = (sysbvm_astCoerceValueNode_t**)node;
    struct {
        sysbvm_tuple_t typeOperand;
        sysbvm_tuple_t valueOperand;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.typeOperand = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, (*coerceValueNode)->typeExpression);
    gcFrame.valueOperand = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, (*coerceValueNode)->valueExpression);
    gcFrame.result = sysbvm_bytecodeCompiler_newTemporary(context, *compiler);
    sysbvm_bytecodeCompiler_coerceValue(context, *compiler, gcFrame.result, gcFrame.typeOperand, gcFrame.valueOperand);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astContinueNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    //sysbvm_tuple_t *node = &arguments[0];
    sysbvm_bytecodeCompiler_t **compiler = (sysbvm_bytecodeCompiler_t **)&arguments[1];
    if(!(*compiler)->continueLabel)
        sysbvm_error("Continue statement in wrong location.");
    sysbvm_bytecodeCompiler_jump(context, (sysbvm_tuple_t)*compiler, (*compiler)->continueLabel);
     
    return sysbvm_bytecodeCompiler_addLiteral(context, (sysbvm_tuple_t)*compiler, SYSBVM_VOID_TUPLE);
}

static sysbvm_tuple_t sysbvm_astDoWhileContinueWithNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *compiler = &arguments[1];

    sysbvm_astDoWhileContinueWithNode_t **doWhileNode = (sysbvm_astDoWhileContinueWithNode_t**)node;
    struct {
        sysbvm_tuple_t doWhileEntryLabel;
        sysbvm_tuple_t doWhileCondition;
        sysbvm_tuple_t doWhileContinue;
        sysbvm_tuple_t doWhileMergeLabel;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.doWhileEntryLabel = sysbvm_bytecodeCompilerInstruction_createLabel(context);
    gcFrame.doWhileCondition = sysbvm_bytecodeCompilerInstruction_createLabel(context);
    gcFrame.doWhileContinue = sysbvm_bytecodeCompilerInstruction_createLabel(context);
    gcFrame.doWhileMergeLabel = sysbvm_bytecodeCompilerInstruction_createLabel(context);

    // Do while body.
    sysbvm_bytecodeCompiler_addInstruction(*compiler, gcFrame.doWhileEntryLabel);
    if((*doWhileNode)->bodyExpression)
        sysbvm_bytecodeCompiler_compileASTNodeWithBreakAndContinue(context, *compiler, (*doWhileNode)->bodyExpression, gcFrame.doWhileMergeLabel, gcFrame.doWhileCondition);
    sysbvm_bytecodeCompiler_jump(context, *compiler, gcFrame.doWhileCondition);

    // Do while condition block.
    sysbvm_bytecodeCompiler_addInstruction(*compiler, gcFrame.doWhileCondition);
    if((*doWhileNode)->conditionExpression)
    {
        sysbvm_tuple_t condition = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, (*doWhileNode)->conditionExpression);
        sysbvm_bytecodeCompiler_jumpIfFalse(context, *compiler, condition, gcFrame.doWhileMergeLabel);
    }
    else
    {
        sysbvm_bytecodeCompiler_jump(context, *compiler, gcFrame.doWhileContinue);
    }

    // Do while continue
    sysbvm_bytecodeCompiler_addInstruction(*compiler, gcFrame.doWhileContinue);
    if((*doWhileNode)->continueExpression)
        sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, (*doWhileNode)->continueExpression);

    sysbvm_bytecodeCompiler_jump(context, *compiler, gcFrame.doWhileEntryLabel);

    // While merge
    sysbvm_bytecodeCompiler_addInstruction(*compiler, gcFrame.doWhileMergeLabel);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return sysbvm_bytecodeCompiler_addLiteral(context, *compiler, SYSBVM_VOID_TUPLE);
}

static sysbvm_tuple_t sysbvm_astDownCastNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *compiler = &arguments[1];

    sysbvm_astDownCastNode_t **downCastNode = (sysbvm_astDownCastNode_t**)node;
    struct {
        sysbvm_tuple_t typeOperand;
        sysbvm_tuple_t valueOperand;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.typeOperand = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, (*downCastNode)->typeExpression);
    gcFrame.valueOperand = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, (*downCastNode)->valueExpression);
    sysbvm_bytecodeCompiler_typecheck(context, *compiler, gcFrame.typeOperand, gcFrame.valueOperand);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.valueOperand;
}


static sysbvm_tuple_t sysbvm_bytecodeCompiler_getLiteralFunctionPrimitiveName(sysbvm_context_t *context, sysbvm_tuple_t compiler, sysbvm_tuple_t operand)
{
    //sysbvm_tuple_t operandType 
    if(sysbvm_tuple_getType(context, operand) != context->roots.bytecodeCompilerInstructionVectorOperandType)
        return SYSBVM_NULL_TUPLE;
    
    sysbvm_bytecodeCompilerInstructionVectorOperand_t *vectorOperand = (sysbvm_bytecodeCompilerInstructionVectorOperand_t*)operand;
    int16_t index = sysbvm_tuple_int16_decode(vectorOperand->index);
    int16_t vectorType = sysbvm_tuple_int16_decode(vectorOperand->vectorType);
    sysbvm_bytecodeCompiler_t *compilerObject = (sysbvm_bytecodeCompiler_t*)compiler;
    if(vectorType != SYSBVM_OPERAND_VECTOR_LITERAL || index < 0 || (size_t)index >= sysbvm_orderedCollection_getSize(compilerObject->literals))
        return SYSBVM_NULL_TUPLE;

    sysbvm_tuple_t literal = sysbvm_orderedCollection_at(compilerObject->literals, index);
    if(!sysbvm_tuple_isFunction(context, literal))
        return SYSBVM_NULL_TUPLE;

    return ((sysbvm_function_t*)literal)->primitiveName;
}

static sysbvm_tuple_t sysbvm_astFunctionApplicationNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *compiler = &arguments[1];

    sysbvm_astFunctionApplicationNode_t **applicationNode = (sysbvm_astFunctionApplicationNode_t**)node;
    struct {
        sysbvm_tuple_t function;
        sysbvm_tuple_t primitiveName;
        sysbvm_tuple_t arguments;
        sysbvm_tuple_t argumentNode;
        sysbvm_tuple_t argumentOperand;
        sysbvm_tuple_t result;

        sysbvm_tuple_t pointerOperand;
        sysbvm_tuple_t valueOperand;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    size_t applicationArgumentCount = sysbvm_array_getSize((*applicationNode)->arguments);

    gcFrame.function = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, (*applicationNode)->functionExpression);

    // Inline some special functions here.
    gcFrame.primitiveName = sysbvm_bytecodeCompiler_getLiteralFunctionPrimitiveName(context, *compiler, gcFrame.function);
    if(gcFrame.primitiveName == context->roots.anyValueToVoidPrimitiveName)
    {
        for(size_t i = 0; i < applicationArgumentCount; ++i)
        {
            gcFrame.argumentNode = sysbvm_array_at((*applicationNode)->arguments, i);
            sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, gcFrame.argumentNode);
        }

        gcFrame.result = sysbvm_bytecodeCompiler_addLiteral(context, *compiler, SYSBVM_VOID_TUPLE);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return gcFrame.result;
    }
    else if(gcFrame.primitiveName == context->roots.pointerLikeLoadPrimitiveName && applicationArgumentCount == 1)
    {
        gcFrame.argumentNode = sysbvm_array_at((*applicationNode)->arguments, 0);
        gcFrame.pointerOperand = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, gcFrame.argumentNode);

        gcFrame.result = sysbvm_bytecodeCompiler_newTemporary(context, *compiler);
        sysbvm_bytecodeCompiler_load(context, *compiler, gcFrame.result, gcFrame.pointerOperand);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return gcFrame.result;
    }
    else if(gcFrame.primitiveName == context->roots.pointerLikeStorePrimitiveName && applicationArgumentCount == 2)
    {
        gcFrame.argumentNode = sysbvm_array_at((*applicationNode)->arguments, 0);
        gcFrame.pointerOperand = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, gcFrame.argumentNode);

        gcFrame.argumentNode = sysbvm_array_at((*applicationNode)->arguments, 1);
        gcFrame.valueOperand = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, gcFrame.argumentNode);

        sysbvm_bytecodeCompiler_store(context, *compiler, gcFrame.pointerOperand, gcFrame.valueOperand);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return gcFrame.pointerOperand;
    }

    gcFrame.arguments = sysbvm_array_create(context, applicationArgumentCount);

    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        gcFrame.argumentNode = sysbvm_array_at((*applicationNode)->arguments, i);
        gcFrame.argumentOperand = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, gcFrame.argumentNode);
        sysbvm_array_atPut(gcFrame.arguments, i, gcFrame.argumentOperand);
    }

    gcFrame.result = sysbvm_bytecodeCompiler_newTemporary(context, *compiler);

    sysbvm_bitflags_t applicationFlags = sysbvm_tuple_bitflags_decode((*applicationNode)->applicationFlags);
    bool isNotypecheck = (applicationFlags & SYSBVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK) != 0;

    if(isNotypecheck)
        sysbvm_bytecodeCompiler_uncheckedCall(context, *compiler, gcFrame.result, gcFrame.function, gcFrame.arguments);
    else
        sysbvm_bytecodeCompiler_call(context, *compiler, gcFrame.result, gcFrame.function, gcFrame.arguments);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astIdentifierReferenceNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *compiler = &arguments[1];

    sysbvm_astIdentifierReferenceNode_t **referenceNode = (sysbvm_astIdentifierReferenceNode_t**)node;
    return sysbvm_bytecodeCompiler_getBindingValue(context, *compiler, (*referenceNode)->binding);
}

static sysbvm_tuple_t sysbvm_astIfNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *compiler = &arguments[1];

    sysbvm_astIfNode_t **ifNode = (sysbvm_astIfNode_t**)node;
    struct {
        sysbvm_tuple_t falseLabel;
        sysbvm_tuple_t mergeLabel;
        sysbvm_tuple_t trueResult;
        sysbvm_tuple_t falseResult;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.falseLabel = sysbvm_bytecodeCompilerInstruction_createLabel(context);
    gcFrame.mergeLabel = sysbvm_bytecodeCompilerInstruction_createLabel(context);

    // Emit the condition
    {
        sysbvm_tuple_t condition = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, (*ifNode)->conditionExpression);
        sysbvm_bytecodeCompiler_jumpIfFalse(context, *compiler, condition, gcFrame.falseLabel);
    }

    gcFrame.result = sysbvm_bytecodeCompiler_newTemporary(context, *compiler);

    // True branch.
    if((*ifNode)->trueExpression)
        gcFrame.trueResult = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, (*ifNode)->trueExpression);
    else
        gcFrame.trueResult = sysbvm_bytecodeCompiler_addLiteral(context, *compiler, SYSBVM_NULL_TUPLE);
    sysbvm_bytecodeCompiler_move(context, *compiler, gcFrame.result, gcFrame.trueResult);
    sysbvm_bytecodeCompiler_jump(context, *compiler, gcFrame.mergeLabel);

    // False branch.
    sysbvm_bytecodeCompiler_addInstruction(*compiler, gcFrame.falseLabel);
    if((*ifNode)->falseExpression)
        gcFrame.falseResult = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, (*ifNode)->falseExpression);
    else
        gcFrame.falseResult = sysbvm_bytecodeCompiler_addLiteral(context, *compiler, SYSBVM_NULL_TUPLE);
    sysbvm_bytecodeCompiler_move(context, *compiler, gcFrame.result, gcFrame.falseResult);

    sysbvm_bytecodeCompiler_addInstruction(*compiler, gcFrame.mergeLabel);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astLambdaNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *compiler = &arguments[1];

    sysbvm_astLambdaNode_t **lambdaNode = (sysbvm_astLambdaNode_t**)node;

    sysbvm_functionDefinition_t *functionDefinition = (sysbvm_functionDefinition_t*)(*lambdaNode)->functionDefinition;
    sysbvm_tuple_t functionDefinitionOperand = sysbvm_bytecodeCompiler_addLiteral(context, *compiler, (sysbvm_tuple_t)functionDefinition);
    size_t captureVectorSize = sysbvm_array_getSize(functionDefinition->analyzedCaptures);
    sysbvm_tuple_t captureVector = sysbvm_array_create(context, captureVectorSize);
    
    for(size_t i = 0; i < captureVectorSize; ++i)
    {
        sysbvm_tuple_t captureBinding = sysbvm_symbolCaptureBinding_getSourceBinding(sysbvm_array_at(functionDefinition->analyzedCaptures, i));
        sysbvm_tuple_t captureValue = sysbvm_bytecodeCompiler_getBindingValue(context, *compiler, captureBinding);

        sysbvm_array_atPut(captureVector, i, captureValue);
    }

    sysbvm_tuple_t result = sysbvm_bytecodeCompiler_newTemporary(context, *compiler);
    sysbvm_bytecodeCompiler_makeClosureWithCaptures(context, *compiler, result, functionDefinitionOperand, captureVector);

    if((*lambdaNode)->binding)
        sysbvm_bytecodeCompiler_setBindingValue(context, *compiler, (*lambdaNode)->binding, result);
    return result;
}

static sysbvm_tuple_t sysbvm_astLexicalBlockNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *compiler = &arguments[1];

    sysbvm_astLexicalBlockNode_t **lexicalBlockNode = (sysbvm_astLexicalBlockNode_t**)node;

    return sysbvm_bytecodeCompiler_compileASTNodeWithEnvironment(context, *compiler, (*lexicalBlockNode)->body, (*lexicalBlockNode)->bodyEnvironment);
}

static sysbvm_tuple_t sysbvm_astLiteralNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *compiler = &arguments[1];

    sysbvm_astLiteralNode_t **literalNode = (sysbvm_astLiteralNode_t**)node;
    return sysbvm_bytecodeCompiler_addLiteral(context, *compiler, (*literalNode)->value);
}

static sysbvm_tuple_t sysbvm_astLocalDefinitionNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *compiler = &arguments[1];

    sysbvm_astLocalDefinitionNode_t **localDefinitionNode = (sysbvm_astLocalDefinitionNode_t**)node;

    sysbvm_tuple_t value = (*localDefinitionNode)->valueExpression
        ? sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, (*localDefinitionNode)->valueExpression)
        : sysbvm_bytecodeCompiler_addLiteral(context, *compiler, SYSBVM_NULL_TUPLE);

    bool isMutable = sysbvm_tuple_boolean_decode((*localDefinitionNode)->isMutable);
    if(isMutable)
    {
        sysbvm_tuple_t localVariable = sysbvm_bytecodeCompiler_newTemporary(context, *compiler);
        sysbvm_bytecodeCompiler_allocaWithValue(context, *compiler, localVariable,
            sysbvm_bytecodeCompiler_addLiteral(context, *compiler, (*localDefinitionNode)->super.analyzedType),
            value);
        
        sysbvm_bytecodeCompiler_setBindingValue(context, *compiler, (*localDefinitionNode)->binding, localVariable);
        return localVariable;
    }
    else
    {
        sysbvm_bytecodeCompiler_setBindingValue(context, *compiler, (*localDefinitionNode)->binding, value);
        return value;
    }
}

static sysbvm_tuple_t sysbvm_astMakeAssociationNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *compiler = &arguments[1];

    sysbvm_astMakeAssociationNode_t **associationNode = (sysbvm_astMakeAssociationNode_t**)node;
    struct {
        sysbvm_tuple_t key;
        sysbvm_tuple_t value;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.key = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, (*associationNode)->key);
    if((*associationNode)->value)
        gcFrame.value = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, (*associationNode)->value);
    else
        gcFrame.value = sysbvm_bytecodeCompiler_addLiteral(context, *compiler, SYSBVM_NULL_TUPLE);

    gcFrame.result = sysbvm_bytecodeCompiler_newTemporary(context, *compiler);
    sysbvm_bytecodeCompiler_makeAssociation(context, *compiler, gcFrame.result, gcFrame.key, gcFrame.value);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astMakeArrayNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *compiler = &arguments[1];

    sysbvm_astMakeArrayNode_t **arrayNode = (sysbvm_astMakeArrayNode_t**)node;
    struct {
        sysbvm_tuple_t elements;
        sysbvm_tuple_t element;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    size_t arrayElementCountCount = sysbvm_array_getSize((*arrayNode)->elements);
    gcFrame.elements = sysbvm_array_create(context, arrayElementCountCount);
    for(size_t i = 0; i < arrayElementCountCount; ++i)
    {
        gcFrame.element = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, sysbvm_array_at((*arrayNode)->elements, i));
        sysbvm_array_atPut(gcFrame.elements, i, gcFrame.element);
    }

    gcFrame.result = sysbvm_bytecodeCompiler_newTemporary(context, *compiler);
    sysbvm_bytecodeCompiler_makeArrayWithElements(context, *compiler, gcFrame.result, gcFrame.elements);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astMakeByteArrayNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *compiler = &arguments[1];

    sysbvm_astMakeByteArrayNode_t **arrayNode = (sysbvm_astMakeByteArrayNode_t**)node;
    struct {
        sysbvm_tuple_t elements;
        sysbvm_tuple_t element;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    size_t arrayElementCountCount = sysbvm_array_getSize((*arrayNode)->elements);
    gcFrame.elements = sysbvm_array_create(context, arrayElementCountCount);
    for(size_t i = 0; i < arrayElementCountCount; ++i)
    {
        gcFrame.element = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, sysbvm_array_at((*arrayNode)->elements, i));
        sysbvm_array_atPut(gcFrame.elements, i, gcFrame.element);
    }

    gcFrame.result = sysbvm_bytecodeCompiler_newTemporary(context, *compiler);
    sysbvm_bytecodeCompiler_makeByteArrayWithElements(context, *compiler, gcFrame.result, gcFrame.elements);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astMakeDictionaryNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *compiler = &arguments[1];

    sysbvm_astMakeDictionaryNode_t **dictionaryNode = (sysbvm_astMakeDictionaryNode_t**)node;
    struct {
        sysbvm_tuple_t elements;
        sysbvm_tuple_t element;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    size_t arrayElementCountCount = sysbvm_array_getSize((*dictionaryNode)->elements);
    gcFrame.elements = sysbvm_array_create(context, arrayElementCountCount);
    for(size_t i = 0; i < arrayElementCountCount; ++i)
    {
        gcFrame.element = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, sysbvm_array_at((*dictionaryNode)->elements, i));
        sysbvm_array_atPut(gcFrame.elements, i, gcFrame.element);
    }

    gcFrame.result = sysbvm_bytecodeCompiler_newTemporary(context, *compiler);
    sysbvm_bytecodeCompiler_makeDictionaryWithElements(context, *compiler, gcFrame.result, gcFrame.elements);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astMessageSendNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *compiler = &arguments[1];

    sysbvm_astMessageSendNode_t **sendNode = (sysbvm_astMessageSendNode_t**)node;
    struct {
        sysbvm_tuple_t receiver;
        sysbvm_tuple_t receiverLookupType;
        sysbvm_tuple_t selector;
        sysbvm_tuple_t arguments;
        sysbvm_tuple_t argument;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.receiver = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, (*sendNode)->receiver);
    if((*sendNode)->receiverLookupType)
        gcFrame.receiverLookupType = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, (*sendNode)->receiverLookupType);
    gcFrame.selector = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, (*sendNode)->selector);

    size_t sendArgumentCount = sysbvm_array_getSize((*sendNode)->arguments);
    gcFrame.arguments = sysbvm_array_create(context, sendArgumentCount);
    for(size_t i = 0; i < sendArgumentCount; ++i)
    {
        gcFrame.argument = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, sysbvm_array_at((*sendNode)->arguments, i));
        sysbvm_array_atPut(gcFrame.arguments, i, gcFrame.argument);
    }

    gcFrame.result = sysbvm_bytecodeCompiler_newTemporary(context, *compiler);
    if((*sendNode)->receiverLookupType)
        sysbvm_bytecodeCompiler_sendWithLookupReceiverType(context, *compiler, gcFrame.result, gcFrame.receiverLookupType, gcFrame.selector, gcFrame.receiver, gcFrame.arguments);
    else
        sysbvm_bytecodeCompiler_send(context, *compiler, gcFrame.result, gcFrame.selector, gcFrame.receiver, gcFrame.arguments);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astReturnNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *compiler = &arguments[1];

    sysbvm_astReturnNode_t **returnNode = (sysbvm_astReturnNode_t**)node;
    sysbvm_tuple_t result = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, (*returnNode)->expression);
    sysbvm_bytecodeCompiler_return(context, *compiler, result);
    return result;
}

static sysbvm_tuple_t sysbvm_astSequenceNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *compiler = &arguments[1];

    sysbvm_astSequenceNode_t **sequenceNode = (sysbvm_astSequenceNode_t**)node;
    struct {
        sysbvm_tuple_t expressionNode;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    size_t expressionCount = sysbvm_array_getSize((*sequenceNode)->expressions);
    gcFrame.result = SYSBVM_NULL_TUPLE;
    if(expressionCount == 0)
        gcFrame.result = sysbvm_bytecodeCompiler_addLiteral(context, *compiler, SYSBVM_VOID_TUPLE);

    for(size_t i = 0; i < expressionCount; ++i)
    {
        gcFrame.expressionNode = sysbvm_array_at((*sequenceNode)->expressions, i);
        gcFrame.result = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, gcFrame.expressionNode);
    }

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astTupleSlotNamedAtNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *compiler = &arguments[1];

    sysbvm_astTupleSlotNamedAtNode_t **slotNamedNode = (sysbvm_astTupleSlotNamedAtNode_t**)node;
    struct {
        sysbvm_tuple_t tuple;
        sysbvm_tuple_t slot;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.tuple = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, (*slotNamedNode)->tupleExpression);
    gcFrame.slot = sysbvm_bytecodeCompiler_addLiteral(context, *compiler, (*slotNamedNode)->boundSlot);
    gcFrame.result = sysbvm_bytecodeCompiler_newTemporary(context, *compiler);
    if(sysbvm_type_isPointerLikeType(sysbvm_astNode_getAnalyzedType((*slotNamedNode)->tupleExpression)))
        sysbvm_bytecodeCompiler_refSlotAt(context, *compiler, gcFrame.result, gcFrame.tuple, gcFrame.slot);
    else
        sysbvm_bytecodeCompiler_slotAt(context, *compiler, gcFrame.result, gcFrame.tuple, gcFrame.slot);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astTupleSlotNamedReferenceAtNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *compiler = &arguments[1];

    sysbvm_astTupleSlotNamedReferenceAtNode_t **slotNamedNode = (sysbvm_astTupleSlotNamedReferenceAtNode_t**)node;
    struct {
        sysbvm_tuple_t tuple;
        sysbvm_tuple_t slot;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.tuple = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, (*slotNamedNode)->tupleExpression);
    gcFrame.slot = sysbvm_bytecodeCompiler_addLiteral(context, *compiler, (*slotNamedNode)->boundSlot);
    gcFrame.result = sysbvm_bytecodeCompiler_newTemporary(context, *compiler);
    if(sysbvm_type_isPointerLikeType(sysbvm_astNode_getAnalyzedType((*slotNamedNode)->tupleExpression)))
        sysbvm_bytecodeCompiler_refSlotReferenceAt(context, *compiler, gcFrame.result, gcFrame.tuple, gcFrame.slot);
    else
        sysbvm_bytecodeCompiler_slotReferenceAt(context, *compiler, gcFrame.result, gcFrame.tuple, gcFrame.slot);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astTupleSlotNamedAtPutNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *compiler = &arguments[1];

    sysbvm_astTupleSlotNamedAtPutNode_t **slotNamedNode = (sysbvm_astTupleSlotNamedAtPutNode_t**)node;
    struct {
        sysbvm_tuple_t tuple;
        sysbvm_tuple_t slot;
        sysbvm_tuple_t value;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.tuple = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, (*slotNamedNode)->tupleExpression);
    gcFrame.slot = sysbvm_bytecodeCompiler_addLiteral(context, *compiler, (*slotNamedNode)->boundSlot);
    gcFrame.value = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, (*slotNamedNode)->valueExpression);
    if(sysbvm_type_isPointerLikeType(sysbvm_astNode_getAnalyzedType((*slotNamedNode)->tupleExpression)))
        sysbvm_bytecodeCompiler_refSlotAtPut(context, *compiler, gcFrame.tuple, gcFrame.slot, gcFrame.value);
    else
        sysbvm_bytecodeCompiler_slotAtPut(context, *compiler, gcFrame.tuple, gcFrame.slot, gcFrame.value);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astUseNamedSlotsOfNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *compiler = &arguments[1];

    sysbvm_astUseNamedSlotsOfNode_t **usedNamedSlots = (sysbvm_astUseNamedSlotsOfNode_t**)node;
    struct {
        sysbvm_tuple_t tuple;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.tuple = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, (*usedNamedSlots)->tupleExpression);
    sysbvm_bytecodeCompiler_setBindingValue(context, *compiler, (*usedNamedSlots)->binding, gcFrame.tuple);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return sysbvm_bytecodeCompiler_addLiteral(context, *compiler, SYSBVM_VOID_TUPLE);
}

static sysbvm_tuple_t sysbvm_astWhileContinueNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *compiler = &arguments[1];

    sysbvm_astWhileContinueWithNode_t **whileNode = (sysbvm_astWhileContinueWithNode_t**)node;
    struct {
        sysbvm_tuple_t whileEntryLabel;
        sysbvm_tuple_t whileBodyLabel;
        sysbvm_tuple_t whileContinue;
        sysbvm_tuple_t whileMergeLabel;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.whileEntryLabel = sysbvm_bytecodeCompilerInstruction_createLabel(context);
    gcFrame.whileBodyLabel = sysbvm_bytecodeCompilerInstruction_createLabel(context);
    gcFrame.whileContinue = sysbvm_bytecodeCompilerInstruction_createLabel(context);
    gcFrame.whileMergeLabel = sysbvm_bytecodeCompilerInstruction_createLabel(context);

    // While condition block.
    sysbvm_bytecodeCompiler_addInstruction(*compiler, gcFrame.whileEntryLabel);
    if((*whileNode)->conditionExpression)
    {
        sysbvm_tuple_t condition = sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, (*whileNode)->conditionExpression);
        sysbvm_bytecodeCompiler_jumpIfFalse(context, *compiler, condition, gcFrame.whileMergeLabel);
    }
    else
    {
        sysbvm_bytecodeCompiler_jump(context, *compiler, gcFrame.whileBodyLabel);
    }

    // While body.
    sysbvm_bytecodeCompiler_addInstruction(*compiler, gcFrame.whileBodyLabel);
    if((*whileNode)->bodyExpression)
        sysbvm_bytecodeCompiler_compileASTNodeWithBreakAndContinue(context, *compiler, (*whileNode)->bodyExpression, gcFrame.whileMergeLabel, gcFrame.whileContinue);
    sysbvm_bytecodeCompiler_jump(context, *compiler, gcFrame.whileContinue);

    // While continue
    sysbvm_bytecodeCompiler_addInstruction(*compiler, gcFrame.whileContinue);
    if((*whileNode)->continueExpression)
        sysbvm_bytecodeCompiler_compileASTNode(context, *compiler, (*whileNode)->continueExpression);

    sysbvm_bytecodeCompiler_jump(context, *compiler, gcFrame.whileEntryLabel);

    // While merge
    sysbvm_bytecodeCompiler_addInstruction(*compiler, gcFrame.whileMergeLabel);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return sysbvm_bytecodeCompiler_addLiteral(context, *compiler, SYSBVM_VOID_TUPLE);
}

static void sysbvm_bytecodeCompiler_setupNodeCompilationFunction(sysbvm_context_t *context, sysbvm_tuple_t astNodeType, sysbvm_functionEntryPoint_t compilationFunction)
{
    sysbvm_type_setMethodWithSelector(context, astNodeType, context->roots.astNodeCompileIntoBytecodeSelector, sysbvm_function_createPrimitive(context, 2, SYSBVM_FUNCTION_FLAGS_NONE, NULL, compilationFunction));
}

void sysbvm_bytecodeCompiler_registerPrimitives(void)
{
    sysbvm_primitiveTable_registerFunction(sysbvm_astBreakNode_primitiveCompileIntoBytecode, "ASTBreakNode::compileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astCoerceValueNode_primitiveCompileIntoBytecode, "ASTCoerceValueNode::compileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astContinueNode_primitiveCompileIntoBytecode, "ASTContinueNode::compileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astDoWhileContinueWithNode_primitiveCompileIntoBytecode, "ASTDoWhileContinueWithNode::compileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astDownCastNode_primitiveCompileIntoBytecode, "ASTDownCastNode::compileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astFunctionApplicationNode_primitiveCompileIntoBytecode, "ASTFunctionApplicationNode::compileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astIdentifierReferenceNode_primitiveCompileIntoBytecode, "ASTIdentifierReferenceNode::compileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astIfNode_primitiveCompileIntoBytecode, "ASTIfNode::compileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astLambdaNode_primitiveCompileIntoBytecode, "ASTLambdaNode::compileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astLexicalBlockNode_primitiveCompileIntoBytecode, "ASTLexicalBlockNode::compileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astLiteralNode_primitiveCompileIntoBytecode, "ASTLiteralNode::compileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astLocalDefinitionNode_primitiveCompileIntoBytecode, "ASTLocalDefinitionNode::compileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astMakeArrayNode_primitiveCompileIntoBytecode, "ASTMakeArrayNode::compileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astMakeAssociationNode_primitiveCompileIntoBytecode, "ASTMakeAssociationNode::compileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astMakeByteArrayNode_primitiveCompileIntoBytecode, "ASTMakeByteArrayNode::compileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astMakeDictionaryNode_primitiveCompileIntoBytecode, "ASTMakeDictionaryNode::compileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astMessageSendNode_primitiveCompileIntoBytecode, "ASTMessageSendNode::compileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astReturnNode_primitiveCompileIntoBytecode, "ASTReturnNode::compileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astSequenceNode_primitiveCompileIntoBytecode, "ASTSequenceNode::compileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astTupleSlotNamedAtNode_primitiveCompileIntoBytecode, "ASTTupleSlotNamedAtNode::compileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astTupleSlotNamedAtPutNode_primitiveCompileIntoBytecode, "ASTTupleSlotNamedAtPutNode::compileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astTupleSlotNamedReferenceAtNode_primitiveCompileIntoBytecode, "ASTTupleSlotNamedReferenceAtNode::compileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astUseNamedSlotsOfNode_primitiveCompileIntoBytecode, "ASTUseNamedSlotsOfNode::compileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astWhileContinueNode_primitiveCompileIntoBytecode, "ASTWhileNodeNode::compileIntoBytecodeWith:");
}

void sysbvm_bytecodeCompiler_setupPrimitives(sysbvm_context_t *context)
{
    sysbvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astBreakNodeType, &sysbvm_astBreakNode_primitiveCompileIntoBytecode);
    sysbvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astCoerceValueNodeType, &sysbvm_astCoerceValueNode_primitiveCompileIntoBytecode);
    sysbvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astContinueNodeType, &sysbvm_astContinueNode_primitiveCompileIntoBytecode);
    sysbvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astDoWhileContinueWithNodeType, &sysbvm_astDoWhileContinueWithNode_primitiveCompileIntoBytecode);
    sysbvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astDownCastNodeType, &sysbvm_astDownCastNode_primitiveCompileIntoBytecode);
    sysbvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astFunctionApplicationNodeType, &sysbvm_astFunctionApplicationNode_primitiveCompileIntoBytecode);
    sysbvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astIdentifierReferenceNodeType, &sysbvm_astIdentifierReferenceNode_primitiveCompileIntoBytecode);
    sysbvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astIfNodeType, &sysbvm_astIfNode_primitiveCompileIntoBytecode);
    sysbvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astLambdaNodeType, &sysbvm_astLambdaNode_primitiveCompileIntoBytecode);
    sysbvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astLexicalBlockNodeType, &sysbvm_astLexicalBlockNode_primitiveCompileIntoBytecode);
    sysbvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astLiteralNodeType, &sysbvm_astLiteralNode_primitiveCompileIntoBytecode);
    sysbvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astLocalDefinitionNodeType, &sysbvm_astLocalDefinitionNode_primitiveCompileIntoBytecode);
    sysbvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astMakeArrayNodeType, &sysbvm_astMakeArrayNode_primitiveCompileIntoBytecode);
    sysbvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astMakeAssociationNodeType, &sysbvm_astMakeAssociationNode_primitiveCompileIntoBytecode);
    sysbvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astMakeByteArrayNodeType, &sysbvm_astMakeByteArrayNode_primitiveCompileIntoBytecode);
    sysbvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astMakeDictionaryNodeType, &sysbvm_astMakeDictionaryNode_primitiveCompileIntoBytecode);
    sysbvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astMessageSendNodeType, &sysbvm_astMessageSendNode_primitiveCompileIntoBytecode);
    sysbvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astReturnNodeType, &sysbvm_astReturnNode_primitiveCompileIntoBytecode);
    sysbvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astSequenceNodeType, &sysbvm_astSequenceNode_primitiveCompileIntoBytecode);
    sysbvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astTupleSlotNamedAtNodeType, &sysbvm_astTupleSlotNamedAtNode_primitiveCompileIntoBytecode);
    sysbvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astTupleSlotNamedAtPutNodeType, &sysbvm_astTupleSlotNamedAtPutNode_primitiveCompileIntoBytecode);
    sysbvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astTupleSlotNamedReferenceAtNodeType, &sysbvm_astTupleSlotNamedReferenceAtNode_primitiveCompileIntoBytecode);
    sysbvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astUseNamedSlotsOfNodeType, &sysbvm_astUseNamedSlotsOfNode_primitiveCompileIntoBytecode);
    sysbvm_bytecodeCompiler_setupNodeCompilationFunction(context, context->roots.astWhileContinueWithNodeType, &sysbvm_astWhileContinueNode_primitiveCompileIntoBytecode);
}
