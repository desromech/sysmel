#include "tuuvm/bytecode.h"
#include "tuuvm/array.h"
#include "tuuvm/assert.h"
#include "tuuvm/association.h"
#include "tuuvm/dictionary.h"
#include "tuuvm/context.h"
#include "tuuvm/function.h"
#include "tuuvm/message.h"
#include "tuuvm/gc.h"
#include "tuuvm/type.h"
#include "tuuvm/stackFrame.h"
#include "internal/context.h"
#include <alloca.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>

#if defined(__x86_64__) && !defined(_WIN32)
#   define TUUVM_JIT_SUPPORTED
#endif

static bool tuuvm_bytecodeInterpreter_tablesAreFilled;
static uint8_t tuuvm_implicitVariableBytecodeOperandCountTable[16];

static void tuuvm_bytecodeInterpreter_ensureTablesAreFilled()
{
    if(tuuvm_bytecodeInterpreter_tablesAreFilled)
        return;

    tuuvm_implicitVariableBytecodeOperandCountTable[TUUVM_OPCODE_CALL >> 4] = 2;
    tuuvm_implicitVariableBytecodeOperandCountTable[TUUVM_OPCODE_UNCHECKED_CALL >> 4] = 2;
    tuuvm_implicitVariableBytecodeOperandCountTable[TUUVM_OPCODE_SEND >> 4] = 3;
    tuuvm_implicitVariableBytecodeOperandCountTable[TUUVM_OPCODE_SEND_WITH_LOOKUP >> 4] = 4;

    tuuvm_implicitVariableBytecodeOperandCountTable[TUUVM_OPCODE_MAKE_ARRAY_WITH_ELEMENTS >> 4] = 1;
    tuuvm_implicitVariableBytecodeOperandCountTable[TUUVM_OPCODE_MAKE_BYTE_ARRAY_WITH_ELEMENTS >> 4] = 1;
    tuuvm_implicitVariableBytecodeOperandCountTable[TUUVM_OPCODE_MAKE_CLOSURE_WITH_CAPTURES >> 4] = 2;
    tuuvm_implicitVariableBytecodeOperandCountTable[TUUVM_OPCODE_MAKE_DICTIONARY_WITH_ELEMENTS >> 4] = 1;
    tuuvm_implicitVariableBytecodeOperandCountTable[TUUVM_OPCODE_MAKE_TUPLE_WITH_ELEMENTS >> 4] = 1;

    tuuvm_bytecodeInterpreter_tablesAreFilled = true;
}

TUUVM_API uint8_t tuuvm_bytecodeInterpreter_destinationOperandCountForOpcode(uint8_t opcode)
{
    switch(opcode)
    {
    case TUUVM_OPCODE_ALLOCA:
    case TUUVM_OPCODE_MOVE:
    case TUUVM_OPCODE_LOAD:
    case TUUVM_OPCODE_ALLOCA_WITH_VALUE:
    case TUUVM_OPCODE_COERCE_VALUE:
    case TUUVM_OPCODE_MAKE_ASSOCIATION:
    case TUUVM_OPCODE_MAKE_CLOSURE_WITH_VECTOR:
    case TUUVM_OPCODE_SLOT_AT:
    case TUUVM_OPCODE_SLOT_REFERENCE_AT:
    case TUUVM_OPCODE_CALL:
    case TUUVM_OPCODE_UNCHECKED_CALL:
    case TUUVM_OPCODE_SEND:
    case TUUVM_OPCODE_SEND_WITH_LOOKUP:
    case TUUVM_OPCODE_MAKE_ARRAY_WITH_ELEMENTS:
    case TUUVM_OPCODE_MAKE_BYTE_ARRAY_WITH_ELEMENTS:
    case TUUVM_OPCODE_MAKE_CLOSURE_WITH_CAPTURES:
    case TUUVM_OPCODE_MAKE_DICTIONARY_WITH_ELEMENTS:
    case TUUVM_OPCODE_MAKE_TUPLE_WITH_ELEMENTS:
        return 1;
    default:
        return 0;
    }
}

static uint8_t tuuvm_bytecodeInterpreter_offsetOperandCountForOpcode(uint8_t opcode)
{
    switch(opcode)
    {
    case TUUVM_OPCODE_JUMP:
    case TUUVM_OPCODE_JUMP_IF_TRUE:
    case TUUVM_OPCODE_JUMP_IF_FALSE:
        return 1;
    default: return 0;
    }
}

static tuuvm_tuple_t tuuvm_bytecodeInterpreter_functionApplyNoCopyArguments(tuuvm_context_t *context, tuuvm_tuple_t function, size_t argumentCount, tuuvm_tuple_t *arguments, tuuvm_bitflags_t applicationFlags)
{
    if(tuuvm_function_isVariadic(context, function))
    {
        size_t expectedArgumentCount = tuuvm_function_getArgumentCount(context, function);
        TUUVM_ASSERT(expectedArgumentCount > 0);

        // Move the variadic arguments into a variadic vector.
        {
            size_t directArgumentCount = expectedArgumentCount - 1;
            if(argumentCount < directArgumentCount)
                tuuvm_error("Missing required arguments.");

            size_t variadicArgumentCount = argumentCount - directArgumentCount;
            tuuvm_tuple_t variadicVector = tuuvm_array_create(context, variadicArgumentCount);
            tuuvm_tuple_t *variadicVectorElements = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(variadicVector)->pointers;
            for(size_t i = 0; i < variadicArgumentCount; ++i)
                variadicVectorElements[i] = arguments[directArgumentCount + i];
            arguments[directArgumentCount] = variadicVector;
        }

        return tuuvm_function_apply(context, function, expectedArgumentCount, arguments, applicationFlags);

    }
    return tuuvm_function_apply(context, function, argumentCount, arguments, applicationFlags);
}

static tuuvm_tuple_t tuuvm_bytecodeInterpreter_functionApply(tuuvm_context_t *context, tuuvm_tuple_t function, size_t argumentCount, tuuvm_tuple_t *arguments, tuuvm_bitflags_t applicationFlags)
{
    tuuvm_tuple_t argumentsBuffer[16];
    memcpy(argumentsBuffer, arguments, argumentCount * sizeof(tuuvm_tuple_t));

    return tuuvm_bytecodeInterpreter_functionApplyNoCopyArguments(context, function, argumentCount, argumentsBuffer, applicationFlags);
}

static tuuvm_tuple_t tuuvm_bytecodeInterpreter_interpretSend(tuuvm_context_t *context, tuuvm_tuple_t receiverType, tuuvm_tuple_t selector, size_t argumentCount, tuuvm_tuple_t *receiverAndArguments)
{
    tuuvm_tuple_t method = tuuvm_type_lookupSelector(context, receiverType, selector);
    if(method)
        return tuuvm_bytecodeInterpreter_functionApply(context, method, argumentCount + 1, receiverAndArguments, 0);

    // Attempt to send doesNotUnderstand:
    if(selector != context->roots.doesNotUnderstandSelector)
        method = tuuvm_type_lookupSelector(context, receiverType, selector);
    if(!method)
        tuuvm_error("Message not understood");

    // Make the message.
    tuuvm_tuple_t arguments = tuuvm_array_create(context, argumentCount);
    for(size_t i = 0; i < argumentCount; ++i)
        tuuvm_array_atPut(arguments, i, receiverAndArguments[1 + i]);

    tuuvm_tuple_t message = tuuvm_message_create(context, selector, arguments);
    return tuuvm_function_apply2(context, method, receiverAndArguments[0], message);
}

static tuuvm_tuple_t tuuvm_bytecodeInterpreter_interpretSendWithReceiverTypeNoCopyArguments(tuuvm_context_t *context, tuuvm_tuple_t receiverType, tuuvm_tuple_t selector, size_t argumentCount, tuuvm_tuple_t *receiverAndArguments, tuuvm_bitflags_t applicationFlags)
{
    tuuvm_tuple_t method = tuuvm_type_lookupSelector(context, receiverType, selector);
    if(method)
        return tuuvm_bytecodeInterpreter_functionApplyNoCopyArguments(context, method, argumentCount + 1, receiverAndArguments, applicationFlags);

    // Attempt to send doesNotUnderstand:
    if(selector != context->roots.doesNotUnderstandSelector)
        method = tuuvm_type_lookupSelector(context, receiverType, selector);
    if(!method)
        tuuvm_error("Message not understood");

    // Make the message.
    tuuvm_tuple_t arguments = tuuvm_array_create(context, argumentCount);
    for(size_t i = 0; i < argumentCount; ++i)
        tuuvm_array_atPut(arguments, i, receiverAndArguments[1 + i]);

    tuuvm_tuple_t message = tuuvm_message_create(context, selector, arguments);
    return tuuvm_function_apply2(context, method, receiverAndArguments[0], message);
}

static tuuvm_tuple_t tuuvm_bytecodeInterpreter_interpretSendNoCopyArguments(tuuvm_context_t *context, tuuvm_tuple_t selector, size_t argumentCount, tuuvm_tuple_t *receiverAndArguments, tuuvm_bitflags_t applicationFlags)
{
    return tuuvm_bytecodeInterpreter_interpretSendWithReceiverTypeNoCopyArguments(context, tuuvm_tuple_getType(context, receiverAndArguments[0]), selector, argumentCount, receiverAndArguments, applicationFlags);
}

TUUVM_API void tuuvm_bytecodeInterpreter_interpretWithActivationRecord(tuuvm_context_t *context, tuuvm_stackFrameBytecodeFunctionActivationRecord_t *activationRecord)
{
    tuuvm_bytecodeInterpreter_ensureTablesAreFilled();
    int16_t decodedOperands[TUUVM_BYTECODE_FUNCTION_OPERAND_REGISTER_FILE_SIZE] = {};
    tuuvm_tuple_t *operandRegisterFile = activationRecord->operandRegisterFile;
    tuuvm_tuple_t *localVector = activationRecord->inlineLocalVector;

    size_t instructionsSize;
    size_t pc;
    for(;;)
    {
        pc = activationRecord->pc;
        instructionsSize = tuuvm_tuple_getSizeInBytes(activationRecord->instructions);
        if(pc >= instructionsSize)
            break;

        uint8_t *instructions = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(activationRecord->instructions)->bytes;
        uint8_t opcode = instructions[pc++];

        uint8_t standardOpcode = opcode;
        uint8_t operandCount = 0;
        if(opcode >= TUUVM_OPCODE_FIRST_VARIABLE)
        {
            operandCount = (opcode & 0x0F) + tuuvm_implicitVariableBytecodeOperandCountTable[opcode >> 4];
            standardOpcode = opcode & 0xF0;
        }
        else
        {
            operandCount = opcode >> 4;
        }

        // Decode the operands.
        TUUVM_ASSERT(pc + operandCount*2 <= instructionsSize);
        for(uint8_t i = 0; i < operandCount; ++i)
        {
            uint16_t lowByte = instructions[pc++];
            uint16_t highByte = instructions[pc++];
            decodedOperands[i] = lowByte | (highByte << 8);
        }

        // Validate the destination operands.
        uint8_t destinationOperandCount = tuuvm_bytecodeInterpreter_destinationOperandCountForOpcode(standardOpcode);
        uint8_t offsetOperandCount = tuuvm_bytecodeInterpreter_offsetOperandCountForOpcode(standardOpcode);

        for(uint8_t i = 0; i < destinationOperandCount; ++i)
        {
            if((decodedOperands[i] & TUUVM_OPERAND_VECTOR_BITMASK) != TUUVM_OPERAND_VECTOR_LOCAL)
                tuuvm_error("Bytecode destination operands must be in the local vector.");
            
            decodedOperands[i] >>= TUUVM_OPERAND_VECTOR_BITS;
            if(decodedOperands[i] >= 0 && (size_t)decodedOperands[i] >= activationRecord->inlineLocalVectorSize)
                tuuvm_error("Bytecode destination operand is beyond the local vector bounds.");
        }

        // Fetch the source operands.
        for(uint8_t i = destinationOperandCount; i < operandCount - offsetOperandCount; ++i)
        {
            TUUVM_ASSERT(i < TUUVM_BYTECODE_FUNCTION_OPERAND_REGISTER_FILE_SIZE);

            int16_t decodedOperand = decodedOperands[i];
            int16_t vectorIndex = decodedOperand >> TUUVM_OPERAND_VECTOR_BITS;
            uint8_t vectorType = decodedOperand & TUUVM_OPERAND_VECTOR_BITMASK;
            switch(vectorType)
            {
            case TUUVM_OPERAND_VECTOR_ARGUMENTS:
                if((size_t)vectorIndex >= activationRecord->argumentCount)
                    tuuvm_error("Bytecode operand is beyond the argument vector bounds.");
                operandRegisterFile[i] = activationRecord->arguments[vectorIndex];
                break;
            case TUUVM_OPERAND_VECTOR_CAPTURES:
                if((size_t)vectorIndex >= tuuvm_tuple_getSizeInSlots(activationRecord->captureVector))
                    tuuvm_error("Bytecode operand is beyond the capture vector bounds.");
                operandRegisterFile[i] = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(activationRecord->captureVector)->pointers[vectorIndex];
                break;
            case TUUVM_OPERAND_VECTOR_LITERAL:
                if((size_t)vectorIndex >= tuuvm_tuple_getSizeInSlots(activationRecord->literalVector))
                    tuuvm_error("Bytecode operand is beyond the literal vector bounds.");
                operandRegisterFile[i] = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(activationRecord->literalVector)->pointers[vectorIndex];
                break;
            case TUUVM_OPERAND_VECTOR_LOCAL:
                if((size_t)vectorIndex >= activationRecord->inlineLocalVectorSize)
                    tuuvm_error("Bytecode operand is beyond the local vector bounds.");
                operandRegisterFile[i] = activationRecord->inlineLocalVector[vectorIndex];
                break;
            default:
                abort();
            }
        }

        // Execute the opcodes.
        bool isBackwardBranch = false;
        switch(standardOpcode)
        {
        // Zero operands
        case TUUVM_OPCODE_NOP:
            // Nothing is required here.
            break;
        case TUUVM_OPCODE_TRAP:
            // Nothing is required here.
            break;
        
        // One operands
        case TUUVM_OPCODE_RETURN:
            activationRecord->result = operandRegisterFile[0];
            return;
        case TUUVM_OPCODE_JUMP:
            pc += decodedOperands[0];
            isBackwardBranch = decodedOperands[0] < 0;
            break;

        // Two operands.
        case TUUVM_OPCODE_ALLOCA:
            operandRegisterFile[0] = tuuvm_pointerLikeType_withEmptyBox(context, operandRegisterFile[1]);
            break;
        case TUUVM_OPCODE_LOAD:
            operandRegisterFile[0] = tuuvm_pointerLikeType_load(context, operandRegisterFile[1]);
            break;
        case TUUVM_OPCODE_STORE:
            tuuvm_pointerLikeType_store(context, operandRegisterFile[0], operandRegisterFile[1]);
            break;
        case TUUVM_OPCODE_MOVE:
            operandRegisterFile[0] = operandRegisterFile[1];
            break;
        case TUUVM_OPCODE_JUMP_IF_TRUE:
            if(tuuvm_tuple_boolean_decode(operandRegisterFile[0]))
            {
                pc += decodedOperands[1];
                isBackwardBranch = decodedOperands[1] < 0;
            }
            break;
        case TUUVM_OPCODE_JUMP_IF_FALSE:
            if(!tuuvm_tuple_boolean_decode(operandRegisterFile[0]))
            {
                pc += decodedOperands[1];
                isBackwardBranch = decodedOperands[1 < 0];
            }
            break;
        case TUUVM_OPCODE_TYPECHECK:
            tuuvm_tuple_typecheckValue(context, operandRegisterFile[0], operandRegisterFile[1]);
            break;

        case TUUVM_OPCODE_SLOT_AT:
            {
                size_t slotIndex = tuuvm_typeSlot_getIndex(operandRegisterFile[2]);
                operandRegisterFile[0] = tuuvm_tuple_slotAt(context, operandRegisterFile[1], slotIndex);
            }
            break;
        case TUUVM_OPCODE_SLOT_REFERENCE_AT:
            {
                tuuvm_tuple_t slotReferenceType = tuuvm_typeSlot_getValidReferenceType(context, operandRegisterFile[2]);
                operandRegisterFile[0] = tuuvm_referenceType_withTupleAndTypeSlot(context, slotReferenceType, operandRegisterFile[1], operandRegisterFile[2]);
            }
            break;
        case TUUVM_OPCODE_SLOT_AT_PUT:
            {
                size_t slotIndex = tuuvm_typeSlot_getIndex(operandRegisterFile[1]);
                tuuvm_tuple_slotAtPut(context, operandRegisterFile[0], slotIndex, operandRegisterFile[2]);
            }
            break;

        // Three operands.
        case TUUVM_OPCODE_ALLOCA_WITH_VALUE:
            operandRegisterFile[0] = tuuvm_pointerLikeType_withBoxForValue(context, operandRegisterFile[1], operandRegisterFile[2]);
            break;
        case TUUVM_OPCODE_COERCE_VALUE:
            operandRegisterFile[0] = tuuvm_type_coerceValue(context, operandRegisterFile[1], operandRegisterFile[2]);
            break;
        case TUUVM_OPCODE_MAKE_ASSOCIATION:
            operandRegisterFile[0] = tuuvm_association_create(context, operandRegisterFile[1], operandRegisterFile[2]);
            break;
        case TUUVM_OPCODE_MAKE_CLOSURE_WITH_VECTOR:
            operandRegisterFile[0] = tuuvm_function_createClosureWithCaptureVector(context, operandRegisterFile[1], operandRegisterFile[2]);
            break;

        // Variable operand.
        case TUUVM_OPCODE_CALL:
            operandRegisterFile[0] = tuuvm_bytecodeInterpreter_functionApply(context, operandRegisterFile[1], opcode & 0xF, operandRegisterFile + 2, 0);
            break;
        case TUUVM_OPCODE_UNCHECKED_CALL:
            operandRegisterFile[0] = tuuvm_bytecodeInterpreter_functionApply(context, operandRegisterFile[1], opcode & 0xF, operandRegisterFile + 2, TUUVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK);
            break;
        case TUUVM_OPCODE_SEND:
            operandRegisterFile[0] = tuuvm_bytecodeInterpreter_interpretSend(context, tuuvm_tuple_getType(context, operandRegisterFile[2]), operandRegisterFile[1], opcode & 0xF, operandRegisterFile + 2);
            break;
        case TUUVM_OPCODE_SEND_WITH_LOOKUP:
            operandRegisterFile[0] = tuuvm_bytecodeInterpreter_interpretSend(context, operandRegisterFile[1], operandRegisterFile[2], opcode & 0xF, operandRegisterFile + 3);
            break;

        case TUUVM_OPCODE_MAKE_ARRAY_WITH_ELEMENTS:
            {
                size_t arraySize = opcode & 0xF;
                operandRegisterFile[0] = tuuvm_array_create(context, arraySize);
                tuuvm_tuple_t *arraySlots = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(operandRegisterFile[0])->pointers;
                for(size_t i = 0; i < arraySize; ++i)
                    arraySlots[i] = operandRegisterFile[1 + i];
            }
            break;
        case TUUVM_OPCODE_MAKE_BYTE_ARRAY_WITH_ELEMENTS:
            {
                size_t arraySize = opcode & 0xF;
                operandRegisterFile[0] = tuuvm_byteArray_create(context, arraySize);
                uint8_t *bytes = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(operandRegisterFile[0])->bytes;
                for(size_t i = 0; i < arraySize; ++i)
                    bytes[i] = tuuvm_tuple_uint8_decode(operandRegisterFile[1 + i]);
            }
            break;
        case TUUVM_OPCODE_MAKE_CLOSURE_WITH_CAPTURES:
            {
                size_t captureVectorSize = opcode & 0xF;
                operandRegisterFile[0] = tuuvm_array_create(context, captureVectorSize);
                tuuvm_tuple_t *captureVectorSlots = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(operandRegisterFile[0])->pointers;
                for(size_t i = 0; i < captureVectorSize; ++i)
                    captureVectorSlots[i] = operandRegisterFile[2 + i];
                operandRegisterFile[0] = tuuvm_function_createClosureWithCaptureVector(context, operandRegisterFile[1], operandRegisterFile[0]);
            }
            break;
        case TUUVM_OPCODE_MAKE_DICTIONARY_WITH_ELEMENTS:
            {
                size_t dictionarySize = opcode & 0xF;
                operandRegisterFile[0] = tuuvm_dictionary_createWithCapacity(context, dictionarySize);
                for(size_t i = 0; i < dictionarySize; ++i)
                    tuuvm_dictionary_add(context, operandRegisterFile[0], operandRegisterFile[1 + i]);
            }
            break;
        default:
            tuuvm_error("Unsupported bytecode instruction.");
            break;
        }

        // Write back the destinations and the new PC.
        for(uint8_t i = 0; i < destinationOperandCount; ++i)
        {
            if(decodedOperands[i] >= 0)
                localVector[decodedOperands[i]] = operandRegisterFile[i];
        }
        activationRecord->pc = pc;

        // Safepoint in backward branches.
        if(isBackwardBranch)
            tuuvm_gc_safepoint(context);
    }

    TUUVM_ASSERT(activationRecord->pc < tuuvm_tuple_getSizeInBytes(activationRecord->instructions));
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeInterpreter_getSourcePositionForPC(tuuvm_context_t *context, tuuvm_functionBytecode_t *functionBytecode, size_t pc)
{
    (void)context;
    if(!functionBytecode->pcToDebugListTable)
        return TUUVM_NULL_TUPLE;

    size_t pcToDebugTableListSize = tuuvm_tuple_getSizeInSlots(functionBytecode->pcToDebugListTable) / 2;
    tuuvm_tuple_t *pcToDebugTableEntries = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(functionBytecode->pcToDebugListTable)->pointers;

    // Binary search
    size_t lower = 0;
    size_t upper = pcToDebugTableListSize;
    intptr_t bestFound = -1;
    while(lower < upper)
    {
        size_t middle = lower + (upper - lower) / 2;
        size_t entryPC = tuuvm_tuple_size_decode(pcToDebugTableEntries[middle*2]);
        if(entryPC <= pc)
        {
            lower = middle + 1;
            bestFound = tuuvm_tuple_size_decode(pcToDebugTableEntries[middle*2 + 1]);
        }
        else
        {
            upper = middle;
        }
    }

    size_t sourcePositionsTableSize = tuuvm_tuple_getSizeInSlots(functionBytecode->debugSourcePositions);
    if(bestFound < 0 || (size_t)bestFound >= sourcePositionsTableSize)
        return TUUVM_NULL_TUPLE;

    return TUUVM_CAST_OOP_TO_OBJECT_TUPLE(functionBytecode->debugSourcePositions)->pointers[bestFound];
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeInterpreter_getSourcePositionForActivationRecord(tuuvm_context_t *context, tuuvm_stackFrameBytecodeFunctionActivationRecord_t *activationRecord)
{
    tuuvm_functionBytecode_t **functionBytecodeObject = (tuuvm_functionBytecode_t **)&activationRecord->functionBytecode;
    tuuvm_tuple_t actualSourcePosition = tuuvm_bytecodeInterpreter_getSourcePositionForPC(context, *functionBytecodeObject, activationRecord->pc);
    if(actualSourcePosition)
        return actualSourcePosition;

    tuuvm_functionDefinition_t **functionDefinitionObject = (tuuvm_functionDefinition_t**)&activationRecord->functionDefinition;
    return (*functionDefinitionObject)->sourcePosition;
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeInterpreter_getSourcePositionForJitActivationRecord(tuuvm_context_t *context, tuuvm_stackFrameBytecodeFunctionJitActivationRecord_t *activationRecord)
{
    tuuvm_function_t *functionObject = (tuuvm_function_t*)activationRecord->function;
    tuuvm_functionDefinition_t *functionDefinitionObject = (tuuvm_functionDefinition_t*)functionObject->definition;
    tuuvm_functionBytecode_t *functionBytecodeObject = (tuuvm_functionBytecode_t *)functionDefinitionObject->bytecode;
    tuuvm_tuple_t actualSourcePosition = tuuvm_bytecodeInterpreter_getSourcePositionForPC(context, functionBytecodeObject, activationRecord->pc);
    if(actualSourcePosition)
        return actualSourcePosition;

    return functionDefinitionObject->sourcePosition;
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeInterpreter_activateAndApply(tuuvm_context_t *context, tuuvm_tuple_t function_, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    tuuvm_stackFrameBytecodeFunctionActivationRecord_t activationRecord = {
        .type = TUUVM_STACK_FRAME_RECORD_TYPE_BYTECODE_FUNCTION_ACTIVATION,
    };
    tuuvm_stackFrame_pushRecord((tuuvm_stackFrameRecord_t*)&activationRecord);  

    activationRecord.function = function_;
    tuuvm_function_t **functionObject = (tuuvm_function_t**)&activationRecord.function;
    
    activationRecord.functionDefinition = (*functionObject)->definition;
    tuuvm_functionDefinition_t **functionDefinitionObject = (tuuvm_functionDefinition_t**)&activationRecord.functionDefinition;

    activationRecord.functionBytecode = (*functionDefinitionObject)->bytecode;
    tuuvm_functionBytecode_t **functionBytecodeObject = (tuuvm_functionBytecode_t **)&activationRecord.functionBytecode;

    activationRecord.argumentCount = argumentCount;
    activationRecord.arguments = arguments;

    activationRecord.captureVector = (*functionObject)->captureVector;
    activationRecord.literalVector = (*functionBytecodeObject)->literalVector;
    activationRecord.instructions = (*functionBytecodeObject)->instructions;
    TUUVM_ASSERT(tuuvm_tuple_isBytes(activationRecord.instructions));

    // Set the initial PC.
    activationRecord.pc = 0;

    // Allocate the inline local vector.
    size_t requiredLocalVectorSize = tuuvm_tuple_size_decode((*functionBytecodeObject)->localVectorSize);
    tuuvm_tuple_t *inlineLocalVector = (tuuvm_tuple_t *)alloca(requiredLocalVectorSize * sizeof(tuuvm_tuple_t));
    memset(inlineLocalVector, 0, requiredLocalVectorSize * sizeof(tuuvm_tuple_t));

    activationRecord.inlineLocalVectorSize = requiredLocalVectorSize;
    activationRecord.inlineLocalVector = inlineLocalVector;

    // Interpret.
    if(!_setjmp(activationRecord.jmpbuffer))
        tuuvm_bytecodeInterpreter_interpretWithActivationRecord(context, &activationRecord);

    tuuvm_stackFrame_popRecord((tuuvm_stackFrameRecord_t*)&activationRecord);
    return activationRecord.result;
}

#ifdef TUUVM_JIT_SUPPORTED
#include "bytecodeJitCommon.c"
#endif

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeInterpreter_apply(tuuvm_context_t *context, tuuvm_tuple_t function, size_t argumentCount, tuuvm_tuple_t *arguments)
{
#ifdef TUUVM_JIT_SUPPORTED
    if(context->jitEnabled)
    {
        tuuvm_function_t *functionObject = (tuuvm_function_t*)function;
        tuuvm_functionDefinition_t *functionDefinitionObject = (tuuvm_functionDefinition_t*)functionObject->definition;
        tuuvm_functionBytecode_t *functionBytecodeObject = (tuuvm_functionBytecode_t *)functionDefinitionObject->bytecode;
        if(!functionBytecodeObject->jittedCode || functionBytecodeObject->jittedCodeSessionToken != context->roots.sessionToken)
            tuuvm_bytecodeJit_jit(context, functionBytecodeObject);

        if(functionBytecodeObject->jittedCode && functionBytecodeObject->jittedCodeSessionToken == context->roots.sessionToken)
        {
            tuuvm_bytecodeJit_entryPoint entryPoint = (tuuvm_bytecodeJit_entryPoint)tuuvm_tuple_systemHandle_decode(functionBytecodeObject->jittedCode);
            return entryPoint(context, function, argumentCount, arguments);
        }
    }
#endif

    return tuuvm_bytecodeInterpreter_activateAndApply(context, function, argumentCount, arguments);
}

TUUVM_API tuuvm_tuple_t tuuvm_bytecodeInterpreter_applyJitTrampolineDestination(tuuvm_context_t *context, tuuvm_tuple_t function, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    return tuuvm_bytecodeInterpreter_apply(context, function, argumentCount, arguments);
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
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::TypeCheck", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_TYPECHECK));

    // Three operands.
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::AllocaWithValue", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_ALLOCA_WITH_VALUE));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::CoerceValue", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_COERCE_VALUE));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::MakeAssociation", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_MAKE_ASSOCIATION));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::MakeClosureWithVector", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_MAKE_CLOSURE_WITH_VECTOR));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::SlotAt", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_SLOT_AT));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::SlotReferenceAt", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_SLOT_REFERENCE_AT));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::SlotAtPut", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_SLOT_AT_PUT));

    // Variable operand count.
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::Call", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_CALL));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::UncheckedCall", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_UNCHECKED_CALL));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::Send", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_SEND));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::SendWithLookup", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_SEND_WITH_LOOKUP));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::MakeArrayWithElements", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_MAKE_ARRAY_WITH_ELEMENTS));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::MakeByteArrayWithElements", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_MAKE_BYTE_ARRAY_WITH_ELEMENTS));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::MakeClosureWithCaptures", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_MAKE_CLOSURE_WITH_CAPTURES));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::MakeDictionaryWithElements", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_MAKE_DICTIONARY_WITH_ELEMENTS));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::MakeTupleWithElements", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_MAKE_TUPLE_WITH_ELEMENTS));

    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::FirstVariable", tuuvm_tuple_uint8_encode(TUUVM_OPCODE_FIRST_VARIABLE));

    // Export the operand vector names.
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::OperandVectorName::Arguments", tuuvm_tuple_int16_encode(TUUVM_OPERAND_VECTOR_ARGUMENTS));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::OperandVectorName::Captures", tuuvm_tuple_int16_encode(TUUVM_OPERAND_VECTOR_CAPTURES));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::OperandVectorName::Literal", tuuvm_tuple_int16_encode(TUUVM_OPERAND_VECTOR_LITERAL));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::OperandVectorName::Local", tuuvm_tuple_int16_encode(TUUVM_OPERAND_VECTOR_LOCAL));

    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::OperandVectorName::Bits", tuuvm_tuple_int16_encode(TUUVM_OPERAND_VECTOR_BITS));
    tuuvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::OperandVectorName::BitMask", tuuvm_tuple_int16_encode(TUUVM_OPERAND_VECTOR_BITMASK));
}
