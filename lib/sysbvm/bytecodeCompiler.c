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

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssemblerInstruction_create(sysbvm_context_t *context, uint8_t opcode, sysbvm_tuple_t operands)
{
    sysbvm_functionBytecodeAssemblerInstruction_t *result = (sysbvm_functionBytecodeAssemblerInstruction_t*)sysbvm_context_allocatePointerTuple(context, context->roots.functionBytecodeAssemblerInstruction, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_functionBytecodeAssemblerInstruction_t));
    result->standardOpcode = sysbvm_tuple_uint8_encode(opcode);
    result->operands = operands;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssemblerInstruction_createLabel(sysbvm_context_t *context)
{
    sysbvm_functionBytecodeAssemblerLabel_t *result = (sysbvm_functionBytecodeAssemblerLabel_t*)sysbvm_context_allocatePointerTuple(context, context->roots.functionBytecodeAssemblerLabel, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_functionBytecodeAssemblerLabel_t));
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssemblerVectorOperand_create(sysbvm_context_t *context, sysbvm_operandVectorName_t vectorName, int16_t vectorIndex)
{
    sysbvm_functionBytecodeAssemblerVectorOperand_t *result = (sysbvm_functionBytecodeAssemblerVectorOperand_t*)sysbvm_context_allocatePointerTuple(context, context->roots.functionBytecodeAssemblerVectorOperand, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_functionBytecodeAssemblerVectorOperand_t));
    result->vectorType = sysbvm_tuple_int16_encode(vectorName);
    result->index = sysbvm_tuple_int16_encode(vectorIndex);
    result->hasAllocaDestination = SYSBVM_FALSE_TUPLE;
    result->hasNonAllocaDestination = SYSBVM_FALSE_TUPLE;
    result->hasSlotReferenceAtDestination = SYSBVM_FALSE_TUPLE;
    result->hasNonSlotReferenceAtDestination = SYSBVM_FALSE_TUPLE;
    result->hasLoadStoreUsage = SYSBVM_FALSE_TUPLE;
    result->hasNonLoadStoreUsage = SYSBVM_FALSE_TUPLE;
    result->allocaPointerRankIsLowered = SYSBVM_FALSE_TUPLE;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_create(sysbvm_context_t *context)
{
    sysbvm_functionBytecodeAssembler_t *result = (sysbvm_functionBytecodeAssembler_t*)sysbvm_context_allocatePointerTuple(context, context->roots.functionBytecodeAssembler, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_functionBytecodeAssembler_t));
    result->arguments = sysbvm_array_create(context, 0);
    result->captures = sysbvm_array_create(context, 0);
    result->literals = sysbvm_orderedCollection_create(context);
    result->literalDictionary = sysbvm_identityDictionary_create(context);
    result->temporaries = sysbvm_orderedCollection_create(context);
    result->temporaryTypes = sysbvm_orderedCollection_create(context);
    result->usedTemporaryCount = sysbvm_tuple_size_encode(context, 0);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeDirectCompiler_create(sysbvm_context_t *context)
{
    sysbvm_functionBytecodeDirectCompiler_t *result = (sysbvm_functionBytecodeDirectCompiler_t*)sysbvm_context_allocatePointerTuple(context, context->roots.functionBytecodeDirectCompiler, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_functionBytecodeDirectCompiler_t));
    result->assembler = (sysbvm_functionBytecodeAssembler_t*)sysbvm_functionBytecodeAssembler_create(context);
    result->bindingDictionary = sysbvm_methodDictionary_create(context);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API void sysbvm_functionBytecodeAssembler_setArgumentCount(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, size_t argumentCount)
{
    sysbvm_tuple_t arguments = sysbvm_array_create(context, argumentCount);
    for(size_t i = 0; i < argumentCount; ++i )
    {
        sysbvm_tuple_t argumentOperand = sysbvm_functionBytecodeAssemblerVectorOperand_create(context, SYSBVM_OPERAND_VECTOR_ARGUMENTS, (int16_t)i);
        sysbvm_array_atPut(arguments, i, argumentOperand);
    }

    assembler->arguments = arguments;
}

SYSBVM_API void sysbvm_functionBytecodeAssembler_setCaptureCount(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, size_t argumentCount)
{
    sysbvm_tuple_t captures = sysbvm_array_create(context, argumentCount);
    for(size_t i = 0; i < argumentCount; ++i )
    {
        sysbvm_tuple_t captureOperand = sysbvm_functionBytecodeAssemblerVectorOperand_create(context, SYSBVM_OPERAND_VECTOR_CAPTURES, (int16_t)i);
        sysbvm_array_atPut(captures, i, captureOperand);
    }

    assembler->captures = captures;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_addLiteral(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t literalValue)
{
    sysbvm_tuple_t existent = SYSBVM_NULL_TUPLE;
    if(sysbvm_identityDictionary_find(assembler->literalDictionary, literalValue, &existent))
        return existent;

    size_t literalIndex = sysbvm_orderedCollection_getSize(assembler->literals);
    sysbvm_tuple_t newLiteralOperand = sysbvm_functionBytecodeAssemblerVectorOperand_create(context, SYSBVM_OPERAND_VECTOR_LITERAL, (int16_t)literalIndex);
    sysbvm_orderedCollection_add(context, assembler->literals, literalValue);
    sysbvm_identityDictionary_atPut(context, assembler->literalDictionary, literalValue, newLiteralOperand);
    return newLiteralOperand;
}

SYSBVM_API void sysbvm_functionBytecodeAssembler_addInstruction(sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t instruction)
{
    sysbvm_functionBytecodeAssemblerAbstractInstruction_t *instructionObject = (sysbvm_functionBytecodeAssemblerAbstractInstruction_t*)instruction;

    instructionObject->previous = assembler->lastInstruction;

    if(assembler->lastInstruction)
    {
        ((sysbvm_functionBytecodeAssemblerAbstractInstruction_t*)assembler->lastInstruction)->next = instructionObject;
    }
    else
    {
        assembler->firstInstruction = instructionObject;
    }

    assembler->lastInstruction = instructionObject;

    instructionObject->sourceASTNode = assembler->sourceASTNode;
    instructionObject->sourceEnvironment = assembler->sourceEnvironment;
    instructionObject->sourcePosition = assembler->sourcePosition;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeDirectCompiler_compileASTNode(sysbvm_context_t *context, sysbvm_functionBytecodeDirectCompiler_t *compiler_, sysbvm_tuple_t astNode_)
{
    struct {
        sysbvm_functionBytecodeDirectCompiler_t *compiler;
        sysbvm_tuple_t astNode;
        sysbvm_tuple_t result;

        sysbvm_tuple_t oldSourcePosition;
        sysbvm_tuple_t oldSourceASTNode;
    } gcFrame = {
        .compiler = compiler_,
        .astNode = astNode_
    };

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.oldSourcePosition = gcFrame.compiler->assembler->sourcePosition;
    gcFrame.oldSourceASTNode = gcFrame.compiler->assembler->sourceASTNode;

    gcFrame.compiler->assembler->sourcePosition = sysbvm_astNode_getSourcePosition(gcFrame.astNode);
    gcFrame.compiler->assembler->sourceASTNode = gcFrame.astNode;

    gcFrame.result = sysbvm_tuple_send1(context, context->roots.astNodeCompileIntoBytecodeSelector, gcFrame.astNode, (sysbvm_tuple_t)gcFrame.compiler);

    gcFrame.compiler->assembler->sourcePosition = gcFrame.oldSourcePosition;
    gcFrame.compiler->assembler->sourceASTNode = gcFrame.oldSourceASTNode;

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeDirectCompiler_compileASTNodeWithBreakAndContinue(sysbvm_context_t *context, sysbvm_functionBytecodeDirectCompiler_t *compiler_, sysbvm_tuple_t astNode, sysbvm_tuple_t breakLabel, sysbvm_tuple_t continueLabel)
{
    struct {
        sysbvm_functionBytecodeDirectCompiler_t *compiler;
        sysbvm_tuple_t oldBreakLabel;
        sysbvm_tuple_t oldContinueLabel;
        sysbvm_tuple_t result;
    } gcFrame = {
        .compiler = compiler_,
    };

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.oldBreakLabel = gcFrame.compiler->breakLabel;
    gcFrame.oldContinueLabel = gcFrame.compiler->continueLabel;
    gcFrame.compiler->breakLabel = breakLabel;
    gcFrame.compiler->continueLabel = continueLabel;

    gcFrame.result = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, gcFrame.compiler, astNode);

    gcFrame.compiler->breakLabel = gcFrame.oldBreakLabel;
    gcFrame.compiler->continueLabel = gcFrame.oldContinueLabel;

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeDirectCompiler_compileASTNodeWithEnvironment(sysbvm_context_t *context, sysbvm_functionBytecodeDirectCompiler_t *compiler_, sysbvm_tuple_t astNode, sysbvm_tuple_t environment)
{
    struct {
        sysbvm_functionBytecodeDirectCompiler_t *compiler;
        sysbvm_tuple_t oldEnvironment;
        sysbvm_tuple_t result;
    } gcFrame = {
        .compiler = compiler_,
    };

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.oldEnvironment = gcFrame.compiler->assembler->sourceEnvironment;
    gcFrame.compiler->assembler->sourceEnvironment = environment;

    gcFrame.result = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, gcFrame.compiler, astNode);

    gcFrame.compiler->assembler->sourceEnvironment = gcFrame.oldEnvironment;

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

SYSBVM_API void sysbvm_functionBytecodeDirectCompiler_setBindingValue(sysbvm_context_t *context, sysbvm_functionBytecodeDirectCompiler_t *compiler, sysbvm_tuple_t binding, sysbvm_tuple_t value)
{
    sysbvm_methodDictionary_atPut(context, compiler->bindingDictionary, binding, value);
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeDirectCompiler_getBindingValue(sysbvm_context_t *context, sysbvm_functionBytecodeDirectCompiler_t *compiler, sysbvm_tuple_t binding)
{
    (void)context;
    if(sysbvm_tuple_isKindOf(context, binding, context->roots.symbolTupleSlotBindingType))
    {
        sysbvm_symbolTupleSlotBinding_t *bindingObject = (sysbvm_symbolTupleSlotBinding_t*)binding;
        sysbvm_tuple_t tupleValue = sysbvm_functionBytecodeDirectCompiler_getBindingValue(context, compiler, bindingObject->tupleBinding);
        sysbvm_tuple_t reference = sysbvm_functionBytecodeAssembler_newTemporary(context, compiler->assembler, sysbvm_typeSlot_getValidReferenceType(context, bindingObject->typeSlot));

        sysbvm_symbolBinding_t *tupleBindingObject = (sysbvm_symbolBinding_t*)bindingObject->tupleBinding;

        if(sysbvm_type_isPointerLikeType(tupleBindingObject->type))
            sysbvm_functionBytecodeAssembler_refSlotReferenceAt(context, compiler->assembler, reference, tupleValue, sysbvm_functionBytecodeAssembler_addLiteral(context, compiler->assembler, bindingObject->typeSlot));
        else
            sysbvm_functionBytecodeAssembler_slotReferenceAt(context, compiler->assembler, reference, tupleValue, sysbvm_functionBytecodeAssembler_addLiteral(context, compiler->assembler, bindingObject->typeSlot));
        return reference;
    }

    if(sysbvm_symbolBinding_isValue(context, binding))
    {
        sysbvm_tuple_t valueTemp = sysbvm_functionBytecodeAssembler_newTemporary(context, compiler->assembler, sysbvm_symbolBinding_getType(binding));
        sysbvm_tuple_t bindingLiteral = sysbvm_functionBytecodeAssembler_addLiteral(context, compiler->assembler, binding);
        sysbvm_functionBytecodeAssembler_loadSymbolValueBinding(context, compiler->assembler, valueTemp, bindingLiteral);
        return valueTemp;
    }

    sysbvm_tuple_t value = SYSBVM_NULL_TUPLE;
    sysbvm_functionBytecodeDirectCompiler_t *compilerObject = (sysbvm_functionBytecodeDirectCompiler_t*)compiler;
    if(!sysbvm_methodDictionary_find(compilerObject->bindingDictionary, binding, &value))
        sysbvm_error("Invalid value binding.");

    return value;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_newTemporary(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t type)
{
    size_t temporaryIndex = sysbvm_orderedCollection_getSize(assembler->temporaries);
    sysbvm_tuple_t temporaryOperand = sysbvm_functionBytecodeAssemblerVectorOperand_create(context, SYSBVM_OPERAND_VECTOR_LOCAL, (int16_t)temporaryIndex);
    sysbvm_orderedCollection_add(context, assembler->temporaries, temporaryOperand);
    sysbvm_orderedCollection_add(context, assembler->temporaryTypes, type);
    return temporaryOperand;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_allocaWithValue(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t pointerLikeType, sysbvm_tuple_t value)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 3);
    sysbvm_array_atPut(operands, 0, result);
    sysbvm_array_atPut(operands, 1, pointerLikeType);
    sysbvm_array_atPut(operands, 2, value);

    sysbvm_tuple_t instruction = sysbvm_functionBytecodeAssemblerInstruction_create(context, SYSBVM_OPCODE_ALLOCA_WITH_VALUE, operands);
    sysbvm_functionBytecodeAssembler_addInstruction(assembler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_coerceValue(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t type, sysbvm_tuple_t value)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 3);
    sysbvm_array_atPut(operands, 0, result);
    sysbvm_array_atPut(operands, 1, type);
    sysbvm_array_atPut(operands, 2, value);

    sysbvm_tuple_t instruction = sysbvm_functionBytecodeAssemblerInstruction_create(context, SYSBVM_OPCODE_COERCE_VALUE, operands);
    sysbvm_functionBytecodeAssembler_addInstruction(assembler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_downCastValue(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t type, sysbvm_tuple_t value)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 3);
    sysbvm_array_atPut(operands, 0, result);
    sysbvm_array_atPut(operands, 1, type);
    sysbvm_array_atPut(operands, 2, value);

    sysbvm_tuple_t instruction = sysbvm_functionBytecodeAssemblerInstruction_create(context, SYSBVM_OPCODE_DOWNCAST_VALUE, operands);
    sysbvm_functionBytecodeAssembler_addInstruction(assembler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_uncheckedDownCastValue(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t type, sysbvm_tuple_t value)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 3);
    sysbvm_array_atPut(operands, 0, result);
    sysbvm_array_atPut(operands, 1, type);
    sysbvm_array_atPut(operands, 2, value);

    sysbvm_tuple_t instruction = sysbvm_functionBytecodeAssemblerInstruction_create(context, SYSBVM_OPCODE_UNCHECKED_DOWNCAST_VALUE, operands);
    sysbvm_functionBytecodeAssembler_addInstruction(assembler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_jump(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t destination)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 1);
    sysbvm_array_atPut(operands, 0, destination);

    sysbvm_tuple_t instruction = sysbvm_functionBytecodeAssemblerInstruction_create(context, SYSBVM_OPCODE_JUMP, operands);
    sysbvm_functionBytecodeAssembler_addInstruction(assembler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_jumpIfFalse(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t condition, sysbvm_tuple_t destination)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 2);
    sysbvm_array_atPut(operands, 0, condition);
    sysbvm_array_atPut(operands, 1, destination);

    sysbvm_tuple_t instruction = sysbvm_functionBytecodeAssemblerInstruction_create(context, SYSBVM_OPCODE_JUMP_IF_FALSE, operands);
    sysbvm_functionBytecodeAssembler_addInstruction(assembler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_setDebugValue(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t value, sysbvm_tuple_t binding)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 2);
    sysbvm_array_atPut(operands, 0, value);
    sysbvm_array_atPut(operands, 1, binding);

    sysbvm_tuple_t instruction = sysbvm_functionBytecodeAssemblerInstruction_create(context, SYSBVM_OPCODE_SET_DEBUG_VALUE, operands);
    sysbvm_functionBytecodeAssembler_addInstruction(assembler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_load(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t destination, sysbvm_tuple_t pointer)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 2);
    sysbvm_array_atPut(operands, 0, destination);
    sysbvm_array_atPut(operands, 1, pointer);

    sysbvm_tuple_t instruction = sysbvm_functionBytecodeAssemblerInstruction_create(context, SYSBVM_OPCODE_LOAD, operands);
    sysbvm_functionBytecodeAssembler_addInstruction(assembler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_loadSymbolValueBinding(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t destination, sysbvm_tuple_t symbolValueBinding)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 2);
    sysbvm_array_atPut(operands, 0, destination);
    sysbvm_array_atPut(operands, 1, symbolValueBinding);

    sysbvm_tuple_t instruction = sysbvm_functionBytecodeAssemblerInstruction_create(context, SYSBVM_OPCODE_LOAD_SYMBOL_VALUE_BINDING, operands);
    sysbvm_functionBytecodeAssembler_addInstruction(assembler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_store(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t pointer, sysbvm_tuple_t value)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 2);
    sysbvm_array_atPut(operands, 0, pointer);
    sysbvm_array_atPut(operands, 1, value);

    sysbvm_tuple_t instruction = sysbvm_functionBytecodeAssemblerInstruction_create(context, SYSBVM_OPCODE_STORE, operands);
    sysbvm_functionBytecodeAssembler_addInstruction(assembler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_slotAt(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 3);
    sysbvm_array_atPut(operands, 0, result);
    sysbvm_array_atPut(operands, 1, tuple);
    sysbvm_array_atPut(operands, 2, typeSlot);

    sysbvm_tuple_t instruction = sysbvm_functionBytecodeAssemblerInstruction_create(context, SYSBVM_OPCODE_SLOT_AT, operands);
    sysbvm_functionBytecodeAssembler_addInstruction(assembler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_slotReferenceAt(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 3);
    sysbvm_array_atPut(operands, 0, result);
    sysbvm_array_atPut(operands, 1, tuple);
    sysbvm_array_atPut(operands, 2, typeSlot);

    sysbvm_tuple_t instruction = sysbvm_functionBytecodeAssemblerInstruction_create(context, SYSBVM_OPCODE_SLOT_REFERENCE_AT, operands);
    sysbvm_functionBytecodeAssembler_addInstruction(assembler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_slotAtPut(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot, sysbvm_tuple_t value)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 3);
    sysbvm_array_atPut(operands, 0, tuple);
    sysbvm_array_atPut(operands, 1, typeSlot);
    sysbvm_array_atPut(operands, 2, value);

    sysbvm_tuple_t instruction = sysbvm_functionBytecodeAssemblerInstruction_create(context, SYSBVM_OPCODE_SLOT_AT_PUT, operands);
    sysbvm_functionBytecodeAssembler_addInstruction(assembler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_refSlotAt(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 3);
    sysbvm_array_atPut(operands, 0, result);
    sysbvm_array_atPut(operands, 1, tuple);
    sysbvm_array_atPut(operands, 2, typeSlot);

    sysbvm_tuple_t instruction = sysbvm_functionBytecodeAssemblerInstruction_create(context, SYSBVM_OPCODE_REF_SLOT_AT, operands);
    sysbvm_functionBytecodeAssembler_addInstruction(assembler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_refSlotReferenceAt(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 3);
    sysbvm_array_atPut(operands, 0, result);
    sysbvm_array_atPut(operands, 1, tuple);
    sysbvm_array_atPut(operands, 2, typeSlot);

    sysbvm_tuple_t instruction = sysbvm_functionBytecodeAssemblerInstruction_create(context, SYSBVM_OPCODE_REF_SLOT_REFERENCE_AT, operands);
    sysbvm_functionBytecodeAssembler_addInstruction(assembler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_refSlotAtPut(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot, sysbvm_tuple_t value)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 3);
    sysbvm_array_atPut(operands, 0, tuple);
    sysbvm_array_atPut(operands, 1, typeSlot);
    sysbvm_array_atPut(operands, 2, value);

    sysbvm_tuple_t instruction = sysbvm_functionBytecodeAssemblerInstruction_create(context, SYSBVM_OPCODE_REF_SLOT_AT_PUT, operands);
    sysbvm_functionBytecodeAssembler_addInstruction(assembler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_jumpIfTrue(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t condition, sysbvm_tuple_t destination)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 2);
    sysbvm_array_atPut(operands, 0, condition);
    sysbvm_array_atPut(operands, 1, destination);

    sysbvm_tuple_t instruction = sysbvm_functionBytecodeAssemblerInstruction_create(context, SYSBVM_OPCODE_JUMP_IF_TRUE, operands);
    sysbvm_functionBytecodeAssembler_addInstruction(assembler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_countExtension(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, size_t count)
{
    if(count == 0)
        return SYSBVM_NULL_TUPLE;

    sysbvm_tuple_t operands = sysbvm_array_create(context, 1);
    sysbvm_array_atPut(operands, 0, sysbvm_tuple_int16_encode(count));

    sysbvm_tuple_t instruction = sysbvm_functionBytecodeAssemblerInstruction_create(context, SYSBVM_OPCODE_COUNT_EXTENSION, operands);
    sysbvm_functionBytecodeAssembler_addInstruction(assembler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_call(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t function, sysbvm_tuple_t arguments)
{
    size_t argumentCount = sysbvm_array_getSize(arguments);
    sysbvm_functionBytecodeAssembler_countExtension(context, assembler, argumentCount>>4);

    sysbvm_tuple_t operands = sysbvm_array_create(context, 2 + argumentCount);
    sysbvm_array_atPut(operands, 0, result);
    sysbvm_array_atPut(operands, 1, function);
    for(size_t i = 0; i < argumentCount; ++i)
        sysbvm_array_atPut(operands, 2 + i, sysbvm_array_at(arguments, i));
    
    sysbvm_tuple_t instruction = sysbvm_functionBytecodeAssemblerInstruction_create(context, SYSBVM_OPCODE_CALL + (argumentCount & 0xF), operands);
    sysbvm_functionBytecodeAssembler_addInstruction(assembler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_uncheckedCall(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t function, sysbvm_tuple_t arguments)
{
    size_t argumentCount = sysbvm_array_getSize(arguments);
    sysbvm_functionBytecodeAssembler_countExtension(context, assembler, argumentCount>>4);

    sysbvm_tuple_t operands = sysbvm_array_create(context, 2 + argumentCount);
    sysbvm_array_atPut(operands, 0, result);
    sysbvm_array_atPut(operands, 1, function);
    for(size_t i = 0; i < argumentCount; ++i)
        sysbvm_array_atPut(operands, 2 + i, sysbvm_array_at(arguments, i));
    
    sysbvm_tuple_t instruction = sysbvm_functionBytecodeAssemblerInstruction_create(context, SYSBVM_OPCODE_UNCHECKED_CALL + (argumentCount & 0xF), operands);
    sysbvm_functionBytecodeAssembler_addInstruction(assembler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_send(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t selector, sysbvm_tuple_t receiver, sysbvm_tuple_t arguments)
{
    size_t argumentCount = sysbvm_array_getSize(arguments);
    sysbvm_functionBytecodeAssembler_countExtension(context, assembler, argumentCount>>4);

    sysbvm_tuple_t operands = sysbvm_array_create(context, 3 + argumentCount);
    sysbvm_array_atPut(operands, 0, result);
    sysbvm_array_atPut(operands, 1, selector);
    sysbvm_array_atPut(operands, 2, receiver);
    for(size_t i = 0; i < argumentCount; ++i)
        sysbvm_array_atPut(operands, 3 + i, sysbvm_array_at(arguments, i));
    
    sysbvm_tuple_t instruction = sysbvm_functionBytecodeAssemblerInstruction_create(context, SYSBVM_OPCODE_SEND + (argumentCount & 0xF), operands);
    sysbvm_functionBytecodeAssembler_addInstruction(assembler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_sendWithLookupReceiverType(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t receiverLookupType, sysbvm_tuple_t selector, sysbvm_tuple_t receiver, sysbvm_tuple_t arguments)
{
    size_t argumentCount = sysbvm_array_getSize(arguments);
    sysbvm_functionBytecodeAssembler_countExtension(context, assembler, argumentCount>>4);

    sysbvm_tuple_t operands = sysbvm_array_create(context, 4 + argumentCount);
    sysbvm_array_atPut(operands, 0, result);
    sysbvm_array_atPut(operands, 1, receiverLookupType);
    sysbvm_array_atPut(operands, 2, selector);
    sysbvm_array_atPut(operands, 3, receiver);
    for(size_t i = 0; i < argumentCount; ++i)
        sysbvm_array_atPut(operands, 4 + i, sysbvm_array_at(arguments, i));
    
    sysbvm_tuple_t instruction = sysbvm_functionBytecodeAssemblerInstruction_create(context, SYSBVM_OPCODE_SEND_WITH_LOOKUP + (argumentCount & 0xF), operands);
    sysbvm_functionBytecodeAssembler_addInstruction(assembler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_makeArrayWithElements(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t elements)
{
    size_t elementCount = sysbvm_array_getSize(elements);
    sysbvm_functionBytecodeAssembler_countExtension(context, assembler, elementCount>>4);

    sysbvm_tuple_t operands = sysbvm_array_create(context, 1 + elementCount);
    sysbvm_array_atPut(operands, 0, result);
    for(size_t i = 0; i < elementCount; ++i)
        sysbvm_array_atPut(operands, 1 + i, sysbvm_array_at(elements, i));
    
    sysbvm_tuple_t instruction = sysbvm_functionBytecodeAssemblerInstruction_create(context, SYSBVM_OPCODE_MAKE_ARRAY_WITH_ELEMENTS + (elementCount & 0xF), operands);
    sysbvm_functionBytecodeAssembler_addInstruction(assembler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_makeAssociation(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t key, sysbvm_tuple_t value)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 3);
    sysbvm_array_atPut(operands, 0, result);
    sysbvm_array_atPut(operands, 1, key);
    sysbvm_array_atPut(operands, 2, value);
    
    sysbvm_tuple_t instruction = sysbvm_functionBytecodeAssemblerInstruction_create(context, SYSBVM_OPCODE_MAKE_ASSOCIATION, operands);
    sysbvm_functionBytecodeAssembler_addInstruction(assembler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_makeByteArrayWithElements(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t elements)
{
    size_t elementCount = sysbvm_array_getSize(elements);
    sysbvm_functionBytecodeAssembler_countExtension(context, assembler, elementCount>>4);

    sysbvm_tuple_t operands = sysbvm_array_create(context, 1 + elementCount);
    sysbvm_array_atPut(operands, 0, result);
    for(size_t i = 0; i < elementCount; ++i)
        sysbvm_array_atPut(operands, 1 + i, sysbvm_array_at(elements, i));
    
    sysbvm_tuple_t instruction = sysbvm_functionBytecodeAssemblerInstruction_create(context, SYSBVM_OPCODE_MAKE_BYTE_ARRAY_WITH_ELEMENTS + (elementCount & 0xF), operands);
    sysbvm_functionBytecodeAssembler_addInstruction(assembler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_makeDictionaryWithElements(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t elements)
{
    size_t elementCount = sysbvm_array_getSize(elements);
    sysbvm_functionBytecodeAssembler_countExtension(context, assembler, elementCount>>4);

    sysbvm_tuple_t operands = sysbvm_array_create(context, 1 + elementCount);
    sysbvm_array_atPut(operands, 0, result);
    for(size_t i = 0; i < elementCount; ++i)
        sysbvm_array_atPut(operands, 1 + i, sysbvm_array_at(elements, i));
    
    sysbvm_tuple_t instruction = sysbvm_functionBytecodeAssemblerInstruction_create(context, SYSBVM_OPCODE_MAKE_DICTIONARY_WITH_ELEMENTS + (elementCount & 0xF), operands);
    sysbvm_functionBytecodeAssembler_addInstruction(assembler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_makeClosureWithCaptures(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t result, sysbvm_tuple_t functionDefinition, sysbvm_tuple_t captures)
{
    size_t captureCount = sysbvm_array_getSize(captures);
    sysbvm_functionBytecodeAssembler_countExtension(context, assembler, captureCount>>4);

    sysbvm_tuple_t operands = sysbvm_array_create(context, 2 + captureCount);
    sysbvm_array_atPut(operands, 0, result);
    sysbvm_array_atPut(operands, 1, functionDefinition);
    for(size_t i = 0; i < captureCount; ++i)
        sysbvm_array_atPut(operands, 2 + i, sysbvm_array_at(captures, i));
    
    sysbvm_tuple_t instruction = sysbvm_functionBytecodeAssemblerInstruction_create(context, SYSBVM_OPCODE_MAKE_CLOSURE_WITH_CAPTURES + (captureCount & 0xF), operands);
    sysbvm_functionBytecodeAssembler_addInstruction(assembler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_move(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t destination, sysbvm_tuple_t value)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 2);
    sysbvm_array_atPut(operands, 0, destination);
    sysbvm_array_atPut(operands, 1, value);

    sysbvm_tuple_t instruction = sysbvm_functionBytecodeAssemblerInstruction_create(context, SYSBVM_OPCODE_MOVE, operands);
    sysbvm_functionBytecodeAssembler_addInstruction(assembler, instruction);
    return instruction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionBytecodeAssembler_return(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t value)
{
    sysbvm_tuple_t operands = sysbvm_array_create(context, 1);
    sysbvm_array_atPut(operands, 0, value);

    sysbvm_tuple_t instruction = sysbvm_functionBytecodeAssemblerInstruction_create(context, SYSBVM_OPCODE_RETURN, operands);
    sysbvm_functionBytecodeAssembler_addInstruction(assembler, instruction);
    return instruction;
}

static bool sysbvm_functionBytecodeAssemblerInstruction_isLabel(sysbvm_context_t *context, sysbvm_functionBytecodeAssemblerAbstractInstruction_t *instruction)
{
    return sysbvm_tuple_getType(context, (sysbvm_tuple_t)instruction) == context->roots.functionBytecodeAssemblerLabel;
}

static size_t sysbvm_functionBytecodeAssemblerInstruction_computeAssembledSize(sysbvm_context_t *context, sysbvm_functionBytecodeAssemblerAbstractInstruction_t *instruction)
{
    // Is this a label?
    if(sysbvm_functionBytecodeAssemblerInstruction_isLabel(context, instruction))
        return 0;

    return 1 + sysbvm_array_getSize(((sysbvm_functionBytecodeAssemblerInstruction_t*)instruction)->operands) * 2;
}

static size_t sysbvm_functionBytecodeAssemblerAbstractOperandFor_assembleInto(sysbvm_context_t *context, sysbvm_tuple_t operand, uint8_t *destination, sysbvm_functionBytecodeAssemblerInstruction_t *instruction)
{
    sysbvm_tuple_t operandType = sysbvm_tuple_getType(context, operand);
    if(operandType == context->roots.functionBytecodeAssemblerInstruction || operandType == context->roots.functionBytecodeAssemblerLabel)
    {
        sysbvm_functionBytecodeAssemblerAbstractInstruction_t *destinationInstruction = (sysbvm_functionBytecodeAssemblerAbstractInstruction_t*)operand;
        size_t destPC = sysbvm_tuple_size_decode(destinationInstruction->pc);
        size_t sourcePC = sysbvm_tuple_size_decode(instruction->super.endPC);
        int16_t pcDelta = (int16_t) (destPC - sourcePC);
        *destination++ = pcDelta & 0xFF;
        *destination++ = (pcDelta >> 8) & 0xFF;
        return 2;
    }
    else if(operandType == context->roots.functionBytecodeAssemblerVectorOperand)
    {
        sysbvm_functionBytecodeAssemblerVectorOperand_t *vectorOperand = (sysbvm_functionBytecodeAssemblerVectorOperand_t*)operand;
        int16_t vectorType = sysbvm_tuple_int16_decode(vectorOperand->vectorType);
        int16_t vectorIndex = sysbvm_tuple_int16_decode(vectorOperand->index);
        int16_t encodedOperand = (vectorIndex << SYSBVM_OPERAND_VECTOR_BITS) | vectorType;
        *destination++ = encodedOperand & 0xFF;
        *destination++ = (encodedOperand >> 8) & 0xFF;
        return 2;
    }
    else if(operandType == context->roots.int16Type)
    {
        int16_t operandValue = sysbvm_tuple_int16_decode(operand);
        *destination++ = operandValue & 0xFF;
        *destination++ = (operandValue >> 8) & 0xFF;
        return 2;
    }
    else
    {
        abort();
        return 0;
    }
}

static size_t sysbvm_functionBytecodeAssemblerInstruction_assembleInto(sysbvm_context_t *context, sysbvm_functionBytecodeAssemblerAbstractInstruction_t *instruction, uint8_t *destination)
{
    // Is this a label?
    if(sysbvm_functionBytecodeAssemblerInstruction_isLabel(context, instruction))
        return 0;

    sysbvm_functionBytecodeAssemblerInstruction_t *bytecodeInstruction = (sysbvm_functionBytecodeAssemblerInstruction_t*)instruction;

    size_t offset = 0;
    destination[offset++] = sysbvm_tuple_uint8_decode(bytecodeInstruction->standardOpcode);

    size_t operandCount = sysbvm_array_getSize(bytecodeInstruction->operands);
    for(size_t i = 0; i < operandCount; ++i)
    {
        sysbvm_tuple_t operand = sysbvm_array_at(bytecodeInstruction->operands, i);
        offset += sysbvm_functionBytecodeAssemblerAbstractOperandFor_assembleInto(context, operand, destination + offset, bytecodeInstruction);
    }

    return offset;
}

static void sysbvm_functionBytecodeAssembler_removeInstruction(sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_functionBytecodeAssemblerAbstractInstruction_t *instruction)
{
    sysbvm_functionBytecodeAssemblerAbstractInstruction_t *previous = instruction->previous;
    sysbvm_functionBytecodeAssemblerAbstractInstruction_t *next = instruction->next;

    if(previous)
        previous->next = next;
    else
        assembler->firstInstruction = next;

    if(next)
        next->previous = previous;
    else
        assembler->lastInstruction = previous;
}

static void sysbvm_functionBytecodeAssembler_optimizeJumps(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler)
{
    sysbvm_functionBytecodeAssemblerAbstractInstruction_t *instruction = assembler->firstInstruction;
    while(instruction)
    {
        sysbvm_functionBytecodeAssemblerAbstractInstruction_t *nextInstruction = instruction->next;
        if(!sysbvm_functionBytecodeAssemblerInstruction_isLabel(context, instruction))
        {
            sysbvm_functionBytecodeAssemblerInstruction_t *bytecodeInstruction = (sysbvm_functionBytecodeAssemblerInstruction_t*)instruction;
            uint8_t opcode = sysbvm_tuple_uint8_decode(bytecodeInstruction->standardOpcode);
            if(opcode == SYSBVM_OPCODE_JUMP || opcode == SYSBVM_OPCODE_JUMP_IF_TRUE || opcode == SYSBVM_OPCODE_JUMP_IF_FALSE)
            {
                sysbvm_tuple_t lastOperand = sysbvm_array_at(bytecodeInstruction->operands, sysbvm_array_getSize(bytecodeInstruction->operands) - 1);
                if(lastOperand == (sysbvm_tuple_t)nextInstruction)
                    sysbvm_functionBytecodeAssembler_removeInstruction(assembler, instruction);
            }
        }

        instruction = nextInstruction;
    }

}

static void sysbvm_functionBytecodeAssembler_markInstructionOperandUsages(sysbvm_context_t *context, sysbvm_functionBytecodeAssemblerAbstractInstruction_t *instruction)
{
    // Ignore labels.
    if(sysbvm_functionBytecodeAssemblerInstruction_isLabel(context, instruction))
        return;

    sysbvm_functionBytecodeAssemblerInstruction_t *bytecodeInstruction = (sysbvm_functionBytecodeAssemblerInstruction_t*)instruction;
    uint8_t opcode = sysbvm_tuple_uint8_decode(bytecodeInstruction->standardOpcode);
    if(opcode == SYSBVM_OPCODE_COUNT_EXTENSION)
        return;

    uint8_t destinationOperandCount = sysbvm_bytecodeInterpreter_destinationOperandCountForOpcode(opcode);

    size_t operandCount = sysbvm_array_getSize(bytecodeInstruction->operands);
    if(opcode == SYSBVM_OPCODE_JUMP || opcode == SYSBVM_OPCODE_JUMP_IF_TRUE || opcode == SYSBVM_OPCODE_JUMP_IF_FALSE)
        --operandCount;
    
    // Mark the destination operands.
    for(size_t i = 0; i < destinationOperandCount; ++i)
    {
        sysbvm_functionBytecodeAssemblerVectorOperand_t *operand = (sysbvm_functionBytecodeAssemblerVectorOperand_t*)sysbvm_array_at(bytecodeInstruction->operands, i);
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
        sysbvm_functionBytecodeAssemblerVectorOperand_t *operand = (sysbvm_functionBytecodeAssemblerVectorOperand_t*)sysbvm_array_at(bytecodeInstruction->operands, i);
        if(opcode == SYSBVM_OPCODE_LOAD)
            operand->hasLoadStoreUsage = SYSBVM_TRUE_TUPLE;
        else if(opcode == SYSBVM_OPCODE_STORE && i == 0)
            operand->hasLoadStoreUsage = SYSBVM_TRUE_TUPLE;
        else
            operand->hasNonLoadStoreUsage = SYSBVM_TRUE_TUPLE;
    }
}

static bool sysbvm_functionBytecodeAssembler_isLocalOnlyAlloca(sysbvm_tuple_t operand)
{
    sysbvm_functionBytecodeAssemblerVectorOperand_t *vectorOperand = (sysbvm_functionBytecodeAssemblerVectorOperand_t*)operand;

    bool hasAllocaDestination = sysbvm_tuple_boolean_decode(vectorOperand->hasAllocaDestination);
    bool hasNonAllocaDestination = sysbvm_tuple_boolean_decode(vectorOperand->hasNonAllocaDestination);
    bool hasLoadStoreUsage = sysbvm_tuple_boolean_decode(vectorOperand->hasLoadStoreUsage);
    bool hasNonLoadStoreUsage = sysbvm_tuple_boolean_decode(vectorOperand->hasNonLoadStoreUsage);
    
    return (hasAllocaDestination && !hasNonAllocaDestination) &&
        (hasLoadStoreUsage && !hasNonLoadStoreUsage);
}

static bool sysbvm_functionBytecodeAssembler_isLocalOnlyReferenceAt(sysbvm_tuple_t operand)
{
    sysbvm_functionBytecodeAssemblerVectorOperand_t *vectorOperand = (sysbvm_functionBytecodeAssemblerVectorOperand_t*)operand;

    bool hasSlotReferenceAtDestination = sysbvm_tuple_boolean_decode(vectorOperand->hasSlotReferenceAtDestination);
    bool hasNonSlotReferenceAtDestination = sysbvm_tuple_boolean_decode(vectorOperand->hasNonSlotReferenceAtDestination);
    bool hasLoadStoreUsage = sysbvm_tuple_boolean_decode(vectorOperand->hasLoadStoreUsage);
    bool hasNonLoadStoreUsage = sysbvm_tuple_boolean_decode(vectorOperand->hasNonLoadStoreUsage);
    
    return (hasSlotReferenceAtDestination && !hasNonSlotReferenceAtDestination) &&
        (hasLoadStoreUsage && !hasNonLoadStoreUsage);
}

static void sysbvm_functionBytecodeAssembler_lowerTemporaryPointerRank(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t operand)
{
    if(sysbvm_tuple_getType(context, operand) != context->roots.functionBytecodeAssemblerVectorOperand)
        return;

    sysbvm_functionBytecodeAssemblerVectorOperand_t *vectorOperand = (sysbvm_functionBytecodeAssemblerVectorOperand_t*)operand;
    if(sysbvm_tuple_boolean_decode(vectorOperand->allocaPointerRankIsLowered))
        return;

    int16_t index = sysbvm_tuple_int16_decode(vectorOperand->index);
    int16_t vectorType = sysbvm_tuple_int16_decode(vectorOperand->vectorType);
    if(vectorType != SYSBVM_OPERAND_VECTOR_LOCAL || index < 0 || (size_t)index >= sysbvm_orderedCollection_getSize(assembler->temporaries))
        return;

    sysbvm_tuple_t temporaryType = sysbvm_orderedCollection_at(assembler->temporaryTypes, index);
    if(!sysbvm_tuple_isKindOf(context, temporaryType, context->roots.pointerLikeType))
        sysbvm_error("Expected a pointer like type for alloca which is going to be lowered.");

    sysbvm_tuple_t baseType = ((sysbvm_pointerLikeType_t*)temporaryType)->baseType;
    sysbvm_orderedCollection_atPut(assembler->temporaryTypes, index, baseType);
}

static void sysbvm_functionBytecodeAssembler_optimizeLocalOnlyAllocaAndSlotReferences(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_functionBytecodeAssemblerAbstractInstruction_t *instruction)
{
    // Ignore labels.
    if(sysbvm_functionBytecodeAssemblerInstruction_isLabel(context, instruction))
        return;

    sysbvm_functionBytecodeAssemblerInstruction_t *bytecodeInstruction = (sysbvm_functionBytecodeAssemblerInstruction_t*)instruction;
    uint8_t opcode = sysbvm_tuple_uint8_decode(bytecodeInstruction->standardOpcode);
    if(opcode == SYSBVM_OPCODE_ALLOCA
        && sysbvm_functionBytecodeAssembler_isLocalOnlyAlloca(sysbvm_array_at(bytecodeInstruction->operands, 0)))
    {
        bytecodeInstruction->standardOpcode = sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_MOVE);
        sysbvm_array_atPut(bytecodeInstruction->operands, 1, sysbvm_functionBytecodeAssembler_addLiteral(context, assembler, SYSBVM_NULL_TUPLE));
        sysbvm_functionBytecodeAssembler_lowerTemporaryPointerRank(context, assembler, sysbvm_array_at(bytecodeInstruction->operands, 0));
    }
    else if(opcode == SYSBVM_OPCODE_ALLOCA_WITH_VALUE
        && sysbvm_functionBytecodeAssembler_isLocalOnlyAlloca(sysbvm_array_at(bytecodeInstruction->operands, 0)))
    {
        sysbvm_functionBytecodeAssembler_lowerTemporaryPointerRank(context, assembler, sysbvm_array_at(bytecodeInstruction->operands, 0));

        sysbvm_tuple_t newOperands = sysbvm_array_create(context, 2);
        sysbvm_array_atPut(newOperands, 0, sysbvm_array_at(bytecodeInstruction->operands, 0));
        sysbvm_array_atPut(newOperands, 1, sysbvm_array_at(bytecodeInstruction->operands, 2));
        bytecodeInstruction->standardOpcode = sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_MOVE);
        bytecodeInstruction->operands = newOperands;
    }
    else if(opcode == SYSBVM_OPCODE_SLOT_REFERENCE_AT
        && sysbvm_functionBytecodeAssembler_isLocalOnlyReferenceAt(sysbvm_array_at(bytecodeInstruction->operands, 0)))
    {
        sysbvm_functionBytecodeAssemblerVectorOperand_t *referenceOperand = (sysbvm_functionBytecodeAssemblerVectorOperand_t*)sysbvm_array_at(bytecodeInstruction->operands, 0);
        referenceOperand->optimizationTupleOperand = sysbvm_array_at(bytecodeInstruction->operands, 1);
        referenceOperand->optimizationTypeSlotOperand = sysbvm_array_at(bytecodeInstruction->operands, 2);

        sysbvm_functionBytecodeAssembler_removeInstruction(assembler, instruction);
    }
    else if(opcode == SYSBVM_OPCODE_LOAD
        && sysbvm_functionBytecodeAssembler_isLocalOnlyAlloca(sysbvm_array_at(bytecodeInstruction->operands, 1)))
    {
        bytecodeInstruction->standardOpcode = sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_MOVE);
    }
    else if(opcode == SYSBVM_OPCODE_LOAD
        && sysbvm_functionBytecodeAssembler_isLocalOnlyReferenceAt(sysbvm_array_at(bytecodeInstruction->operands, 1)))
    {
        sysbvm_functionBytecodeAssemblerVectorOperand_t *referenceOperand = (sysbvm_functionBytecodeAssemblerVectorOperand_t*)sysbvm_array_at(bytecodeInstruction->operands, 1);

        sysbvm_tuple_t newOperands = sysbvm_array_create(context, 3);
        sysbvm_array_atPut(newOperands, 0, sysbvm_array_at(bytecodeInstruction->operands, 0));
        sysbvm_array_atPut(newOperands, 1, referenceOperand->optimizationTupleOperand);
        sysbvm_array_atPut(newOperands, 2, referenceOperand->optimizationTypeSlotOperand);
        bytecodeInstruction->standardOpcode = sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_SLOT_AT);
        bytecodeInstruction->operands = newOperands;
    }
    else if(opcode == SYSBVM_OPCODE_STORE
        && sysbvm_functionBytecodeAssembler_isLocalOnlyAlloca(sysbvm_array_at(bytecodeInstruction->operands, 0)))
    {
        bytecodeInstruction->standardOpcode = sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_MOVE);
    }
    else if(opcode == SYSBVM_OPCODE_STORE
        && sysbvm_functionBytecodeAssembler_isLocalOnlyReferenceAt(sysbvm_array_at(bytecodeInstruction->operands, 0)))
    {
        sysbvm_functionBytecodeAssemblerVectorOperand_t *referenceOperand = (sysbvm_functionBytecodeAssemblerVectorOperand_t*)sysbvm_array_at(bytecodeInstruction->operands, 0);

        sysbvm_tuple_t newOperands = sysbvm_array_create(context, 3);
        sysbvm_array_atPut(newOperands, 0, referenceOperand->optimizationTupleOperand);
        sysbvm_array_atPut(newOperands, 1, referenceOperand->optimizationTypeSlotOperand);
        sysbvm_array_atPut(newOperands, 2, sysbvm_array_at(bytecodeInstruction->operands, 1));
        bytecodeInstruction->standardOpcode = sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_SLOT_AT_PUT);
        bytecodeInstruction->operands = newOperands;
    }
}

static void sysbvm_functionBytecodeAssembler_optimizeTemporaries(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler)
{
    // Clear the temporaries usage flags.
    size_t temporaryCount = sysbvm_orderedCollection_getSize(assembler->temporaries);
    for(size_t i = 0; i < temporaryCount; ++i)
    {
        sysbvm_functionBytecodeAssemblerVectorOperand_t *temporary = (sysbvm_functionBytecodeAssemblerVectorOperand_t*)sysbvm_orderedCollection_at(assembler->temporaries, i);
        temporary->hasAllocaDestination = SYSBVM_FALSE_TUPLE;
        temporary->hasNonAllocaDestination = SYSBVM_FALSE_TUPLE;
        temporary->hasLoadStoreUsage = SYSBVM_FALSE_TUPLE;
        temporary->hasNonLoadStoreUsage = SYSBVM_FALSE_TUPLE;
    }

    // Mark the temporaries per instruction usage.
    sysbvm_functionBytecodeAssemblerAbstractInstruction_t *instruction = assembler->firstInstruction;
    while(instruction)
    {
        sysbvm_functionBytecodeAssembler_markInstructionOperandUsages(context, instruction);
        instruction = instruction->next;
    }

    // Optimize the local only allocas.
    instruction = assembler->firstInstruction;
    while(instruction)
    {
        sysbvm_functionBytecodeAssemblerAbstractInstruction_t *nextInstruction = instruction->next;
        sysbvm_functionBytecodeAssembler_optimizeLocalOnlyAllocaAndSlotReferences(context, assembler, instruction);
        instruction = nextInstruction;
    }
}

SYSBVM_API void sysbvm_functionBytecodeDirectCompiler_compileFunctionDefinition(sysbvm_context_t *context, sysbvm_functionDefinition_t *definition_)
{
    struct {
        sysbvm_functionDefinition_t *definition;
        sysbvm_functionSourceAnalyzedDefinition_t *sourceAnalyzedDefinition;
        sysbvm_functionBytecodeDirectCompiler_t *compiler;
        sysbvm_tuple_t bodyResult;
        sysbvm_functionBytecode_t *bytecode;
        sysbvm_functionBytecodeAssemblerAbstractInstruction_t *instruction;

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

    gcFrame.sourceAnalyzedDefinition = (sysbvm_functionSourceAnalyzedDefinition_t*)gcFrame.definition->sourceAnalyzedDefinition;

    gcFrame.compiler = (sysbvm_functionBytecodeDirectCompiler_t*)sysbvm_functionBytecodeDirectCompiler_create(context);
    gcFrame.compiler->assembler->sourceEnvironment = gcFrame.sourceAnalyzedDefinition->environment;
    gcFrame.compiler->assembler->sourcePosition = gcFrame.sourceAnalyzedDefinition->sourcePosition;

    {
        // Make the argument operands.
        size_t argumentCount = sysbvm_array_getSize(gcFrame.sourceAnalyzedDefinition->arguments);
        sysbvm_functionBytecodeAssembler_setArgumentCount(context, gcFrame.compiler->assembler, argumentCount);
        for(size_t i = 0; i < argumentCount; ++i)
        {
            sysbvm_tuple_t binding = sysbvm_array_at(gcFrame.sourceAnalyzedDefinition->arguments, i);
            sysbvm_tuple_t value = sysbvm_array_at(gcFrame.compiler->assembler->arguments, i);
            sysbvm_functionBytecodeDirectCompiler_setBindingValue(context, gcFrame.compiler, binding, value);
        }

        // Make the binding operands.
        size_t captureCount = sysbvm_array_getSize(gcFrame.sourceAnalyzedDefinition->captures);
        sysbvm_functionBytecodeAssembler_setCaptureCount(context, gcFrame.compiler->assembler, captureCount);
        for(size_t i = 0; i < captureCount; ++i)
        {
            sysbvm_tuple_t binding = sysbvm_array_at(gcFrame.sourceAnalyzedDefinition->captures, i);
            sysbvm_tuple_t value = sysbvm_array_at(gcFrame.compiler->assembler->captures, i);
            sysbvm_functionBytecodeDirectCompiler_setBindingValue(context, gcFrame.compiler, binding, value);
        }
    }

    // Compile the body.
    gcFrame.bodyResult = gcFrame.sourceAnalyzedDefinition->bodyNode
        ? sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, gcFrame.compiler, gcFrame.sourceAnalyzedDefinition->bodyNode)
        : sysbvm_functionBytecodeAssembler_addLiteral(context, gcFrame.compiler->assembler, SYSBVM_VOID_TUPLE);

    if(gcFrame.bodyResult)
        sysbvm_functionBytecodeAssembler_return(context, gcFrame.compiler->assembler, gcFrame.bodyResult);

    // Perform some optimizations.
    sysbvm_functionBytecodeAssembler_optimizeJumps(context, gcFrame.compiler->assembler);
    sysbvm_functionBytecodeAssembler_optimizeTemporaries(context, gcFrame.compiler->assembler);
    
    // Assemble the instructions.
    gcFrame.compiler->assembler->usedTemporaryCount = sysbvm_tuple_size_encode(context, sysbvm_orderedCollection_getSize(gcFrame.compiler->assembler->temporaries));

    gcFrame.bytecode = (sysbvm_functionBytecode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.functionBytecodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_functionBytecode_t));
    gcFrame.bytecode->argumentCount = sysbvm_tuple_size_encode(context, sysbvm_array_getSize(gcFrame.compiler->assembler->arguments));
    gcFrame.bytecode->captureVectorSize = sysbvm_tuple_size_encode(context, sysbvm_array_getSize(gcFrame.compiler->assembler->captures));
    gcFrame.bytecode->localVectorSize = gcFrame.compiler->assembler->usedTemporaryCount;
    gcFrame.bytecode->literalVector = sysbvm_orderedCollection_asArray(context, gcFrame.compiler->assembler->literals);

    gcFrame.bytecode->arguments = gcFrame.sourceAnalyzedDefinition->arguments;
    gcFrame.bytecode->captures = gcFrame.sourceAnalyzedDefinition->captures;
    gcFrame.bytecode->temporaryTypes = sysbvm_orderedCollection_asArray(context, gcFrame.compiler->assembler->temporaryTypes);

    gcFrame.bytecode->jittedCode = sysbvm_tuple_systemHandle_encode(context, 0);
    gcFrame.bytecode->jittedCodeSessionToken = sysbvm_tuple_systemHandle_encode(context, 0);
    gcFrame.bytecode->jittedCodeTrampoline = sysbvm_tuple_systemHandle_encode(context, 0);
    gcFrame.bytecode->jittedCodeTrampolineSessionToken = sysbvm_tuple_systemHandle_encode(context, 0);

    // Tables for the debug information.
    gcFrame.pcToDebugListTable = sysbvm_orderedCollection_create(context);
    gcFrame.debugSourceASTNodes = sysbvm_orderedCollection_create(context);
    gcFrame.debugSourcePositions = sysbvm_orderedCollection_create(context);
    gcFrame.debugSourceEnvironments = sysbvm_orderedCollection_create(context);
    gcFrame.debugListsDictionary = sysbvm_dictionary_create(context);

    // Assemble the instructions.
    size_t instructionsOffset = 0;
    for(gcFrame.instruction = gcFrame.compiler->assembler->firstInstruction; gcFrame.instruction; gcFrame.instruction = gcFrame.instruction->next)
    {
        size_t pc = instructionsOffset;
        gcFrame.instruction->pc = sysbvm_tuple_size_encode(context, instructionsOffset);
        instructionsOffset += sysbvm_functionBytecodeAssemblerInstruction_computeAssembledSize(context, gcFrame.instruction);
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
    for(gcFrame.instruction = gcFrame.compiler->assembler->firstInstruction; gcFrame.instruction; gcFrame.instruction = gcFrame.instruction->next)
        instructionsOffset += sysbvm_functionBytecodeAssemblerInstruction_assembleInto(context, gcFrame.instruction, destInstructions + instructionsOffset);

    // Finish by installing it on the definition.
    gcFrame.definition->bytecode = (sysbvm_tuple_t)gcFrame.bytecode;

    gcFrame.bytecode->pcToDebugListTable = sysbvm_orderedCollection_asArray(context, gcFrame.pcToDebugListTable);
    gcFrame.bytecode->debugSourceASTNodes = sysbvm_orderedCollection_asArray(context, gcFrame.debugSourceASTNodes);
    gcFrame.bytecode->debugSourcePositions = sysbvm_orderedCollection_asArray(context, gcFrame.debugSourcePositions);
    gcFrame.bytecode->debugSourceEnvironments = sysbvm_orderedCollection_asArray(context, gcFrame.debugSourceEnvironments);
}

static sysbvm_tuple_t sysbvm_astNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);
    
    sysbvm_error("ASTNode compileIntoBytecode: subclass responsibility");
    return SYSBVM_NULL_TUPLE;
}

static sysbvm_tuple_t sysbvm_astBreakNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    //sysbvm_tuple_t *node = &arguments[0];
    sysbvm_functionBytecodeDirectCompiler_t **compiler = (sysbvm_functionBytecodeDirectCompiler_t **)&arguments[1];
    if(!(*compiler)->breakLabel)
        sysbvm_error("Break statement in wrong location.");
    sysbvm_functionBytecodeAssembler_jump(context, (*compiler)->assembler, (*compiler)->breakLabel);
     
    return sysbvm_functionBytecodeAssembler_addLiteral(context, (*compiler)->assembler, SYSBVM_VOID_TUPLE);
}

static sysbvm_tuple_t sysbvm_astCoerceValueNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_functionBytecodeDirectCompiler_t **compiler = (sysbvm_functionBytecodeDirectCompiler_t **)&arguments[1];

    sysbvm_astCoerceValueNode_t **coerceValueNode = (sysbvm_astCoerceValueNode_t**)node;
    struct {
        sysbvm_tuple_t typeOperand;
        sysbvm_tuple_t valueOperand;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.typeOperand = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, (*coerceValueNode)->typeExpression);
    gcFrame.valueOperand = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, (*coerceValueNode)->valueExpression);
    gcFrame.result = sysbvm_functionBytecodeAssembler_newTemporary(context, (*compiler)->assembler, (*coerceValueNode)->super.analyzedType);
    sysbvm_functionBytecodeAssembler_coerceValue(context, (*compiler)->assembler, gcFrame.result, gcFrame.typeOperand, gcFrame.valueOperand);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astContinueNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    //sysbvm_tuple_t *node = &arguments[0];
    sysbvm_functionBytecodeDirectCompiler_t **compiler = (sysbvm_functionBytecodeDirectCompiler_t **)&arguments[1];
    if(!(*compiler)->continueLabel)
        sysbvm_error("Continue statement in wrong location.");
    sysbvm_functionBytecodeAssembler_jump(context, (*compiler)->assembler, (*compiler)->continueLabel);
     
    return sysbvm_functionBytecodeAssembler_addLiteral(context, (*compiler)->assembler, SYSBVM_VOID_TUPLE);
}

static sysbvm_tuple_t sysbvm_astDoWhileContinueWithNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_functionBytecodeDirectCompiler_t **compiler = (sysbvm_functionBytecodeDirectCompiler_t **)&arguments[1];

    sysbvm_astDoWhileContinueWithNode_t **doWhileNode = (sysbvm_astDoWhileContinueWithNode_t**)node;
    struct {
        sysbvm_tuple_t doWhileEntryLabel;
        sysbvm_tuple_t doWhileCondition;
        sysbvm_tuple_t doWhileContinue;
        sysbvm_tuple_t doWhileMergeLabel;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.doWhileEntryLabel = sysbvm_functionBytecodeAssemblerInstruction_createLabel(context);
    gcFrame.doWhileCondition = sysbvm_functionBytecodeAssemblerInstruction_createLabel(context);
    gcFrame.doWhileContinue = sysbvm_functionBytecodeAssemblerInstruction_createLabel(context);
    gcFrame.doWhileMergeLabel = sysbvm_functionBytecodeAssemblerInstruction_createLabel(context);

    // Do while body.
    sysbvm_functionBytecodeAssembler_addInstruction((*compiler)->assembler, gcFrame.doWhileEntryLabel);
    if((*doWhileNode)->bodyExpression)
        sysbvm_functionBytecodeDirectCompiler_compileASTNodeWithBreakAndContinue(context, *compiler, (*doWhileNode)->bodyExpression, gcFrame.doWhileMergeLabel, gcFrame.doWhileCondition);
    sysbvm_functionBytecodeAssembler_jump(context, (*compiler)->assembler, gcFrame.doWhileCondition);

    // Do while condition block.
    sysbvm_functionBytecodeAssembler_addInstruction((*compiler)->assembler, gcFrame.doWhileCondition);
    if((*doWhileNode)->conditionExpression)
    {
        sysbvm_tuple_t condition = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, (*doWhileNode)->conditionExpression);
        sysbvm_functionBytecodeAssembler_jumpIfFalse(context, (*compiler)->assembler, condition, gcFrame.doWhileMergeLabel);
    }
    else
    {
        sysbvm_functionBytecodeAssembler_jump(context, (*compiler)->assembler, gcFrame.doWhileContinue);
    }

    // Do while continue
    sysbvm_functionBytecodeAssembler_addInstruction((*compiler)->assembler, gcFrame.doWhileContinue);
    if((*doWhileNode)->continueExpression)
        sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, (*doWhileNode)->continueExpression);

    sysbvm_functionBytecodeAssembler_jump(context, (*compiler)->assembler, gcFrame.doWhileEntryLabel);

    // While merge
    sysbvm_functionBytecodeAssembler_addInstruction((*compiler)->assembler, gcFrame.doWhileMergeLabel);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return sysbvm_functionBytecodeAssembler_addLiteral(context, (*compiler)->assembler, SYSBVM_VOID_TUPLE);
}

static sysbvm_tuple_t sysbvm_astDownCastNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_functionBytecodeDirectCompiler_t **compiler = (sysbvm_functionBytecodeDirectCompiler_t **)&arguments[1];

    sysbvm_astDownCastNode_t **downCastNode = (sysbvm_astDownCastNode_t**)node;
    struct {
        sysbvm_tuple_t typeOperand;
        sysbvm_tuple_t valueOperand;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.typeOperand = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, (*downCastNode)->typeExpression);
    gcFrame.valueOperand = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, (*downCastNode)->valueExpression);
    gcFrame.result = sysbvm_functionBytecodeAssembler_newTemporary(context, (*compiler)->assembler, (*downCastNode)->super.analyzedType);
    if(sysbvm_tuple_boolean_decode((*downCastNode)->isUnchecked))
        sysbvm_functionBytecodeAssembler_uncheckedDownCastValue(context, (*compiler)->assembler, gcFrame.result, gcFrame.typeOperand, gcFrame.valueOperand);
    else
        sysbvm_functionBytecodeAssembler_downCastValue(context, (*compiler)->assembler, gcFrame.result, gcFrame.typeOperand, gcFrame.valueOperand);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_functionBytecodeAssembler_getLiteralFunctionPrimitiveName(sysbvm_context_t *context, sysbvm_functionBytecodeAssembler_t *assembler, sysbvm_tuple_t operand)
{
    //sysbvm_tuple_t operandType 
    if(sysbvm_tuple_getType(context, operand) != context->roots.functionBytecodeAssemblerVectorOperand)
        return SYSBVM_NULL_TUPLE;
    
    sysbvm_functionBytecodeAssemblerVectorOperand_t *vectorOperand = (sysbvm_functionBytecodeAssemblerVectorOperand_t*)operand;
    int16_t index = sysbvm_tuple_int16_decode(vectorOperand->index);
    int16_t vectorType = sysbvm_tuple_int16_decode(vectorOperand->vectorType);
    if(vectorType != SYSBVM_OPERAND_VECTOR_LITERAL || index < 0 || (size_t)index >= sysbvm_orderedCollection_getSize(assembler->literals))
        return SYSBVM_NULL_TUPLE;

    sysbvm_tuple_t literal = sysbvm_orderedCollection_at(assembler->literals, index);
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
    sysbvm_functionBytecodeDirectCompiler_t **compiler = (sysbvm_functionBytecodeDirectCompiler_t **)&arguments[1];

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

    gcFrame.function = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, (*applicationNode)->functionExpression);

    // Inline some special functions here.
    gcFrame.primitiveName = sysbvm_functionBytecodeAssembler_getLiteralFunctionPrimitiveName(context, (*compiler)->assembler, gcFrame.function);
    if(gcFrame.primitiveName == context->roots.anyValueToVoidPrimitiveName)
    {
        for(size_t i = 0; i < applicationArgumentCount; ++i)
        {
            gcFrame.argumentNode = sysbvm_array_at((*applicationNode)->arguments, i);
            sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, gcFrame.argumentNode);
        }

        gcFrame.result = sysbvm_functionBytecodeAssembler_addLiteral(context, (*compiler)->assembler, SYSBVM_VOID_TUPLE);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return gcFrame.result;
    }
    else if(gcFrame.primitiveName == context->roots.pointerLikeLoadPrimitiveName && applicationArgumentCount == 1)
    {
        gcFrame.argumentNode = sysbvm_array_at((*applicationNode)->arguments, 0);
        gcFrame.pointerOperand = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, gcFrame.argumentNode);

        gcFrame.result = sysbvm_functionBytecodeAssembler_newTemporary(context, (*compiler)->assembler, (*applicationNode)->super.analyzedType);
        sysbvm_functionBytecodeAssembler_load(context, (*compiler)->assembler, gcFrame.result, gcFrame.pointerOperand);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return gcFrame.result;
    }
    else if(gcFrame.primitiveName == context->roots.pointerLikeStorePrimitiveName && applicationArgumentCount == 2)
    {
        gcFrame.argumentNode = sysbvm_array_at((*applicationNode)->arguments, 0);
        gcFrame.pointerOperand = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, gcFrame.argumentNode);

        gcFrame.argumentNode = sysbvm_array_at((*applicationNode)->arguments, 1);
        gcFrame.valueOperand = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, gcFrame.argumentNode);

        sysbvm_functionBytecodeAssembler_store(context, (*compiler)->assembler, gcFrame.pointerOperand, gcFrame.valueOperand);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return gcFrame.pointerOperand;
    }

    gcFrame.arguments = sysbvm_array_create(context, applicationArgumentCount);

    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        gcFrame.argumentNode = sysbvm_array_at((*applicationNode)->arguments, i);
        gcFrame.argumentOperand = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, gcFrame.argumentNode);
        sysbvm_array_atPut(gcFrame.arguments, i, gcFrame.argumentOperand);
    }

    gcFrame.result = sysbvm_functionBytecodeAssembler_newTemporary(context, (*compiler)->assembler, (*applicationNode)->super.analyzedType);

    sysbvm_bitflags_t applicationFlags = sysbvm_tuple_bitflags_decode((*applicationNode)->applicationFlags);
    bool isNotypecheck = (applicationFlags & SYSBVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK) != 0;

    if(isNotypecheck)
        sysbvm_functionBytecodeAssembler_uncheckedCall(context, (*compiler)->assembler, gcFrame.result, gcFrame.function, gcFrame.arguments);
    else
        sysbvm_functionBytecodeAssembler_call(context, (*compiler)->assembler, gcFrame.result, gcFrame.function, gcFrame.arguments);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astIdentifierReferenceNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_functionBytecodeDirectCompiler_t **compiler = (sysbvm_functionBytecodeDirectCompiler_t **)&arguments[1];

    sysbvm_astIdentifierReferenceNode_t **referenceNode = (sysbvm_astIdentifierReferenceNode_t**)node;
    return sysbvm_functionBytecodeDirectCompiler_getBindingValue(context, *compiler, (*referenceNode)->binding);
}

static sysbvm_tuple_t sysbvm_astIfNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_functionBytecodeDirectCompiler_t **compiler = (sysbvm_functionBytecodeDirectCompiler_t **)&arguments[1];

    sysbvm_astIfNode_t **ifNode = (sysbvm_astIfNode_t**)node;
    struct {
        sysbvm_tuple_t falseLabel;
        sysbvm_tuple_t mergeLabel;
        sysbvm_tuple_t trueResult;
        sysbvm_tuple_t falseResult;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.falseLabel = sysbvm_functionBytecodeAssemblerInstruction_createLabel(context);
    gcFrame.mergeLabel = sysbvm_functionBytecodeAssemblerInstruction_createLabel(context);

    // Emit the condition
    {
        sysbvm_tuple_t condition = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, (*ifNode)->conditionExpression);
        sysbvm_functionBytecodeAssembler_jumpIfFalse(context, (*compiler)->assembler, condition, gcFrame.falseLabel);
    }

    gcFrame.result = sysbvm_functionBytecodeAssembler_newTemporary(context, (*compiler)->assembler, (*ifNode)->super.analyzedType);

    // True branch.
    if((*ifNode)->trueExpression)
        gcFrame.trueResult = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, (*ifNode)->trueExpression);
    else
        gcFrame.trueResult = sysbvm_functionBytecodeAssembler_addLiteral(context, (*compiler)->assembler, SYSBVM_NULL_TUPLE);
    sysbvm_functionBytecodeAssembler_move(context, (*compiler)->assembler, gcFrame.result, gcFrame.trueResult);
    sysbvm_functionBytecodeAssembler_jump(context, (*compiler)->assembler, gcFrame.mergeLabel);

    // False branch.
    sysbvm_functionBytecodeAssembler_addInstruction((*compiler)->assembler, gcFrame.falseLabel);
    if((*ifNode)->falseExpression)
        gcFrame.falseResult = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, (*ifNode)->falseExpression);
    else
        gcFrame.falseResult = sysbvm_functionBytecodeAssembler_addLiteral(context, (*compiler)->assembler, SYSBVM_NULL_TUPLE);
    sysbvm_functionBytecodeAssembler_move(context, (*compiler)->assembler, gcFrame.result, gcFrame.falseResult);

    sysbvm_functionBytecodeAssembler_addInstruction((*compiler)->assembler, gcFrame.mergeLabel);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astLambdaNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_functionBytecodeDirectCompiler_t **compiler = (sysbvm_functionBytecodeDirectCompiler_t **)&arguments[1];

    sysbvm_astLambdaNode_t **lambdaNode = (sysbvm_astLambdaNode_t**)node;

    sysbvm_functionDefinition_t *functionDefinition = (sysbvm_functionDefinition_t*)(*lambdaNode)->functionDefinition;
    sysbvm_tuple_t functionDefinitionOperand = sysbvm_functionBytecodeAssembler_addLiteral(context, (*compiler)->assembler, (sysbvm_tuple_t)functionDefinition);
    sysbvm_functionSourceAnalyzedDefinition_t *sourceAnalyzedDefinition = (sysbvm_functionSourceAnalyzedDefinition_t*)functionDefinition->sourceAnalyzedDefinition;

    size_t captureVectorSize = sysbvm_array_getSize(sourceAnalyzedDefinition->captures);
    sysbvm_tuple_t captureVector = sysbvm_array_create(context, captureVectorSize);
    
    for(size_t i = 0; i < captureVectorSize; ++i)
    {
        sysbvm_tuple_t captureBinding = sysbvm_symbolCaptureBinding_getSourceBinding(sysbvm_array_at(sourceAnalyzedDefinition->captures, i));
        sysbvm_tuple_t captureValue = sysbvm_functionBytecodeDirectCompiler_getBindingValue(context, *compiler, captureBinding);

        sysbvm_array_atPut(captureVector, i, captureValue);
    }

    sysbvm_tuple_t result = sysbvm_functionBytecodeAssembler_newTemporary(context, (*compiler)->assembler, (*lambdaNode)->super.analyzedType);
    sysbvm_functionBytecodeAssembler_makeClosureWithCaptures(context, (*compiler)->assembler, result, functionDefinitionOperand, captureVector);

    if((*lambdaNode)->binding)
        sysbvm_functionBytecodeDirectCompiler_setBindingValue(context, *compiler, (*lambdaNode)->binding, result);
    return result;
}

static sysbvm_tuple_t sysbvm_astLexicalBlockNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_functionBytecodeDirectCompiler_t **compiler = (sysbvm_functionBytecodeDirectCompiler_t **)&arguments[1];

    sysbvm_astLexicalBlockNode_t **lexicalBlockNode = (sysbvm_astLexicalBlockNode_t**)node;

    return sysbvm_functionBytecodeDirectCompiler_compileASTNodeWithEnvironment(context, *compiler, (*lexicalBlockNode)->body, (*lexicalBlockNode)->bodyEnvironment);
}

static sysbvm_tuple_t sysbvm_astLiteralNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_functionBytecodeDirectCompiler_t **compiler = (sysbvm_functionBytecodeDirectCompiler_t **)&arguments[1];

    sysbvm_astLiteralNode_t **literalNode = (sysbvm_astLiteralNode_t**)node;
    return sysbvm_functionBytecodeAssembler_addLiteral(context, (*compiler)->assembler, (*literalNode)->value);
}

static bool sysbvm_symbolBinding_hasValidNameForDebugging(sysbvm_context_t *context, sysbvm_tuple_t binding)
{
    sysbvm_tuple_t name = sysbvm_symbolBinding_getName(binding);
    return name && !sysbvm_tuple_isKindOf(context, name, context->roots.generatedSymbolType);
}

static sysbvm_tuple_t sysbvm_astVariableDefinitionNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_functionBytecodeDirectCompiler_t **compiler = (sysbvm_functionBytecodeDirectCompiler_t **)&arguments[1];

    sysbvm_astVariableDefinitionNode_t **variableDefinitionNode = (sysbvm_astVariableDefinitionNode_t**)node;

    sysbvm_tuple_t value = (*variableDefinitionNode)->valueExpression
        ? sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, (*variableDefinitionNode)->valueExpression)
        : sysbvm_functionBytecodeAssembler_addLiteral(context, (*compiler)->assembler, SYSBVM_NULL_TUPLE);

    bool isMutable = sysbvm_tuple_boolean_decode((*variableDefinitionNode)->isMutable);
    if(isMutable)
    {
        sysbvm_tuple_t localVariable = sysbvm_functionBytecodeAssembler_newTemporary(context, (*compiler)->assembler, (*variableDefinitionNode)->super.analyzedType);
        sysbvm_functionBytecodeAssembler_allocaWithValue(context, (*compiler)->assembler, localVariable,
            sysbvm_functionBytecodeAssembler_addLiteral(context, (*compiler)->assembler, (*variableDefinitionNode)->super.analyzedType),
            value);

        if(sysbvm_symbolBinding_hasValidNameForDebugging(context, (*variableDefinitionNode)->binding))
            sysbvm_functionBytecodeAssembler_setDebugValue(context, (*compiler)->assembler, localVariable,
                sysbvm_functionBytecodeAssembler_addLiteral(context, (*compiler)->assembler, (*variableDefinitionNode)->binding));

        sysbvm_functionBytecodeDirectCompiler_setBindingValue(context, *compiler, (*variableDefinitionNode)->binding, localVariable);
        return localVariable;
    }
    else
    {
        if(sysbvm_symbolBinding_hasValidNameForDebugging(context, (*variableDefinitionNode)->binding))
            sysbvm_functionBytecodeAssembler_setDebugValue(context, (*compiler)->assembler, value,
                sysbvm_functionBytecodeAssembler_addLiteral(context, (*compiler)->assembler, (*variableDefinitionNode)->binding));

        sysbvm_functionBytecodeDirectCompiler_setBindingValue(context, *compiler, (*variableDefinitionNode)->binding, value);
        return value;
    }
}

static sysbvm_tuple_t sysbvm_astMakeAssociationNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_functionBytecodeDirectCompiler_t **compiler = (sysbvm_functionBytecodeDirectCompiler_t **)&arguments[1];

    sysbvm_astMakeAssociationNode_t **associationNode = (sysbvm_astMakeAssociationNode_t**)node;
    struct {
        sysbvm_tuple_t key;
        sysbvm_tuple_t value;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.key = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, (*associationNode)->key);
    if((*associationNode)->value)
        gcFrame.value = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, (*associationNode)->value);
    else
        gcFrame.value = sysbvm_functionBytecodeAssembler_addLiteral(context, (*compiler)->assembler, SYSBVM_NULL_TUPLE);

    gcFrame.result = sysbvm_functionBytecodeAssembler_newTemporary(context, (*compiler)->assembler, (*associationNode)->super.analyzedType);
    sysbvm_functionBytecodeAssembler_makeAssociation(context, (*compiler)->assembler, gcFrame.result, gcFrame.key, gcFrame.value);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astMakeArrayNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_functionBytecodeDirectCompiler_t **compiler = (sysbvm_functionBytecodeDirectCompiler_t **)&arguments[1];

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
        gcFrame.element = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, sysbvm_array_at((*arrayNode)->elements, i));
        sysbvm_array_atPut(gcFrame.elements, i, gcFrame.element);
    }

    gcFrame.result = sysbvm_functionBytecodeAssembler_newTemporary(context, (*compiler)->assembler, (*arrayNode)->super.analyzedType);
    sysbvm_functionBytecodeAssembler_makeArrayWithElements(context, (*compiler)->assembler, gcFrame.result, gcFrame.elements);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astMakeByteArrayNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_functionBytecodeDirectCompiler_t **compiler = (sysbvm_functionBytecodeDirectCompiler_t **)&arguments[1];

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
        gcFrame.element = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, sysbvm_array_at((*arrayNode)->elements, i));
        sysbvm_array_atPut(gcFrame.elements, i, gcFrame.element);
    }

    gcFrame.result = sysbvm_functionBytecodeAssembler_newTemporary(context, (*compiler)->assembler, (*arrayNode)->super.analyzedType);
    sysbvm_functionBytecodeAssembler_makeByteArrayWithElements(context, (*compiler)->assembler, gcFrame.result, gcFrame.elements);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astMakeDictionaryNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_functionBytecodeDirectCompiler_t **compiler = (sysbvm_functionBytecodeDirectCompiler_t **)&arguments[1];

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
        gcFrame.element = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, sysbvm_array_at((*dictionaryNode)->elements, i));
        sysbvm_array_atPut(gcFrame.elements, i, gcFrame.element);
    }

    gcFrame.result = sysbvm_functionBytecodeAssembler_newTemporary(context, (*compiler)->assembler, (*dictionaryNode)->super.analyzedType);
    sysbvm_functionBytecodeAssembler_makeDictionaryWithElements(context, (*compiler)->assembler, gcFrame.result, gcFrame.elements);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astMessageSendNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_functionBytecodeDirectCompiler_t **compiler = (sysbvm_functionBytecodeDirectCompiler_t **)&arguments[1];

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

    gcFrame.receiver = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, (*sendNode)->receiver);
    if((*sendNode)->receiverLookupType)
        gcFrame.receiverLookupType = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, (*sendNode)->receiverLookupType);
    gcFrame.selector = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, (*sendNode)->selector);

    size_t sendArgumentCount = sysbvm_array_getSize((*sendNode)->arguments);
    gcFrame.arguments = sysbvm_array_create(context, sendArgumentCount);
    for(size_t i = 0; i < sendArgumentCount; ++i)
    {
        gcFrame.argument = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, sysbvm_array_at((*sendNode)->arguments, i));
        sysbvm_array_atPut(gcFrame.arguments, i, gcFrame.argument);
    }

    gcFrame.result = sysbvm_functionBytecodeAssembler_newTemporary(context, (*compiler)->assembler, (*sendNode)->super.analyzedType);
    if((*sendNode)->receiverLookupType)
        sysbvm_functionBytecodeAssembler_sendWithLookupReceiverType(context, (*compiler)->assembler, gcFrame.result, gcFrame.receiverLookupType, gcFrame.selector, gcFrame.receiver, gcFrame.arguments);
    else
        sysbvm_functionBytecodeAssembler_send(context, (*compiler)->assembler, gcFrame.result, gcFrame.selector, gcFrame.receiver, gcFrame.arguments);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astReturnNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_functionBytecodeDirectCompiler_t **compiler = (sysbvm_functionBytecodeDirectCompiler_t **)&arguments[1];

    sysbvm_astReturnNode_t **returnNode = (sysbvm_astReturnNode_t**)node;
    sysbvm_tuple_t result = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, (*returnNode)->expression);
    sysbvm_functionBytecodeAssembler_return(context, (*compiler)->assembler, result);
    return result;
}

static sysbvm_tuple_t sysbvm_astSequenceNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_functionBytecodeDirectCompiler_t **compiler = (sysbvm_functionBytecodeDirectCompiler_t **)&arguments[1];

    sysbvm_astSequenceNode_t **sequenceNode = (sysbvm_astSequenceNode_t**)node;
    struct {
        sysbvm_tuple_t expressionNode;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    size_t expressionCount = sysbvm_array_getSize((*sequenceNode)->expressions);
    gcFrame.result = SYSBVM_NULL_TUPLE;
    if(expressionCount == 0)
        gcFrame.result = sysbvm_functionBytecodeAssembler_addLiteral(context, (*compiler)->assembler, SYSBVM_VOID_TUPLE);

    for(size_t i = 0; i < expressionCount; ++i)
    {
        gcFrame.expressionNode = sysbvm_array_at((*sequenceNode)->expressions, i);
        gcFrame.result = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, gcFrame.expressionNode);
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
    sysbvm_functionBytecodeDirectCompiler_t **compiler = (sysbvm_functionBytecodeDirectCompiler_t **)&arguments[1];

    sysbvm_astTupleSlotNamedAtNode_t **slotNamedNode = (sysbvm_astTupleSlotNamedAtNode_t**)node;
    struct {
        sysbvm_tuple_t tuple;
        sysbvm_tuple_t slot;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.tuple = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, (*slotNamedNode)->tupleExpression);
    gcFrame.slot = sysbvm_functionBytecodeAssembler_addLiteral(context, (*compiler)->assembler, (*slotNamedNode)->boundSlot);
    gcFrame.result = sysbvm_functionBytecodeAssembler_newTemporary(context, (*compiler)->assembler, (*slotNamedNode)->super.analyzedType);
    if(sysbvm_type_isPointerLikeType(sysbvm_astNode_getAnalyzedType((*slotNamedNode)->tupleExpression)))
        sysbvm_functionBytecodeAssembler_refSlotAt(context, (*compiler)->assembler, gcFrame.result, gcFrame.tuple, gcFrame.slot);
    else
        sysbvm_functionBytecodeAssembler_slotAt(context, (*compiler)->assembler, gcFrame.result, gcFrame.tuple, gcFrame.slot);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astTupleSlotNamedReferenceAtNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_functionBytecodeDirectCompiler_t **compiler = (sysbvm_functionBytecodeDirectCompiler_t **)&arguments[1];

    sysbvm_astTupleSlotNamedReferenceAtNode_t **slotNamedNode = (sysbvm_astTupleSlotNamedReferenceAtNode_t**)node;
    struct {
        sysbvm_tuple_t tuple;
        sysbvm_tuple_t slot;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.tuple = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, (*slotNamedNode)->tupleExpression);
    gcFrame.slot = sysbvm_functionBytecodeAssembler_addLiteral(context, (*compiler)->assembler, (*slotNamedNode)->boundSlot);
    gcFrame.result = sysbvm_functionBytecodeAssembler_newTemporary(context, (*compiler)->assembler, (*slotNamedNode)->super.analyzedType);
    if(sysbvm_type_isPointerLikeType(sysbvm_astNode_getAnalyzedType((*slotNamedNode)->tupleExpression)))
        sysbvm_functionBytecodeAssembler_refSlotReferenceAt(context, (*compiler)->assembler, gcFrame.result, gcFrame.tuple, gcFrame.slot);
    else
        sysbvm_functionBytecodeAssembler_slotReferenceAt(context, (*compiler)->assembler, gcFrame.result, gcFrame.tuple, gcFrame.slot);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astTupleSlotNamedAtPutNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_functionBytecodeDirectCompiler_t **compiler = (sysbvm_functionBytecodeDirectCompiler_t **)&arguments[1];

    sysbvm_astTupleSlotNamedAtPutNode_t **slotNamedNode = (sysbvm_astTupleSlotNamedAtPutNode_t**)node;
    struct {
        sysbvm_tuple_t tuple;
        sysbvm_tuple_t slot;
        sysbvm_tuple_t value;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.tuple = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, (*slotNamedNode)->tupleExpression);
    gcFrame.slot = sysbvm_functionBytecodeAssembler_addLiteral(context, (*compiler)->assembler, (*slotNamedNode)->boundSlot);
    gcFrame.value = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, (*slotNamedNode)->valueExpression);
    if(sysbvm_type_isPointerLikeType(sysbvm_astNode_getAnalyzedType((*slotNamedNode)->tupleExpression)))
        sysbvm_functionBytecodeAssembler_refSlotAtPut(context, (*compiler)->assembler, gcFrame.tuple, gcFrame.slot, gcFrame.value);
    else
        sysbvm_functionBytecodeAssembler_slotAtPut(context, (*compiler)->assembler, gcFrame.tuple, gcFrame.slot, gcFrame.value);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astUseNamedSlotsOfNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_functionBytecodeDirectCompiler_t **compiler = (sysbvm_functionBytecodeDirectCompiler_t **)&arguments[1];

    sysbvm_astUseNamedSlotsOfNode_t **usedNamedSlots = (sysbvm_astUseNamedSlotsOfNode_t**)node;
    struct {
        sysbvm_tuple_t tuple;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.tuple = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, (*usedNamedSlots)->tupleExpression);
    sysbvm_functionBytecodeDirectCompiler_setBindingValue(context, *compiler, (*usedNamedSlots)->binding, gcFrame.tuple);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return sysbvm_functionBytecodeAssembler_addLiteral(context, (*compiler)->assembler, SYSBVM_VOID_TUPLE);
}

static sysbvm_tuple_t sysbvm_astWhileContinueNode_primitiveCompileIntoBytecode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_functionBytecodeDirectCompiler_t **compiler = (sysbvm_functionBytecodeDirectCompiler_t **)&arguments[1];

    sysbvm_astWhileContinueWithNode_t **whileNode = (sysbvm_astWhileContinueWithNode_t**)node;
    struct {
        sysbvm_tuple_t whileEntryLabel;
        sysbvm_tuple_t whileBodyLabel;
        sysbvm_tuple_t whileContinue;
        sysbvm_tuple_t whileMergeLabel;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.whileEntryLabel = sysbvm_functionBytecodeAssemblerInstruction_createLabel(context);
    gcFrame.whileBodyLabel = sysbvm_functionBytecodeAssemblerInstruction_createLabel(context);
    gcFrame.whileContinue = sysbvm_functionBytecodeAssemblerInstruction_createLabel(context);
    gcFrame.whileMergeLabel = sysbvm_functionBytecodeAssemblerInstruction_createLabel(context);

    // While condition block.
    sysbvm_functionBytecodeAssembler_addInstruction((*compiler)->assembler, gcFrame.whileEntryLabel);
    if((*whileNode)->conditionExpression)
    {
        sysbvm_tuple_t condition = sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, (*whileNode)->conditionExpression);
        sysbvm_functionBytecodeAssembler_jumpIfFalse(context, (*compiler)->assembler, condition, gcFrame.whileMergeLabel);
    }
    else
    {
        sysbvm_functionBytecodeAssembler_jump(context, (*compiler)->assembler, gcFrame.whileBodyLabel);
    }

    // While body.
    sysbvm_functionBytecodeAssembler_addInstruction((*compiler)->assembler, gcFrame.whileBodyLabel);
    if((*whileNode)->bodyExpression)
        sysbvm_functionBytecodeDirectCompiler_compileASTNodeWithBreakAndContinue(context, *compiler, (*whileNode)->bodyExpression, gcFrame.whileMergeLabel, gcFrame.whileContinue);
    sysbvm_functionBytecodeAssembler_jump(context, (*compiler)->assembler, gcFrame.whileContinue);

    // While continue
    sysbvm_functionBytecodeAssembler_addInstruction((*compiler)->assembler, gcFrame.whileContinue);
    if((*whileNode)->continueExpression)
        sysbvm_functionBytecodeDirectCompiler_compileASTNode(context, *compiler, (*whileNode)->continueExpression);

    sysbvm_functionBytecodeAssembler_jump(context, (*compiler)->assembler, gcFrame.whileEntryLabel);

    // While merge
    sysbvm_functionBytecodeAssembler_addInstruction((*compiler)->assembler, gcFrame.whileMergeLabel);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return sysbvm_functionBytecodeAssembler_addLiteral(context, (*compiler)->assembler, SYSBVM_VOID_TUPLE);
}

static void sysbvm_functionBytecodeDirectCompiler_setupNodeCompilationFunction(sysbvm_context_t *context, sysbvm_tuple_t astNodeType, sysbvm_functionEntryPoint_t compilationFunction)
{
    sysbvm_type_setMethodWithSelector(context, astNodeType, context->roots.astNodeCompileIntoBytecodeSelector, sysbvm_function_createPrimitive(context, 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | (context->roots.astNodeType == astNodeType ? SYSBVM_FUNCTION_FLAGS_ABSTRACT : SYSBVM_FUNCTION_FLAGS_OVERRIDE), NULL, compilationFunction));
}

void sysbvm_functionBytecodeDirectCompiler_registerPrimitives(void)
{
    sysbvm_primitiveTable_registerFunction(sysbvm_astNode_primitiveCompileIntoBytecode, "ASTNode::doCompileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astBreakNode_primitiveCompileIntoBytecode, "ASTBreakNode::doCompileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astCoerceValueNode_primitiveCompileIntoBytecode, "ASTCoerceValueNode::doCompileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astContinueNode_primitiveCompileIntoBytecode, "ASTContinueNode::doCompileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astDoWhileContinueWithNode_primitiveCompileIntoBytecode, "ASTDoWhileContinueWithNode::doCompileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astDownCastNode_primitiveCompileIntoBytecode, "ASTDownCastNode::doCompileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astFunctionApplicationNode_primitiveCompileIntoBytecode, "ASTFunctionApplicationNode::doCompileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astIdentifierReferenceNode_primitiveCompileIntoBytecode, "ASTIdentifierReferenceNode::doCompileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astIfNode_primitiveCompileIntoBytecode, "ASTIfNode::doCompileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astLambdaNode_primitiveCompileIntoBytecode, "ASTLambdaNode::doCompileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astLexicalBlockNode_primitiveCompileIntoBytecode, "ASTLexicalBlockNode::doCompileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astLiteralNode_primitiveCompileIntoBytecode, "ASTLiteralNode::doCompileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astVariableDefinitionNode_primitiveCompileIntoBytecode, "ASTVariableDefinitionNode::doCompileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astMakeArrayNode_primitiveCompileIntoBytecode, "ASTMakeArrayNode::doCompileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astMakeAssociationNode_primitiveCompileIntoBytecode, "ASTMakeAssociationNode::doCompileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astMakeByteArrayNode_primitiveCompileIntoBytecode, "ASTMakeByteArrayNode::doCompileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astMakeDictionaryNode_primitiveCompileIntoBytecode, "ASTMakeDictionaryNode::doCompileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astMessageSendNode_primitiveCompileIntoBytecode, "ASTMessageSendNode::doCompileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astReturnNode_primitiveCompileIntoBytecode, "ASTReturnNode::doCompileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astSequenceNode_primitiveCompileIntoBytecode, "ASTSequenceNode::doCompileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astTupleSlotNamedAtNode_primitiveCompileIntoBytecode, "ASTTupleSlotNamedAtNode::doCompileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astTupleSlotNamedAtPutNode_primitiveCompileIntoBytecode, "ASTTupleSlotNamedAtPutNode::doCompileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astTupleSlotNamedReferenceAtNode_primitiveCompileIntoBytecode, "ASTTupleSlotNamedReferenceAtNode::doCompileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astUseNamedSlotsOfNode_primitiveCompileIntoBytecode, "ASTUseNamedSlotsOfNode::doCompileIntoBytecodeWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astWhileContinueNode_primitiveCompileIntoBytecode, "ASTWhileNodeNode::doCompileIntoBytecodeWith:");
}

void sysbvm_functionBytecodeDirectCompiler_setupPrimitives(sysbvm_context_t *context)
{
    sysbvm_functionBytecodeDirectCompiler_setupNodeCompilationFunction(context, context->roots.astNodeType, &sysbvm_astNode_primitiveCompileIntoBytecode);
    sysbvm_functionBytecodeDirectCompiler_setupNodeCompilationFunction(context, context->roots.astBreakNodeType, &sysbvm_astBreakNode_primitiveCompileIntoBytecode);
    sysbvm_functionBytecodeDirectCompiler_setupNodeCompilationFunction(context, context->roots.astCoerceValueNodeType, &sysbvm_astCoerceValueNode_primitiveCompileIntoBytecode);
    sysbvm_functionBytecodeDirectCompiler_setupNodeCompilationFunction(context, context->roots.astContinueNodeType, &sysbvm_astContinueNode_primitiveCompileIntoBytecode);
    sysbvm_functionBytecodeDirectCompiler_setupNodeCompilationFunction(context, context->roots.astDoWhileContinueWithNodeType, &sysbvm_astDoWhileContinueWithNode_primitiveCompileIntoBytecode);
    sysbvm_functionBytecodeDirectCompiler_setupNodeCompilationFunction(context, context->roots.astDownCastNodeType, &sysbvm_astDownCastNode_primitiveCompileIntoBytecode);
    sysbvm_functionBytecodeDirectCompiler_setupNodeCompilationFunction(context, context->roots.astFunctionApplicationNodeType, &sysbvm_astFunctionApplicationNode_primitiveCompileIntoBytecode);
    sysbvm_functionBytecodeDirectCompiler_setupNodeCompilationFunction(context, context->roots.astIdentifierReferenceNodeType, &sysbvm_astIdentifierReferenceNode_primitiveCompileIntoBytecode);
    sysbvm_functionBytecodeDirectCompiler_setupNodeCompilationFunction(context, context->roots.astIfNodeType, &sysbvm_astIfNode_primitiveCompileIntoBytecode);
    sysbvm_functionBytecodeDirectCompiler_setupNodeCompilationFunction(context, context->roots.astLambdaNodeType, &sysbvm_astLambdaNode_primitiveCompileIntoBytecode);
    sysbvm_functionBytecodeDirectCompiler_setupNodeCompilationFunction(context, context->roots.astLexicalBlockNodeType, &sysbvm_astLexicalBlockNode_primitiveCompileIntoBytecode);
    sysbvm_functionBytecodeDirectCompiler_setupNodeCompilationFunction(context, context->roots.astLiteralNodeType, &sysbvm_astLiteralNode_primitiveCompileIntoBytecode);
    sysbvm_functionBytecodeDirectCompiler_setupNodeCompilationFunction(context, context->roots.astVariableDefinitionNodeType, &sysbvm_astVariableDefinitionNode_primitiveCompileIntoBytecode);
    sysbvm_functionBytecodeDirectCompiler_setupNodeCompilationFunction(context, context->roots.astMakeArrayNodeType, &sysbvm_astMakeArrayNode_primitiveCompileIntoBytecode);
    sysbvm_functionBytecodeDirectCompiler_setupNodeCompilationFunction(context, context->roots.astMakeAssociationNodeType, &sysbvm_astMakeAssociationNode_primitiveCompileIntoBytecode);
    sysbvm_functionBytecodeDirectCompiler_setupNodeCompilationFunction(context, context->roots.astMakeByteArrayNodeType, &sysbvm_astMakeByteArrayNode_primitiveCompileIntoBytecode);
    sysbvm_functionBytecodeDirectCompiler_setupNodeCompilationFunction(context, context->roots.astMakeDictionaryNodeType, &sysbvm_astMakeDictionaryNode_primitiveCompileIntoBytecode);
    sysbvm_functionBytecodeDirectCompiler_setupNodeCompilationFunction(context, context->roots.astMessageSendNodeType, &sysbvm_astMessageSendNode_primitiveCompileIntoBytecode);
    sysbvm_functionBytecodeDirectCompiler_setupNodeCompilationFunction(context, context->roots.astReturnNodeType, &sysbvm_astReturnNode_primitiveCompileIntoBytecode);
    sysbvm_functionBytecodeDirectCompiler_setupNodeCompilationFunction(context, context->roots.astSequenceNodeType, &sysbvm_astSequenceNode_primitiveCompileIntoBytecode);
    sysbvm_functionBytecodeDirectCompiler_setupNodeCompilationFunction(context, context->roots.astTupleSlotNamedAtNodeType, &sysbvm_astTupleSlotNamedAtNode_primitiveCompileIntoBytecode);
    sysbvm_functionBytecodeDirectCompiler_setupNodeCompilationFunction(context, context->roots.astTupleSlotNamedAtPutNodeType, &sysbvm_astTupleSlotNamedAtPutNode_primitiveCompileIntoBytecode);
    sysbvm_functionBytecodeDirectCompiler_setupNodeCompilationFunction(context, context->roots.astTupleSlotNamedReferenceAtNodeType, &sysbvm_astTupleSlotNamedReferenceAtNode_primitiveCompileIntoBytecode);
    sysbvm_functionBytecodeDirectCompiler_setupNodeCompilationFunction(context, context->roots.astUseNamedSlotsOfNodeType, &sysbvm_astUseNamedSlotsOfNode_primitiveCompileIntoBytecode);
    sysbvm_functionBytecodeDirectCompiler_setupNodeCompilationFunction(context, context->roots.astWhileContinueWithNodeType, &sysbvm_astWhileContinueNode_primitiveCompileIntoBytecode);
}
