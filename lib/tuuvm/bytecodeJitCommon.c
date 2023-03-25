#include "internal/context.h"

typedef tuuvm_tuple_t (*tuuvm_bytecodeJit_entryPoint) (tuuvm_context_t *context, tuuvm_tuple_t function, size_t argumentCount, tuuvm_tuple_t *arguments);

typedef enum tuuvm_bytecodeJitRelocationType_e
{
    TUUVM_BYTECODE_JIT_RELOCATION_RELATIVE32,
    TUUVM_BYTECODE_JIT_RELOCATION_RELATIVE64,
} tuuvm_bytecodeJitRelocationType_t;

typedef struct tuuvm_bytecodeJitRelocation_s
{
    size_t offset;
    tuuvm_bytecodeJitRelocationType_t type;
    intptr_t value;
    intptr_t addend;
} tuuvm_bytecodeJitRelocation_t;

typedef struct tuuvm_bytecodeJitPCRelocation_s
{
    size_t offset;
    size_t targetPC;
    intptr_t addend;
} tuuvm_bytecodeJitPCRelocation_t;

typedef struct tuuvm_bytecodeJit_s
{
    size_t argumentCount;
    size_t captureVectorSize;
    size_t literalCount;
    size_t localVectorSize;

    int32_t localVectorOffset;
    int32_t argumentVectorOffset;
    int32_t literalVectorOffset;
    int32_t captureVectorOffset;
    int32_t pcOffset;
    int32_t stackFrameRecordOffset;

    size_t instructionsCapacity;
    size_t instructionsSize;
    uint8_t *instructions;

    size_t relocationsCapacity;
    size_t relocationsSize;
    tuuvm_bytecodeJitRelocation_t *relocations;

    size_t pcRelocationsCapacity;
    size_t pcRelocationsSize;
    tuuvm_bytecodeJitPCRelocation_t *pcRelocations;

    intptr_t *pcDestinations;

    tuuvm_tuple_t *literalVectorGCRoot;
} tuuvm_bytecodeJit_t;

static size_t tuuvm_bytecodeJit_addBytes(tuuvm_bytecodeJit_t *jit, size_t byteCount, uint8_t *bytes)
{
    if(jit->instructionsSize + byteCount > jit->instructionsCapacity)
    {
        size_t newCapacity = jit->instructionsCapacity * 2;
        if(newCapacity < 1024)
            newCapacity = 1024;

        uint8_t *newInstructions = (uint8_t*)malloc(newCapacity);
        memcpy(newInstructions, jit->instructions, jit->instructionsSize);
        free(jit->instructions);
        jit->instructions = newInstructions;
        jit->instructionsCapacity = newCapacity;
    }

    memcpy(jit->instructions + jit->instructionsSize, bytes, byteCount);
    jit->instructionsSize += byteCount;
    return jit->instructionsSize;
}

static size_t tuuvm_bytecodeJit_addByte(tuuvm_bytecodeJit_t *jit, uint8_t byte)
{
    tuuvm_bytecodeJit_addBytes(jit, 1, &byte);
    return jit->instructionsSize;
}

static void tuuvm_bytecodeJit_addPCRelocation(tuuvm_bytecodeJit_t *jit, tuuvm_bytecodeJitPCRelocation_t relocation)
{
    if(jit->pcRelocationsSize >= jit->pcRelocationsCapacity)
    {
        size_t newCapacity = jit->pcRelocationsCapacity * 2;
        if(newCapacity < 16)
            newCapacity = 16;

        tuuvm_bytecodeJitPCRelocation_t *newRelocations = (tuuvm_bytecodeJitPCRelocation_t*)malloc(newCapacity*sizeof(tuuvm_bytecodeJitPCRelocation_t));
        memcpy(newRelocations, jit->pcRelocations, jit->pcRelocationsSize * sizeof(tuuvm_bytecodeJitPCRelocation_t));
        free(jit->pcRelocations);
        jit->pcRelocations = newRelocations;
        jit->pcRelocationsCapacity = newCapacity;
    }

    jit->pcRelocations[jit->pcRelocationsSize++] = relocation;
}

static void tuuvm_bytecodeJit_addRelocation(tuuvm_bytecodeJit_t *jit, tuuvm_bytecodeJitRelocation_t relocation)
{
    if(jit->relocationsSize >= jit->relocationsCapacity)
    {
        size_t newCapacity = jit->relocationsCapacity * 2;
        if(newCapacity < 16)
            newCapacity = 16;

        tuuvm_bytecodeJitRelocation_t *newRelocations = (tuuvm_bytecodeJitRelocation_t*)malloc(newCapacity*sizeof(tuuvm_bytecodeJitRelocation_t));
        memcpy(newRelocations, jit->relocations, jit->relocationsSize * sizeof(tuuvm_bytecodeJitRelocation_t));
        free(jit->relocations);
        jit->relocations = newRelocations;
        jit->relocationsCapacity = newCapacity;
    }

    jit->relocations[jit->relocationsSize++] = relocation;
}

static void tuuvm_bytecodeJit_jitFree(tuuvm_bytecodeJit_t *jit)
{
    free(jit->instructions);
    free(jit->relocations);
    free(jit->pcRelocations);
    free(jit->pcDestinations);
}

#if defined(__x86_64__)
#   include "bytecodeJitX86.c"
#elif defined(__aarch64__)
// CHECK Properly for ARMv8
#   include "bytecodeJitArmArch64.c"
#endif

static void tuuvm_bytecodeJit_jit(tuuvm_context_t *context, tuuvm_functionBytecode_t *functionBytecode)
{
    (void)context;
    tuuvm_bytecodeInterpreter_ensureTablesAreFilled();

    tuuvm_bytecodeJit_t jit = {};

    jit.literalVectorGCRoot = tuuvm_heap_allocateGCRootTableEntry(&context->heap);
    *jit.literalVectorGCRoot = functionBytecode->literalVector;

    size_t instructionsSize = tuuvm_tuple_getSizeInBytes(functionBytecode->instructions);
    uint8_t *instructions = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(functionBytecode->instructions)->bytes;

    int16_t decodedOperands[TUUVM_BYTECODE_FUNCTION_OPERAND_REGISTER_FILE_SIZE] = {};
    jit.argumentCount = tuuvm_tuple_size_decode(functionBytecode->argumentCount);
    jit.captureVectorSize = tuuvm_tuple_size_decode(functionBytecode->captureVectorSize);
    jit.literalCount = tuuvm_tuple_getSizeInSlots(functionBytecode->literalVector);
    jit.localVectorSize = tuuvm_tuple_size_decode(functionBytecode->localVectorSize);

    jit.pcDestinations = (intptr_t*)malloc(sizeof(intptr_t)*instructionsSize);
    memset(jit.pcDestinations, -1, sizeof(intptr_t)*instructionsSize);

    tuuvm_jit_prologue(&jit);

    size_t pc = 0;
    while(pc < instructionsSize)
    {
        jit.pcDestinations[pc] = jit.instructionsSize;
        tuuvm_jit_storePC(&jit, pc);

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
            
            int16_t vectorIndex = decodedOperands[i] >> TUUVM_OPERAND_VECTOR_BITS;
            if(vectorIndex >= 0 && (size_t)vectorIndex >= jit.localVectorSize)
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
                if((size_t)vectorIndex >= jit.argumentCount)
                    tuuvm_error("Bytecode operand is beyond the argument vector bounds.");
                break;
            case TUUVM_OPERAND_VECTOR_CAPTURES:
                if((size_t)vectorIndex >= jit.captureVectorSize)
                    tuuvm_error("Bytecode operand is beyond the capture vector bounds.");
                break;
            case TUUVM_OPERAND_VECTOR_LITERAL:
                if((size_t)vectorIndex >= jit.literalCount)
                    tuuvm_error("Bytecode operand is beyond the literal vector bounds.");
                break;
            case TUUVM_OPERAND_VECTOR_LOCAL:
                if((size_t)vectorIndex >= jit.localVectorSize)
                    tuuvm_error("Bytecode operand is beyond the local vector bounds.");
                break;
            default:
                abort();
            }
        }

        // Execute the opcodes.
        switch(standardOpcode)
        {
        // Zero operands
        case TUUVM_OPCODE_NOP:
            // Nothing is required here.
            tuuvm_jit_trap(&jit);
            break;
        case TUUVM_OPCODE_TRAP:
            // Nothing is required here.
            tuuvm_jit_callNoResult0(&jit, &tuuvm_error_trap);
            break;
        
        // One operands
        case TUUVM_OPCODE_RETURN:
            tuuvm_jit_return(&jit, decodedOperands[0]);
            break;
        case TUUVM_OPCODE_JUMP:
            if(decodedOperands[0] < 0)
                tuuvm_jit_callNoResult0(&jit, &tuuvm_gc_safepoint);
            tuuvm_jit_jumpRelative(&jit, pc + decodedOperands[0]);
            break;
        // Two operands.
        case TUUVM_OPCODE_ALLOCA:
            tuuvm_jit_callWithContext1(&jit, &tuuvm_pointerLikeType_withEmptyBox, decodedOperands[0], decodedOperands[1]);
            break;
        case TUUVM_OPCODE_LOAD:
            tuuvm_jit_callWithContext1(&jit, &tuuvm_pointerLikeType_load, decodedOperands[0], decodedOperands[1]);
            break;
        case TUUVM_OPCODE_STORE:
            tuuvm_jit_callWithContextNoResult2(&jit, &tuuvm_pointerLikeType_store, decodedOperands[0], decodedOperands[1]);
            break;
        case TUUVM_OPCODE_MOVE:
            tuuvm_jit_moveOperandToOperand(&jit, decodedOperands[0], decodedOperands[1]);
            break;
        case TUUVM_OPCODE_JUMP_IF_TRUE:
            tuuvm_jit_jumpRelativeIfTrue(&jit, decodedOperands[0], pc + decodedOperands[1]);
            break;
        case TUUVM_OPCODE_JUMP_IF_FALSE:
            tuuvm_jit_jumpRelativeIfFalse(&jit, decodedOperands[0], pc + decodedOperands[1]);
            break;

        // Three operands.
        case TUUVM_OPCODE_ALLOCA_WITH_VALUE:
            tuuvm_jit_callWithContext2(&jit, &tuuvm_pointerLikeType_withBoxForValue, decodedOperands[0], decodedOperands[1], decodedOperands[2]);
            break;
        case TUUVM_OPCODE_COERCE_VALUE:
            tuuvm_jit_callWithContext2(&jit, &tuuvm_type_coerceValue, decodedOperands[0], decodedOperands[1], decodedOperands[2]);
            break;
        case TUUVM_OPCODE_MAKE_ASSOCIATION:
            tuuvm_jit_callWithContext2(&jit, &tuuvm_association_create, decodedOperands[0], decodedOperands[1], decodedOperands[2]);
            break;
        case TUUVM_OPCODE_MAKE_CLOSURE_WITH_VECTOR:
            tuuvm_jit_callWithContext2(&jit, &tuuvm_function_createClosureWithCaptureVector, decodedOperands[0], decodedOperands[1], decodedOperands[2]);
            break;

        // Variable operand.
        case TUUVM_OPCODE_CALL:
            tuuvm_jit_functionApply(&jit, decodedOperands[0], decodedOperands[1], opcode & 0xF, decodedOperands + 2, 0);
            break;
        case TUUVM_OPCODE_UNCHECKED_CALL:
            tuuvm_jit_functionApply(&jit, decodedOperands[0], decodedOperands[1], opcode & 0xF, decodedOperands + 2, TUUVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK);
            break;
        case TUUVM_OPCODE_SEND:
            tuuvm_jit_send(&jit, decodedOperands[0], decodedOperands[1], opcode & 0xF, decodedOperands + 2, 0);
            break;
        case TUUVM_OPCODE_SEND_WITH_LOOKUP:
            tuuvm_jit_sendWithReceiverType(&jit, decodedOperands[0], decodedOperands[1], decodedOperands[2], opcode & 0xF, decodedOperands + 3, 0);
            break;
        case TUUVM_OPCODE_MAKE_ARRAY_WITH_ELEMENTS:
            tuuvm_jit_makeArray(&jit, decodedOperands[0], opcode & 0xF, decodedOperands + 1);
            break;
        case TUUVM_OPCODE_MAKE_BYTE_ARRAY_WITH_ELEMENTS:
            tuuvm_jit_makeByteArray(&jit, decodedOperands[0], opcode & 0xF, decodedOperands + 1);
            break;
        case TUUVM_OPCODE_MAKE_CLOSURE_WITH_CAPTURES:
            tuuvm_jit_makeClosureWithCaptures(&jit, decodedOperands[0], decodedOperands[1], opcode & 0xF, decodedOperands + 2);
            break;
        case TUUVM_OPCODE_MAKE_DICTIONARY_WITH_ELEMENTS:
            tuuvm_jit_makeDictionary(&jit, decodedOperands[0], opcode & 0xF, decodedOperands + 1);
            break;
        default:
            tuuvm_error("Unsupported bytecode instruction.");
            break;
        }
    }

    tuuvm_jit_finish(&jit);

    uint8_t *codeZonePointer = tuuvm_heap_allocateAndLockCodeZone(&context->heap, jit.instructionsSize, 16);
    tuuvm_jit_installIn(&jit, codeZonePointer);
    tuuvm_heap_unlockCodeZone(&context->heap, codeZonePointer, jit.instructionsSize);

    functionBytecode->jittedCode = tuuvm_tuple_systemHandle_encode(context, (tuuvm_systemHandle_t)(uintptr_t)codeZonePointer);
    functionBytecode->jittedCodeSessionToken = context->roots.sessionToken;

    tuuvm_bytecodeJit_jitFree(&jit);
}
