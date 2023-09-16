#include "internal/context.h"
#include "internal/dynarray.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 
#define NOMINMAX
#include <windows.h>
#endif

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

    int32_t contextPointerOffset;
    int32_t localVectorOffset;
    int32_t argumentVectorOffset;
    int32_t literalVectorOffset;
    int32_t captureVectorOffset;
    int32_t pcOffset;
    int32_t stackFrameRecordOffset;
    int32_t callArgumentVectorSizeOffset;
    int32_t callArgumentVectorOffset;
    int32_t stackFrameSize;
    int32_t stackCallReservationSize;
    int32_t cfiFrameOffset;

    sysbvm_dynarray_t instructions;
    sysbvm_dynarray_t constants;
    sysbvm_dynarray_t relocations;
    sysbvm_dynarray_t pcRelocations;
    sysbvm_dynarray_t unwindInfo;
    sysbvm_dynarray_t unwindInfoBytecode;
    size_t prologueSize;

    intptr_t *pcDestinations;

    sysbvm_tuple_t *literalVectorGCRoot;
} sysbvm_bytecodeJit_t;

static size_t sizeAlignedTo(size_t pointer, size_t alignment)
{
    return (pointer + alignment - 1) & (~(alignment - 1));
}

static void sysbvm_bytecodeJit_initialize(sysbvm_bytecodeJit_t *jit, sysbvm_context_t *context)
{
    memset(jit, 0, sizeof(sysbvm_bytecodeJit_t));
    jit->context = context;
    sysbvm_dynarray_initialize(&jit->instructions, 1, 1024);
    sysbvm_dynarray_initialize(&jit->constants, 1, 1024);
    sysbvm_dynarray_initialize(&jit->relocations, sizeof(sysbvm_bytecodeJitRelocation_t), 0);
    sysbvm_dynarray_initialize(&jit->pcRelocations, sizeof(sysbvm_bytecodeJitPCRelocation_t), 0);
    sysbvm_dynarray_initialize(&jit->unwindInfo, 1, 64);
    sysbvm_dynarray_initialize(&jit->unwindInfoBytecode, 1, 64);
}

static size_t sysbvm_bytecodeJit_addBytes(sysbvm_bytecodeJit_t *jit, size_t byteCount, uint8_t *bytes)
{
    return sysbvm_dynarray_addAll(&jit->instructions, byteCount, bytes);
}

static size_t sysbvm_bytecodeJit_addByte(sysbvm_bytecodeJit_t *jit, uint8_t byte)
{
    return sysbvm_bytecodeJit_addBytes(jit, 1, &byte);
}

static size_t sysbvm_bytecodeJit_addConstantsBytes(sysbvm_bytecodeJit_t *jit, size_t byteCount, uint8_t *bytes)
{
    size_t offset = jit->constants.size;
    sysbvm_dynarray_addAll(&jit->constants, byteCount, bytes);
    return offset;
}

static size_t sysbvm_bytecodeJit_addUnwindInfoBytes(sysbvm_bytecodeJit_t *jit, size_t byteCount, uint8_t *bytes)
{
    size_t offset = jit->unwindInfo.size;
    sysbvm_dynarray_addAll(&jit->unwindInfo, byteCount, bytes);
    return offset;
}

static size_t sysbvm_bytecodeJit_addUnwindInfoByte(sysbvm_bytecodeJit_t *jit, uint8_t byte)
{
    return sysbvm_bytecodeJit_addUnwindInfoBytes(jit, 1, &byte);
}

#ifdef _WIN32
static void sysbvm_bytecodeJit_uwop(sysbvm_bytecodeJit_t *jit, uint8_t opcode, uint8_t operationInfo)
{
    uint8_t prologueOffset = (uint8_t)jit->instructions.size;
    uint8_t operation = (operationInfo << 4) | opcode;
    uint16_t code = prologueOffset | (operation << 8);
    sysbvm_dynarray_addAll(&jit->unwindInfoBytecode, 2, &code);
}

static void sysbvm_bytecodeJit_uwop_pushNonVol(sysbvm_bytecodeJit_t *jit, uint8_t reg)
{
    sysbvm_bytecodeJit_uwop(jit, /*UWOP_PUSH_NONVOL */0 , reg);
}

static void sysbvm_bytecodeJit_uwop_setFPReg(sysbvm_bytecodeJit_t *jit)
{
    sysbvm_bytecodeJit_uwop(jit, /* UWOP_SET_FPREG */3, 0);
}

static void sysbvm_bytecodeJit_uwop_alloc(sysbvm_bytecodeJit_t *jit, size_t amount)
{
    if(amount == 0) return;

    SYSBVM_ASSERT((amount % 8) == 0);
    if(amount <= 128)
    {
        sysbvm_bytecodeJit_uwop(jit, /* UWOP_ALLOC_SMALL */2, (uint8_t)(amount/8 - 1));
    }
    else if(amount <= 512*1024 - 8)
    {
        size_t encodedAmount = amount / 8;
        SYSBVM_ASSERT(encodedAmount <= 0xFFFF);
        uint16_t encodedAmountU16 = (uint16_t)encodedAmount;
        sysbvm_dynarray_addAll(&jit->unwindInfoBytecode, 2, &encodedAmountU16);
        sysbvm_bytecodeJit_uwop(jit, /* UWOP_ALLOC_LARGE */1, 0);
    }
    else
    {
        abort();
    }
}

#endif

static void sysbvm_bytecodeJit_addPCRelocation(sysbvm_bytecodeJit_t *jit, sysbvm_bytecodeJitPCRelocation_t relocation)
{
    sysbvm_dynarray_add(&jit->pcRelocations, &relocation);
}

static void sysbvm_bytecodeJit_addRelocation(sysbvm_bytecodeJit_t *jit, sysbvm_bytecodeJitRelocation_t relocation)
{
    sysbvm_dynarray_add(&jit->relocations, &relocation);
}

static void sysbvm_bytecodeJit_jitFree(sysbvm_bytecodeJit_t *jit)
{
    sysbvm_dynarray_destroy(&jit->instructions);
    sysbvm_dynarray_destroy(&jit->constants);
    sysbvm_dynarray_destroy(&jit->relocations);
    sysbvm_dynarray_destroy(&jit->pcRelocations);
    free(jit->pcDestinations);
    sysbvm_dynarray_destroy(&jit->unwindInfo);
    sysbvm_dynarray_destroy(&jit->unwindInfoBytecode);
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

#if defined(SYSBVM_ARCH_X86_64)
#   include "bytecodeJitX86.c"
#elif defined(SYSBVM_ARCH_AARCH64)
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

static sysbvm_tuple_t sysbvm_bytecodeJit_refSlotAt(sysbvm_context_t *context, sysbvm_tuple_t tupleReference, sysbvm_tuple_t typeSlot)
{
    size_t slotIndex = sysbvm_typeSlot_getIndex(typeSlot);
    return sysbvm_tuple_slotAt(context, sysbvm_pointerLikeType_load(context, tupleReference), slotIndex);
}

static sysbvm_tuple_t sysbvm_bytecodeJit_refSlotReferenceAt(sysbvm_context_t *context, sysbvm_tuple_t tupleReference, sysbvm_tuple_t typeSlot)
{
    sysbvm_tuple_t slotReferenceType = sysbvm_typeSlot_getValidReferenceType(context, typeSlot);
    return sysbvm_referenceType_incrementWithTypeSlot(context, slotReferenceType, tupleReference, typeSlot);
}

static void sysbvm_bytecodeJit_refSlotAtPut(sysbvm_context_t *context, sysbvm_tuple_t tupleReference, sysbvm_tuple_t typeSlot, sysbvm_tuple_t value)
{
    size_t slotIndex = sysbvm_typeSlot_getIndex(typeSlot);
    sysbvm_tuple_slotAtPut(context, sysbvm_pointerLikeType_load(context, tupleReference), slotIndex, value);
}

static void sysbvm_bytecodeJit_jit(sysbvm_context_t *context, sysbvm_functionBytecode_t *functionBytecode)
{
    (void)context;
    sysbvm_bytecodeInterpreter_ensureTablesAreFilled();

    sysbvm_bytecodeJit_t jit;
    sysbvm_bytecodeJit_initialize(&jit, context);

    jit.literalVectorGCRoot = sysbvm_heap_allocateGCRootTableEntry(&context->heap);
    *jit.literalVectorGCRoot = functionBytecode->literalVector;

    size_t instructionsSize = sysbvm_tuple_getSizeInBytes(functionBytecode->instructions);
    uint8_t *instructions = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(functionBytecode->instructions)->bytes;

    int16_t decodedOperands[SYSBVM_BYTECODE_FUNCTION_OPERAND_REGISTER_FILE_SIZE] = {0};
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
        jit.pcDestinations[pc] = jit.instructions.size;
        sysbvm_jit_storePC(&jit, (uint16_t)pc);

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

        // Three operands.
        case SYSBVM_OPCODE_ALLOCA_WITH_VALUE:
            sysbvm_jit_callWithContext2(&jit, &sysbvm_pointerLikeType_withBoxForValue, decodedOperands[0], decodedOperands[1], decodedOperands[2]);
            break;
        case SYSBVM_OPCODE_COERCE_VALUE:
            sysbvm_jit_callWithContext2(&jit, &sysbvm_type_coerceValue, decodedOperands[0], decodedOperands[1], decodedOperands[2]);
            break;
        case SYSBVM_OPCODE_DOWNCAST_VALUE:
            sysbvm_jit_callWithContext2(&jit, &sysbvm_type_downCastValue, decodedOperands[0], decodedOperands[1], decodedOperands[2]);
            break;
        case SYSBVM_OPCODE_UNCHECKED_DOWNCAST_VALUE:
            sysbvm_jit_moveOperandToOperand(&jit, decodedOperands[0], decodedOperands[2]);
            break;
        case SYSBVM_OPCODE_MAKE_ASSOCIATION:
            sysbvm_jit_callWithContext2(&jit, &sysbvm_association_create, decodedOperands[0], decodedOperands[1], decodedOperands[2]);
            break;
        case SYSBVM_OPCODE_MAKE_CLOSURE_WITH_VECTOR:
            sysbvm_jit_callWithContext2(&jit, &sysbvm_function_createClosureWithCaptureVectorArray, decodedOperands[0], decodedOperands[1], decodedOperands[2]);
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
        case SYSBVM_OPCODE_REF_SLOT_AT:
            sysbvm_jit_callWithContext2(&jit, &sysbvm_bytecodeJit_refSlotAt, decodedOperands[0], decodedOperands[1], decodedOperands[2]);
            break;
        case SYSBVM_OPCODE_REF_SLOT_AT_PUT:
            sysbvm_jit_callWithContextNoResult3(&jit, &sysbvm_bytecodeJit_refSlotAtPut, decodedOperands[0], decodedOperands[1], decodedOperands[2]);
            break;
        case SYSBVM_OPCODE_REF_SLOT_REFERENCE_AT:
            sysbvm_jit_callWithContext2(&jit, &sysbvm_bytecodeJit_refSlotReferenceAt, decodedOperands[0], decodedOperands[1], decodedOperands[2]);
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

    size_t textSectionSize = sizeAlignedTo(jit.instructions.size, 16);
    size_t rodataSectionSize = sizeAlignedTo(jit.constants.size, 16) + sizeAlignedTo(jit.unwindInfo.size, 16);

    size_t requiredCodeSize = textSectionSize + rodataSectionSize;
    uint8_t *codeZonePointer = sysbvm_heap_allocateAndLockCodeZone(&context->heap, requiredCodeSize, 16);
    memset(codeZonePointer, 0xcc, textSectionSize); // int3;
    memset(codeZonePointer + textSectionSize, 0, rodataSectionSize); // int3;
    sysbvm_jit_installIn(&jit, codeZonePointer);
    sysbvm_heap_unlockCodeZone(&context->heap, codeZonePointer, requiredCodeSize);

    functionBytecode->jittedCode = sysbvm_tuple_systemHandle_encode(context, (sysbvm_systemHandle_t)(uintptr_t)codeZonePointer);
    functionBytecode->jittedCodeSessionToken = context->roots.sessionToken;

    // Patch the trampoline.
    sysbvm_jit_patchTrampolineWithRealEntryPoint(&jit, functionBytecode);

    sysbvm_bytecodeJit_jitFree(&jit);
}
