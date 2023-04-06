#include "internal/context.h"

typedef sysbvm_tuple_t (*sysbvm_bytecodeJit_entryPoint) (sysbvm_context_t *context, sysbvm_tuple_t function, size_t argumentCount, sysbvm_tuple_t *arguments);

typedef enum sysbvm_bytecodeJitRelocationType_e
{
    SYSBVM_BYTECODE_JIT_RELOCATION_RELATIVE32,
    SYSBVM_BYTECODE_JIT_RELOCATION_RELATIVE64,
} sysbvm_bytecodeJitRelocationType_t;

typedef struct sysbvm_bytecodeJitRelocation_s
{
    size_t offset;
    sysbvm_bytecodeJitRelocationType_t type;
    intptr_t value;
    intptr_t addend;
} sysbvm_bytecodeJitRelocation_t;

typedef struct sysbvm_bytecodeJitPCRelocation_s
{
    size_t offset;
    size_t targetPC;
    intptr_t addend;
} sysbvm_bytecodeJitPCRelocation_t;

typedef struct sysbvm_bytecodeJit_s
{
    sysbvm_context_t *context;

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

    size_t constantsCapacity;
    size_t constantsSize;
    uint8_t *constants;

    size_t relocationsCapacity;
    size_t relocationsSize;
    sysbvm_bytecodeJitRelocation_t *relocations;

    size_t pcRelocationsCapacity;
    size_t pcRelocationsSize;
    sysbvm_bytecodeJitPCRelocation_t *pcRelocations;

    intptr_t *pcDestinations;

    sysbvm_tuple_t *literalVectorGCRoot;
} sysbvm_bytecodeJit_t;

static size_t sizeAlignedTo(size_t pointer, size_t alignment)
{
    return (pointer + alignment - 1) & (~(alignment - 1));
}

static size_t sysbvm_bytecodeJit_addBytes(sysbvm_bytecodeJit_t *jit, size_t byteCount, uint8_t *bytes)
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

static size_t sysbvm_bytecodeJit_addByte(sysbvm_bytecodeJit_t *jit, uint8_t byte)
{
    sysbvm_bytecodeJit_addBytes(jit, 1, &byte);
    return jit->instructionsSize;
}

static size_t sysbvm_bytecodeJit_addConstantsBytes(sysbvm_bytecodeJit_t *jit, size_t byteCount, uint8_t *bytes)
{
    if(jit->constantsSize + byteCount > jit->constantsCapacity)
    {
        size_t newCapacity = jit->constantsCapacity * 2;
        if(newCapacity < 1024)
            newCapacity = 1024;

        uint8_t *newInstructions = (uint8_t*)malloc(newCapacity);
        memcpy(newInstructions, jit->constants, jit->constantsSize);
        free(jit->constants);
        jit->constants = newInstructions;
        jit->constantsCapacity = newCapacity;
    }

    size_t constantOffset = jit->constantsSize;
    memcpy(jit->constants + jit->constantsSize, bytes, byteCount);
    jit->constantsSize += byteCount;
    return constantOffset;
}

static void sysbvm_bytecodeJit_addPCRelocation(sysbvm_bytecodeJit_t *jit, sysbvm_bytecodeJitPCRelocation_t relocation)
{
    if(jit->pcRelocationsSize >= jit->pcRelocationsCapacity)
    {
        size_t newCapacity = jit->pcRelocationsCapacity * 2;
        if(newCapacity < 16)
            newCapacity = 16;

        sysbvm_bytecodeJitPCRelocation_t *newRelocations = (sysbvm_bytecodeJitPCRelocation_t*)malloc(newCapacity*sizeof(sysbvm_bytecodeJitPCRelocation_t));
        memcpy(newRelocations, jit->pcRelocations, jit->pcRelocationsSize * sizeof(sysbvm_bytecodeJitPCRelocation_t));
        free(jit->pcRelocations);
        jit->pcRelocations = newRelocations;
        jit->pcRelocationsCapacity = newCapacity;
    }

    jit->pcRelocations[jit->pcRelocationsSize++] = relocation;
}

static void sysbvm_bytecodeJit_addRelocation(sysbvm_bytecodeJit_t *jit, sysbvm_bytecodeJitRelocation_t relocation)
{
    if(jit->relocationsSize >= jit->relocationsCapacity)
    {
        size_t newCapacity = jit->relocationsCapacity * 2;
        if(newCapacity < 16)
            newCapacity = 16;

        sysbvm_bytecodeJitRelocation_t *newRelocations = (sysbvm_bytecodeJitRelocation_t*)malloc(newCapacity*sizeof(sysbvm_bytecodeJitRelocation_t));
        memcpy(newRelocations, jit->relocations, jit->relocationsSize * sizeof(sysbvm_bytecodeJitRelocation_t));
        free(jit->relocations);
        jit->relocations = newRelocations;
        jit->relocationsCapacity = newCapacity;
    }

    jit->relocations[jit->relocationsSize++] = relocation;
}

static void sysbvm_bytecodeJit_jitFree(sysbvm_bytecodeJit_t *jit)
{
    free(jit->instructions);
    free(jit->constants);
    free(jit->relocations);
    free(jit->pcRelocations);
    free(jit->pcDestinations);
}

static bool sysbvm_bytecodeJit_getLiteralValueForOperand(sysbvm_bytecodeJit_t *jit, int16_t operand, sysbvm_tuple_t *outLiteralValue)
{
    *outLiteralValue = SYSBVM_NULL_TUPLE;
    int16_t vectorType = operand & SYSBVM_OPERAND_VECTOR_BITMASK;
    int16_t vectorIndex = operand >> SYSBVM_OPERAND_VECTOR_BITS;
    sysbvm_tuple_t literalVector = *jit->literalVectorGCRoot;
    if(vectorType == SYSBVM_OPERAND_VECTOR_LITERAL &&
        vectorIndex >= 0 &&
        (size_t)vectorIndex < sysbvm_tuple_getSizeInSlots(literalVector))
    {
        *outLiteralValue = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(literalVector)->pointers[vectorIndex];
        return true;
    }

    return false;   
}

#if defined(__x86_64__)
#   include "bytecodeJitX86.c"
#elif defined(__aarch64__)
// CHECK Properly for ARMv8
#   include "bytecodeJitArmArch64.c"
#endif

static sysbvm_tuple_t sysbvm_bytecodeJit_slotAt(sysbvm_context_t *context, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot)
{
    size_t slotIndex = sysbvm_typeSlot_getIndex(typeSlot);
    return sysbvm_tuple_slotAt(context, tuple, slotIndex);
}

static sysbvm_tuple_t sysbvm_bytecodeJit_slotReferenceAt(sysbvm_context_t *context, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot)
{
    sysbvm_tuple_t slotReferenceType = sysbvm_typeSlot_getValidReferenceType(context, typeSlot);
    return sysbvm_referenceType_withTupleAndTypeSlot(context, slotReferenceType, tuple, typeSlot);
}

static void sysbvm_bytecodeJit_slotAtPut(sysbvm_context_t *context, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot, sysbvm_tuple_t value)
{
    size_t slotIndex = sysbvm_typeSlot_getIndex(typeSlot);
    sysbvm_tuple_slotAtPut(context, tuple, slotIndex, value);
}

static void sysbvm_bytecodeJit_jit(sysbvm_context_t *context, sysbvm_functionBytecode_t *functionBytecode)
{
    (void)context;
    sysbvm_bytecodeInterpreter_ensureTablesAreFilled();

    sysbvm_bytecodeJit_t jit = {
        .context = context,
    };

    jit.literalVectorGCRoot = sysbvm_heap_allocateGCRootTableEntry(&context->heap);
    *jit.literalVectorGCRoot = functionBytecode->literalVector;

    size_t instructionsSize = sysbvm_tuple_getSizeInBytes(functionBytecode->instructions);
    uint8_t *instructions = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(functionBytecode->instructions)->bytes;

    int16_t decodedOperands[SYSBVM_BYTECODE_FUNCTION_OPERAND_REGISTER_FILE_SIZE] = {};
    jit.argumentCount = sysbvm_tuple_size_decode(functionBytecode->argumentCount);
    jit.captureVectorSize = sysbvm_tuple_size_decode(functionBytecode->captureVectorSize);
    jit.literalCount = sysbvm_tuple_getSizeInSlots(functionBytecode->literalVector);
    jit.localVectorSize = sysbvm_tuple_size_decode(functionBytecode->localVectorSize);

    jit.pcDestinations = (intptr_t*)malloc(sizeof(intptr_t)*instructionsSize);
    memset(jit.pcDestinations, -1, sizeof(intptr_t)*instructionsSize);

    sysbvm_jit_prologue(&jit);

    size_t pc = 0;
    while(pc < instructionsSize)
    {
        jit.pcDestinations[pc] = jit.instructionsSize;
        sysbvm_jit_storePC(&jit, pc);

        uint8_t opcode = instructions[pc++];

        uint8_t standardOpcode = opcode;
        uint8_t operandCount = 0;
        if(opcode >= SYSBVM_OPCODE_FIRST_VARIABLE)
        {
            operandCount = (opcode & 0x0F) + sysbvm_implicitVariableBytecodeOperandCountTable[opcode >> 4];
            standardOpcode = opcode & 0xF0;
        }
        else
        {
            operandCount = opcode >> 4;
        }

        // Decode the operands.
        SYSBVM_ASSERT(pc + operandCount*2 <= instructionsSize);
        for(uint8_t i = 0; i < operandCount; ++i)
        {
            uint16_t lowByte = instructions[pc++];
            uint16_t highByte = instructions[pc++];
            decodedOperands[i] = lowByte | (highByte << 8);
        }

        // Validate the destination operands.
        uint8_t destinationOperandCount = sysbvm_bytecodeInterpreter_destinationOperandCountForOpcode(standardOpcode);
        uint8_t offsetOperandCount = sysbvm_bytecodeInterpreter_offsetOperandCountForOpcode(standardOpcode);

        for(uint8_t i = 0; i < destinationOperandCount; ++i)
        {
            if((decodedOperands[i] & SYSBVM_OPERAND_VECTOR_BITMASK) != SYSBVM_OPERAND_VECTOR_LOCAL)
                sysbvm_error("Bytecode destination operands must be in the local vector.");
            
            int16_t vectorIndex = decodedOperands[i] >> SYSBVM_OPERAND_VECTOR_BITS;
            if(vectorIndex >= 0 && (size_t)vectorIndex >= jit.localVectorSize)
                sysbvm_error("Bytecode destination operand is beyond the local vector bounds.");
        }

        // Fetch the source operands.
        for(uint8_t i = destinationOperandCount; i < operandCount - offsetOperandCount; ++i)
        {
            SYSBVM_ASSERT(i < SYSBVM_BYTECODE_FUNCTION_OPERAND_REGISTER_FILE_SIZE);

            int16_t decodedOperand = decodedOperands[i];
            int16_t vectorIndex = decodedOperand >> SYSBVM_OPERAND_VECTOR_BITS;
            uint8_t vectorType = decodedOperand & SYSBVM_OPERAND_VECTOR_BITMASK;
            switch(vectorType)
            {
            case SYSBVM_OPERAND_VECTOR_ARGUMENTS:
                if((size_t)vectorIndex >= jit.argumentCount)
                    sysbvm_error("Bytecode operand is beyond the argument vector bounds.");
                break;
            case SYSBVM_OPERAND_VECTOR_CAPTURES:
                if((size_t)vectorIndex >= jit.captureVectorSize)
                    sysbvm_error("Bytecode operand is beyond the capture vector bounds.");
                break;
            case SYSBVM_OPERAND_VECTOR_LITERAL:
                if((size_t)vectorIndex >= jit.literalCount)
                    sysbvm_error("Bytecode operand is beyond the literal vector bounds.");
                break;
            case SYSBVM_OPERAND_VECTOR_LOCAL:
                if((size_t)vectorIndex >= jit.localVectorSize)
                    sysbvm_error("Bytecode operand is beyond the local vector bounds.");
                break;
            default:
                abort();
            }
        }

        // Execute the opcodes.
        switch(standardOpcode)
        {
        // Zero operands
        case SYSBVM_OPCODE_NOP:
            // Nothing is required here.
            break;
        case SYSBVM_OPCODE_TRAP:
            // Nothing is required here.
            sysbvm_jit_callNoResult0(&jit, &sysbvm_error_trap);
            break;
        
        // One operands
        case SYSBVM_OPCODE_RETURN:
            sysbvm_jit_return(&jit, decodedOperands[0]);
            break;
        case SYSBVM_OPCODE_JUMP:
            if(decodedOperands[0] < 0)
                sysbvm_jit_callWithContextNoResult0(&jit, &sysbvm_gc_safepoint);
            sysbvm_jit_jumpRelative(&jit, pc + decodedOperands[0]);
            break;
        // Two operands.
        case SYSBVM_OPCODE_ALLOCA:
            sysbvm_jit_callWithContext1(&jit, &sysbvm_pointerLikeType_withEmptyBox, decodedOperands[0], decodedOperands[1]);
            break;
        case SYSBVM_OPCODE_LOAD:
            sysbvm_jit_callWithContext1(&jit, &sysbvm_pointerLikeType_load, decodedOperands[0], decodedOperands[1]);
            break;
        case SYSBVM_OPCODE_STORE:
            sysbvm_jit_callWithContextNoResult2(&jit, &sysbvm_pointerLikeType_store, decodedOperands[0], decodedOperands[1]);
            break;
        case SYSBVM_OPCODE_MOVE:
            sysbvm_jit_moveOperandToOperand(&jit, decodedOperands[0], decodedOperands[1]);
            break;
        case SYSBVM_OPCODE_JUMP_IF_TRUE:
            if(decodedOperands[1] < 0)
                sysbvm_jit_callWithContextNoResult0(&jit, &sysbvm_gc_safepoint);
            sysbvm_jit_jumpRelativeIfTrue(&jit, decodedOperands[0], pc + decodedOperands[1]);
            break;
        case SYSBVM_OPCODE_JUMP_IF_FALSE:
            if(decodedOperands[1] < 0)
                sysbvm_jit_callWithContextNoResult0(&jit, &sysbvm_gc_safepoint);
            sysbvm_jit_jumpRelativeIfFalse(&jit, decodedOperands[0], pc + decodedOperands[1]);
            break;

        case SYSBVM_OPCODE_TYPECHECK:
            sysbvm_jit_callWithContextNoResult2(&jit, &sysbvm_tuple_typecheckValue, decodedOperands[0], decodedOperands[1]);
            break;
        // Three operands.
        case SYSBVM_OPCODE_ALLOCA_WITH_VALUE:
            sysbvm_jit_callWithContext2(&jit, &sysbvm_pointerLikeType_withBoxForValue, decodedOperands[0], decodedOperands[1], decodedOperands[2]);
            break;
        case SYSBVM_OPCODE_COERCE_VALUE:
            sysbvm_jit_callWithContext2(&jit, &sysbvm_type_coerceValue, decodedOperands[0], decodedOperands[1], decodedOperands[2]);
            break;
        case SYSBVM_OPCODE_MAKE_ASSOCIATION:
            sysbvm_jit_callWithContext2(&jit, &sysbvm_association_create, decodedOperands[0], decodedOperands[1], decodedOperands[2]);
            break;
        case SYSBVM_OPCODE_MAKE_CLOSURE_WITH_VECTOR:
            sysbvm_jit_callWithContext2(&jit, &sysbvm_function_createClosureWithCaptureVector, decodedOperands[0], decodedOperands[1], decodedOperands[2]);
            break;
        case SYSBVM_OPCODE_SLOT_AT:
            sysbvm_jit_callWithContext2(&jit, &sysbvm_bytecodeJit_slotAt, decodedOperands[0], decodedOperands[1], decodedOperands[2]);
            break;
        case SYSBVM_OPCODE_SLOT_AT_PUT:
            sysbvm_jit_callWithContextNoResult3(&jit, &sysbvm_bytecodeJit_slotAtPut, decodedOperands[0], decodedOperands[1], decodedOperands[2]);
            break;
        case SYSBVM_OPCODE_SLOT_REFERENCE_AT:
            sysbvm_jit_callWithContext2(&jit, &sysbvm_bytecodeJit_slotReferenceAt, decodedOperands[0], decodedOperands[1], decodedOperands[2]);
            break;

        // Variable operand.
        case SYSBVM_OPCODE_CALL:
            sysbvm_jit_functionApply(&jit, decodedOperands[0], decodedOperands[1], opcode & 0xF, decodedOperands + 2, 0);
            break;
        case SYSBVM_OPCODE_UNCHECKED_CALL:
            sysbvm_jit_functionApply(&jit, decodedOperands[0], decodedOperands[1], opcode & 0xF, decodedOperands + 2, SYSBVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK);
            break;
        case SYSBVM_OPCODE_SEND:
            sysbvm_jit_send(&jit, decodedOperands[0], decodedOperands[1], opcode & 0xF, decodedOperands + 2, 0);
            break;
        case SYSBVM_OPCODE_SEND_WITH_LOOKUP:
            sysbvm_jit_sendWithReceiverType(&jit, decodedOperands[0], decodedOperands[1], decodedOperands[2], opcode & 0xF, decodedOperands + 3, 0);
            break;
        case SYSBVM_OPCODE_MAKE_ARRAY_WITH_ELEMENTS:
            sysbvm_jit_makeArray(&jit, decodedOperands[0], opcode & 0xF, decodedOperands + 1);
            break;
        case SYSBVM_OPCODE_MAKE_BYTE_ARRAY_WITH_ELEMENTS:
            sysbvm_jit_makeByteArray(&jit, decodedOperands[0], opcode & 0xF, decodedOperands + 1);
            break;
        case SYSBVM_OPCODE_MAKE_CLOSURE_WITH_CAPTURES:
            sysbvm_jit_makeClosureWithCaptures(&jit, decodedOperands[0], decodedOperands[1], opcode & 0xF, decodedOperands + 2);
            break;
        case SYSBVM_OPCODE_MAKE_DICTIONARY_WITH_ELEMENTS:
            sysbvm_jit_makeDictionary(&jit, decodedOperands[0], opcode & 0xF, decodedOperands + 1);
            break;
        default:
            sysbvm_error("Unsupported bytecode instruction.");
            break;
        }
    }

    sysbvm_jit_finish(&jit);

    size_t requiredCodeSize = sizeAlignedTo(jit.instructionsSize, 16) + sizeAlignedTo(jit.constantsSize, 16);
    uint8_t *codeZonePointer = sysbvm_heap_allocateAndLockCodeZone(&context->heap, requiredCodeSize, 16);
    memset(codeZonePointer, 0xcc, requiredCodeSize); // int3;
    sysbvm_jit_installIn(&jit, codeZonePointer);
    sysbvm_heap_unlockCodeZone(&context->heap, codeZonePointer, requiredCodeSize);

    functionBytecode->jittedCode = sysbvm_tuple_systemHandle_encode(context, (sysbvm_systemHandle_t)(uintptr_t)codeZonePointer);
    functionBytecode->jittedCodeSessionToken = context->roots.sessionToken;

    // Patch the trampoline.
    sysbvm_jit_patchTrampolineWithRealEntryPoint(&jit, functionBytecode);

    sysbvm_bytecodeJit_jitFree(&jit);
}
