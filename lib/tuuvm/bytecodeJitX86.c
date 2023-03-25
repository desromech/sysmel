typedef enum tuuvm_x86_register_e
{
    TUUVM_X86_RAX = 0,
    TUUVM_X86_RCX = 1,
    TUUVM_X86_RDX = 2,
    TUUVM_X86_RBX = 3,
    TUUVM_X86_RSP = 4,
    TUUVM_X86_RBP = 5,
    TUUVM_X86_RSI = 6,
    TUUVM_X86_RDI = 7,

    TUUVM_X86_R8 = 8,
    TUUVM_X86_R9 = 9,
    TUUVM_X86_R10 = 10,
    TUUVM_X86_R11 = 11,
    TUUVM_X86_R12 = 12,
    TUUVM_X86_R13 = 13,
    TUUVM_X86_R14 = 14,
    TUUVM_X86_R15 = 15,

    TUUVM_X86_REG_HALF_MASK = 7,

    TUUVM_X86_SYSV_ARG0 = TUUVM_X86_RDI,
    TUUVM_X86_SYSV_ARG1 = TUUVM_X86_RSI,
    TUUVM_X86_SYSV_ARG2 = TUUVM_X86_RDX,
    TUUVM_X86_SYSV_ARG3 = TUUVM_X86_RCX,
    TUUVM_X86_SYSV_ARG4 = TUUVM_X86_R8,
    TUUVM_X86_SYSV_ARG5 = TUUVM_X86_R9,
} tuuvm_x86_register_t;

static void tuuvm_jit_x86_mov64Absolute(tuuvm_bytecodeJit_t *jit, tuuvm_x86_register_t destination, uint64_t value);
static void tuuvm_jit_moveRegisterToOperand(tuuvm_bytecodeJit_t *jit, int16_t operand, tuuvm_x86_register_t reg);
static void tuuvm_jit_moveOperandToRegister(tuuvm_bytecodeJit_t *jit, tuuvm_x86_register_t reg, int16_t operand);
static void tuuvm_jit_moveOperandToOperand(tuuvm_bytecodeJit_t *jit, int16_t destinationOperand, int16_t sourceOperand);
static void tuuvm_jit_pushOperand(tuuvm_bytecodeJit_t *jit, int16_t operand);

static uint8_t tuuvm_jit_x86_modRM(int8_t rm, uint8_t regOpcode, uint8_t mod)
{
    return (rm & TUUVM_X86_REG_HALF_MASK) | ((regOpcode & TUUVM_X86_REG_HALF_MASK) << 3) | (mod << 6);
}

static uint8_t tuuvm_jit_x86_modRMRegister(tuuvm_x86_register_t rm, tuuvm_x86_register_t reg)
{
    return tuuvm_jit_x86_modRM(rm, reg, 3);
}

static uint8_t tuuvm_jit_x86_rex(bool W, bool R, bool X, bool B)
{
    return 0x40 | ((W ? 1 : 0) << 3) | ((R ? 1 : 0) << 2) | ((X ? 1 : 0) << 1) | (B ? 1 : 0);
}

static void tuuvm_jit_x86_call(tuuvm_bytecodeJit_t *jit, void *functionPointer)
{
#if 0
    uint8_t instruction[] = {
        0xE8, 0x00, 0x00, 0x00, 0x00
    };

    size_t relocationOffset = tuuvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction) - 4;
    
    // This direct copy is placed here for debugging purposes.
    int32_t disp = (intptr_t) functionPointer - (intptr_t)(jit->instructions + jit->instructionsSize);
    memcpy(jit->instructions + relocationOffset, &disp, 4);

    tuuvm_bytecodeJitRelocation_t relocation = {
        .offset = relocationOffset,
        .type = TUUVM_BYTECODE_JIT_RELOCATION_RELATIVE32,
        .value = (intptr_t)functionPointer,
        .addend = -4
    };
    tuuvm_bytecodeJit_addRelocation(jit, relocation);
#elif 0
    tuuvm_jit_x86_mov64Absolute(jit, TUUVM_X86_RAX, (uint64_t)functionPointer);
    uint8_t instruction[] = {
        0xFF,
        tuuvm_jit_x86_modRMRegister(TUUVM_X86_RAX, 2),
    };

    tuuvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
#else
    size_t constantOffset = tuuvm_bytecodeJit_addConstantsBytes(jit, sizeof(functionPointer), (uint8_t*)&functionPointer);
    uint8_t instruction[] = {
        0xFF,
        tuuvm_jit_x86_modRM(5, 2, 0),
        0x00, 0x00, 0x00, 0x00,
    };

    size_t relocationOffset = tuuvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction) - 4;

    tuuvm_bytecodeJitRelocation_t relocation = {
        .offset = relocationOffset,
        .type = TUUVM_BYTECODE_JIT_RELOCATION_RELATIVE32,
        .value = constantOffset,
        .addend = -4
    };
    tuuvm_bytecodeJit_addRelocation(jit, relocation);
#endif
}

static void tuuvm_jit_x86_pushRegister(tuuvm_bytecodeJit_t *jit, tuuvm_x86_register_t reg)
{
    if(reg > TUUVM_X86_REG_HALF_MASK)
        tuuvm_bytecodeJit_addByte(jit, tuuvm_jit_x86_rex(false, false, false, true));
    tuuvm_bytecodeJit_addByte(jit, 0x50 + (reg & TUUVM_X86_REG_HALF_MASK));
}

static void tuuvm_jit_x86_pushImmediate32(tuuvm_bytecodeJit_t *jit, int32_t immediate)
{
    uint8_t instruction[] = {
        0x68,
        immediate & 0xFF, (immediate >> 8) & 0xFF, (immediate >> 16) & 0xFF, (immediate >> 24) & 0xFF,
    };

    tuuvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void tuuvm_jit_x86_pushFromMemoryWithOffset(tuuvm_bytecodeJit_t *jit, tuuvm_x86_register_t reg, int32_t offset)
{
    uint8_t instruction[] = {
        0xFF,
        tuuvm_jit_x86_modRM(reg, 6, 2),
        offset & 0xFF, (offset >> 8) & 0xFF, (offset >> 16) & 0xFF, (offset >> 24) & 0xFF,
    };

    tuuvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void tuuvm_jit_x86_popRegister(tuuvm_bytecodeJit_t *jit, tuuvm_x86_register_t reg)
{
    if(reg > TUUVM_X86_REG_HALF_MASK)
        tuuvm_bytecodeJit_addByte(jit, tuuvm_jit_x86_rex(false, false, false, true));
    tuuvm_bytecodeJit_addByte(jit, 0x58 + (reg & TUUVM_X86_REG_HALF_MASK));
}

static void tuuvm_jit_x86_endbr64(tuuvm_bytecodeJit_t *jit)
{
    uint8_t instruction[] = {
        0xF3, 0x0F, 0x1E, 0xFA,
    };

    tuuvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void tuuvm_jit_x86_ret(tuuvm_bytecodeJit_t *jit)
{
    tuuvm_bytecodeJit_addByte(jit, 0xc3);
}

static void tuuvm_jit_x86_int3(tuuvm_bytecodeJit_t *jit)
{
    tuuvm_bytecodeJit_addByte(jit, 0xcc);
}

static void tuuvm_jit_x86_mov64Register(tuuvm_bytecodeJit_t *jit, tuuvm_x86_register_t destination, tuuvm_x86_register_t source)
{
    uint8_t instruction[] = {
        tuuvm_jit_x86_rex(true, source > TUUVM_X86_REG_HALF_MASK, false, destination > TUUVM_X86_REG_HALF_MASK),
        0x8B,
        tuuvm_jit_x86_modRMRegister(source, destination),
    };

    tuuvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void tuuvm_jit_x86_mov64Absolute(tuuvm_bytecodeJit_t *jit, tuuvm_x86_register_t destination, uint64_t value)
{
    uint8_t instruction[] = {
        tuuvm_jit_x86_rex(true, false, false, destination > TUUVM_X86_REG_HALF_MASK),
        0xB8 + (destination & TUUVM_X86_REG_HALF_MASK),
        value & 0xFF, (value >> 8) & 0xFF, (value >> 16) & 0xFF, (value >> 24) & 0xFF,
        (value >> 32) & 0xFF, (value >> 40) & 0xFF, (value >> 48) & 0xFF, (value >> 56) & 0xFF,
    };

    tuuvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void tuuvm_jit_x86_addImmediate32(tuuvm_bytecodeJit_t *jit, tuuvm_x86_register_t destination, int32_t value)
{
    if(value == 0)
        return;

    uint8_t instruction[] = {
        tuuvm_jit_x86_rex(true, false, false, destination > TUUVM_X86_REG_HALF_MASK),
        0x81,
        tuuvm_jit_x86_modRMRegister(destination, 0),
        value & 0xFF, (value >> 8) & 0xFF, (value >> 16) & 0xFF, (value >> 24) & 0xFF,
    };

    tuuvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void tuuvm_jit_x86_subImmediate32(tuuvm_bytecodeJit_t *jit, tuuvm_x86_register_t destination, int32_t value)
{
    if(value == 0)
        return;

    uint8_t instruction[] = {
        tuuvm_jit_x86_rex(true, false, false, destination > TUUVM_X86_REG_HALF_MASK),
        0x81,
        tuuvm_jit_x86_modRMRegister(destination, 5),
        value & 0xFF, (value >> 8) & 0xFF, (value >> 16) & 0xFF, (value >> 24) & 0xFF,
    };

    tuuvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void tuuvm_jit_x86_movImmediate32(tuuvm_bytecodeJit_t *jit, tuuvm_x86_register_t destination, int32_t value)
{
    uint8_t instruction[] = {
        tuuvm_jit_x86_rex(true, false, false, destination > TUUVM_X86_REG_HALF_MASK),
        0xC7,
        tuuvm_jit_x86_modRMRegister(destination, 0),
        value & 0xFF, (value >> 8) & 0xFF, (value >> 16) & 0xFF, (value >> 24) & 0xFF,
    };

    tuuvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void tuuvm_jit_x86_mov64FromMemoryWithOffset(tuuvm_bytecodeJit_t *jit, tuuvm_x86_register_t destination, tuuvm_x86_register_t source, int32_t offset)
{
    if(offset == 0 && source != TUUVM_X86_RBP)
    {
        uint8_t instruction[] = {
            tuuvm_jit_x86_rex(true, source > TUUVM_X86_REG_HALF_MASK, false, destination > TUUVM_X86_REG_HALF_MASK),
            0x8B,
            tuuvm_jit_x86_modRM(source, destination, 0),
        };

        tuuvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
    }
    else
    {
        uint8_t instruction[] = {
            tuuvm_jit_x86_rex(true, source > TUUVM_X86_REG_HALF_MASK, false, destination > TUUVM_X86_REG_HALF_MASK),
            0x8B,
            tuuvm_jit_x86_modRM(source, destination, 2),
            offset & 0xFF, (offset >> 8) & 0xFF, (offset >> 16) & 0xFF, (offset >> 24) & 0xFF,
        };

        tuuvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
    }
}

static void tuuvm_jit_x86_mov64IntoMemoryWithOffset(tuuvm_bytecodeJit_t *jit, tuuvm_x86_register_t destination, int32_t offset, tuuvm_x86_register_t source)
{
    if(offset == 0 && destination != TUUVM_X86_RBP)
    {
        uint8_t instruction[] = {
            tuuvm_jit_x86_rex(true, destination > TUUVM_X86_REG_HALF_MASK, false, source > TUUVM_X86_REG_HALF_MASK),
            0x89,
            tuuvm_jit_x86_modRM(destination, source, 0),
        };

        tuuvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
    }
    else
    {
        uint8_t instruction[] = {
            tuuvm_jit_x86_rex(true, destination > TUUVM_X86_REG_HALF_MASK, false, source > TUUVM_X86_REG_HALF_MASK),
            0x89,
            tuuvm_jit_x86_modRM(destination, source, 2),
            offset & 0xFF, (offset >> 8) & 0xFF, (offset >> 16) & 0xFF, (offset >> 24) & 0xFF,
        };

        tuuvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
    }
}


static void tuuvm_jit_x86_logicalShiftRightImmediate(tuuvm_bytecodeJit_t *jit, tuuvm_x86_register_t destination, uint8_t shiftAmount)
{
    uint8_t instruction[] = {
        tuuvm_jit_x86_rex(true, destination > TUUVM_X86_REG_HALF_MASK, false, false),
        0xC1,
        tuuvm_jit_x86_modRMRegister(destination, 5),
        shiftAmount
    };

    tuuvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void tuuvm_jit_x86_mov8IntoMemoryWithOffset(tuuvm_bytecodeJit_t *jit, tuuvm_x86_register_t destination, int32_t offset, tuuvm_x86_register_t source)
{
    if(offset == 0 && destination != TUUVM_X86_RBP)
    {
        uint8_t instruction[] = {
            tuuvm_jit_x86_rex(false, destination > TUUVM_X86_REG_HALF_MASK, false, source > TUUVM_X86_REG_HALF_MASK),
            0x88,
            tuuvm_jit_x86_modRM(destination, source, 0),
        };

        tuuvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
    }
    else
    {
        uint8_t instruction[] = {
            tuuvm_jit_x86_rex(false, destination > TUUVM_X86_REG_HALF_MASK, false, source > TUUVM_X86_REG_HALF_MASK),
            0x88,
            tuuvm_jit_x86_modRM(destination, source, 2),
            offset & 0xFF, (offset >> 8) & 0xFF, (offset >> 16) & 0xFF, (offset >> 24) & 0xFF,
        };

        tuuvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
    }
}

static void tuuvm_jit_x86_movImmediateI32IntoMemory64WithOffset(tuuvm_bytecodeJit_t *jit, tuuvm_x86_register_t destination, int32_t offset, int16_t immediate)
{
    uint8_t instruction[] = {
        tuuvm_jit_x86_rex(true, destination > TUUVM_X86_REG_HALF_MASK, false, false),
        0xC7,
        tuuvm_jit_x86_modRM(destination, 0, 2),
        offset & 0xFF, (offset >> 8) & 0xFF, (offset >> 16) & 0xFF, (offset >> 24) & 0xFF,
        immediate & 0xFF, (immediate >> 8) & 0xFF, (immediate >> 16) & 0xFF, (immediate >> 24) & 0xFF,
    };

    tuuvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}


static void tuuvm_jit_x86_xorRegister(tuuvm_bytecodeJit_t *jit, tuuvm_x86_register_t destination, tuuvm_x86_register_t source)
{
    uint8_t instruction[] = {
        tuuvm_jit_x86_rex(true, destination > TUUVM_X86_REG_HALF_MASK, false, destination > TUUVM_X86_REG_HALF_MASK),
        0x33,
        tuuvm_jit_x86_modRMRegister(source, destination),
    };

    tuuvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void tuuvm_jit_trap(tuuvm_bytecodeJit_t *jit)
{
    tuuvm_jit_x86_int3(jit);
}

static void tuuvm_jit_x86_jitLoadContextInRegister(tuuvm_bytecodeJit_t *jit, tuuvm_x86_register_t reg)
{
    tuuvm_jit_x86_mov64FromMemoryWithOffset(jit, reg, TUUVM_X86_RBP, -(int32_t)sizeof(void*));
}

static void tuuvm_jit_callNoResult0(tuuvm_bytecodeJit_t *jit, void *functionPointer)
{
    tuuvm_jit_x86_call(jit, functionPointer);
}

static void tuuvm_jit_callWithContextNoResult0(tuuvm_bytecodeJit_t *jit, void *functionPointer)
{
    tuuvm_jit_x86_jitLoadContextInRegister(jit, TUUVM_X86_SYSV_ARG0);
    tuuvm_jit_x86_call(jit, functionPointer);
}

static void tuuvm_jit_callWithContext1(tuuvm_bytecodeJit_t *jit, void *functionPointer, int16_t resultOperand, int16_t argumentOperand0)
{
    tuuvm_jit_x86_jitLoadContextInRegister(jit, TUUVM_X86_SYSV_ARG0);
    tuuvm_jit_moveOperandToRegister(jit, TUUVM_X86_SYSV_ARG1, argumentOperand0);
    tuuvm_jit_x86_call(jit, functionPointer);
    tuuvm_jit_moveRegisterToOperand(jit, resultOperand, TUUVM_X86_RAX);
}

static void tuuvm_jit_callWithContext2(tuuvm_bytecodeJit_t *jit, void *functionPointer, int16_t resultOperand, int16_t argumentOperand0, int16_t argumentOperand1)
{
    tuuvm_jit_x86_jitLoadContextInRegister(jit, TUUVM_X86_SYSV_ARG0);
    tuuvm_jit_moveOperandToRegister(jit, TUUVM_X86_SYSV_ARG1, argumentOperand0);
    tuuvm_jit_moveOperandToRegister(jit, TUUVM_X86_SYSV_ARG2, argumentOperand1);
    tuuvm_jit_x86_call(jit, functionPointer);
    tuuvm_jit_moveRegisterToOperand(jit, resultOperand, TUUVM_X86_RAX);
}

static void tuuvm_jit_callWithContextNoResult2(tuuvm_bytecodeJit_t *jit, void *functionPointer, int16_t argumentOperand0, int16_t argumentOperand1)
{
    tuuvm_jit_x86_jitLoadContextInRegister(jit, TUUVM_X86_SYSV_ARG0);
    tuuvm_jit_moveOperandToRegister(jit, TUUVM_X86_SYSV_ARG1, argumentOperand0);
    tuuvm_jit_moveOperandToRegister(jit, TUUVM_X86_SYSV_ARG2, argumentOperand1);
    tuuvm_jit_x86_call(jit, functionPointer);
}

static void tuuvm_jit_functionApply(tuuvm_bytecodeJit_t *jit, int16_t resultOperand, int16_t functionOperand, size_t argumentCount, int16_t *argumentOperands, int applicationFlags)
{
    size_t stackSize = argumentCount * sizeof(void*);
    size_t alignedStackSize = (stackSize + 15) & (-16);
    size_t paddingSize = alignedStackSize - stackSize;
    tuuvm_jit_x86_subImmediate32(jit, TUUVM_X86_RSP, paddingSize);

    // Push all of the arguments in the stack.
    for(size_t i = 0; i < argumentCount; ++i)
        tuuvm_jit_pushOperand(jit, argumentOperands[argumentCount - i - 1]);

    tuuvm_jit_x86_jitLoadContextInRegister(jit, TUUVM_X86_SYSV_ARG0);
    tuuvm_jit_moveOperandToRegister(jit, TUUVM_X86_SYSV_ARG1, functionOperand);
    tuuvm_jit_x86_movImmediate32(jit, TUUVM_X86_SYSV_ARG2, argumentCount);
    tuuvm_jit_x86_mov64Register(jit, TUUVM_X86_SYSV_ARG3, TUUVM_X86_RSP);
    tuuvm_jit_x86_movImmediate32(jit, TUUVM_X86_SYSV_ARG4, applicationFlags);
    tuuvm_jit_x86_call(jit, &tuuvm_bytecodeInterpreter_functionApplyNoCopyArguments);
    tuuvm_jit_x86_addImmediate32(jit, TUUVM_X86_RSP, paddingSize + argumentCount * sizeof(void*));

    tuuvm_jit_moveRegisterToOperand(jit, resultOperand, TUUVM_X86_RAX);
}

static void tuuvm_jit_send(tuuvm_bytecodeJit_t *jit, int16_t resultOperand, int16_t selectorOperand, size_t argumentCount, int16_t *argumentOperands, int applicationFlags)
{
    size_t stackSize = (argumentCount + 1) * sizeof(void*);
    size_t alignedStackSize = (stackSize + 15) & (-16);
    size_t paddingSize = alignedStackSize - stackSize;
    tuuvm_jit_x86_subImmediate32(jit, TUUVM_X86_RSP, paddingSize);

    // Push all of the arguments in the stack.
    size_t pushCount = argumentCount + 1;
    for(size_t i = 0; i < pushCount; ++i)
        tuuvm_jit_pushOperand(jit, argumentOperands[pushCount - i - 1]);

    tuuvm_jit_x86_jitLoadContextInRegister(jit, TUUVM_X86_SYSV_ARG0);
    tuuvm_jit_moveOperandToRegister(jit, TUUVM_X86_SYSV_ARG1, selectorOperand);
    tuuvm_jit_x86_movImmediate32(jit, TUUVM_X86_SYSV_ARG2, argumentCount);
    tuuvm_jit_x86_mov64Register(jit, TUUVM_X86_SYSV_ARG3, TUUVM_X86_RSP);
    tuuvm_jit_x86_movImmediate32(jit, TUUVM_X86_SYSV_ARG4, applicationFlags);
    tuuvm_jit_x86_call(jit, &tuuvm_bytecodeInterpreter_interpretSendNoCopyArguments);
    tuuvm_jit_x86_addImmediate32(jit, TUUVM_X86_RSP, paddingSize + (1 + argumentCount) * sizeof(void*));

    tuuvm_jit_moveRegisterToOperand(jit, resultOperand, TUUVM_X86_RAX);
}

static void tuuvm_jit_sendWithReceiverType(tuuvm_bytecodeJit_t *jit, int16_t resultOperand, int16_t receiverTypeOperand, int16_t selectorOperand, size_t argumentCount, int16_t *argumentOperands, int applicationFlags)
{
    size_t stackSize = (argumentCount + 1 + 1) * sizeof(void*);
    size_t alignedStackSize = (stackSize + 15) & (-16);
    size_t paddingSize = alignedStackSize - stackSize;
    tuuvm_jit_x86_subImmediate32(jit, TUUVM_X86_RSP, paddingSize);

    // Push all of the arguments in the stack.
    size_t pushCount = argumentCount + 1;
    for(size_t i = 0; i < pushCount; ++i)
        tuuvm_jit_pushOperand(jit, argumentOperands[pushCount - i - 1]);
    tuuvm_jit_x86_mov64Register(jit, TUUVM_X86_SYSV_ARG5, TUUVM_X86_RSP);

    tuuvm_jit_x86_pushImmediate32(jit, applicationFlags);

    tuuvm_jit_x86_jitLoadContextInRegister(jit, TUUVM_X86_SYSV_ARG0);
    tuuvm_jit_moveOperandToRegister(jit, TUUVM_X86_SYSV_ARG2, receiverTypeOperand);
    tuuvm_jit_moveOperandToRegister(jit, TUUVM_X86_SYSV_ARG3, selectorOperand);
    tuuvm_jit_x86_movImmediate32(jit, TUUVM_X86_SYSV_ARG4, argumentCount);
    tuuvm_jit_x86_call(jit, &tuuvm_bytecodeInterpreter_interpretSendWithReceiverTypeNoCopyArguments);
    tuuvm_jit_x86_addImmediate32(jit, TUUVM_X86_RSP, (/* application flags */ 1 + /* receiver */1 + argumentCount) * sizeof(void*));

    tuuvm_jit_moveRegisterToOperand(jit, resultOperand, TUUVM_X86_RAX);
}

static void tuuvm_jit_makeArray(tuuvm_bytecodeJit_t *jit, int16_t resultOperand, size_t elementCount, int16_t *elementOperands)
{
    if(resultOperand < 0)
        return;

    tuuvm_jit_x86_jitLoadContextInRegister(jit, TUUVM_X86_SYSV_ARG0);
    tuuvm_jit_x86_movImmediate32(jit, TUUVM_X86_SYSV_ARG1, elementCount);
    tuuvm_jit_x86_call(jit, &tuuvm_array_create);

    for(size_t i = 0; i < elementCount; ++i)
    {
        tuuvm_jit_moveOperandToRegister(jit, TUUVM_X86_RSI, elementOperands[i]);
        tuuvm_jit_x86_mov64IntoMemoryWithOffset(jit, TUUVM_X86_RAX, sizeof(tuuvm_tuple_header_t) + i * sizeof(void*), TUUVM_X86_RSI);
    }

    tuuvm_jit_moveRegisterToOperand(jit, resultOperand, TUUVM_X86_RAX);
}

static void tuuvm_jit_makeByteArray(tuuvm_bytecodeJit_t *jit, int16_t resultOperand, size_t elementCount, int16_t *elementOperands)
{
    if(resultOperand < 0)
        return;

    tuuvm_jit_x86_jitLoadContextInRegister(jit, TUUVM_X86_SYSV_ARG0);
    tuuvm_jit_x86_movImmediate32(jit, TUUVM_X86_SYSV_ARG1, elementCount);
    tuuvm_jit_x86_call(jit, &tuuvm_byteArray_create);

    for(size_t i = 0; i < elementCount; ++i)
    {
        tuuvm_jit_moveOperandToRegister(jit, TUUVM_X86_RSI, elementOperands[i]);
        tuuvm_jit_x86_logicalShiftRightImmediate(jit, TUUVM_X86_RSI, TUUVM_TUPLE_TAG_BIT_COUNT);
        tuuvm_jit_x86_mov8IntoMemoryWithOffset(jit, TUUVM_X86_RAX, sizeof(tuuvm_tuple_header_t) + i, TUUVM_X86_RSI);
    }

    tuuvm_jit_moveRegisterToOperand(jit, resultOperand, TUUVM_X86_RAX);
}

static void tuuvm_jit_makeDictionary(tuuvm_bytecodeJit_t *jit, int16_t resultOperand, size_t elementCount, int16_t *elementOperands)
{
    if(resultOperand < 0)
        return;

    tuuvm_jit_x86_jitLoadContextInRegister(jit, TUUVM_X86_SYSV_ARG0);
    tuuvm_jit_x86_movImmediate32(jit, TUUVM_X86_SYSV_ARG1, elementCount);
    tuuvm_jit_x86_call(jit, &tuuvm_dictionary_createWithCapacity);
    tuuvm_jit_moveRegisterToOperand(jit, resultOperand, TUUVM_X86_RAX);

    for(size_t i = 0; i < elementCount; ++i)
    {
        tuuvm_jit_x86_jitLoadContextInRegister(jit, TUUVM_X86_SYSV_ARG0);
        tuuvm_jit_moveOperandToRegister(jit, TUUVM_X86_SYSV_ARG1, resultOperand);
        tuuvm_jit_moveOperandToRegister(jit, TUUVM_X86_SYSV_ARG2, elementOperands[i]);
        tuuvm_jit_x86_call(jit, &tuuvm_dictionary_add);
    }
}

static void tuuvm_jit_makeClosureWithCaptures(tuuvm_bytecodeJit_t *jit, int16_t resultOperand, int16_t functionDefinitionOperand, size_t captureCount, int16_t *elementOperands)
{
    if(resultOperand < 0)
        return;

    // Make the capture vector.
    tuuvm_jit_x86_jitLoadContextInRegister(jit, TUUVM_X86_SYSV_ARG0);
    tuuvm_jit_x86_movImmediate32(jit, TUUVM_X86_SYSV_ARG1, captureCount);
    tuuvm_jit_x86_call(jit, &tuuvm_array_create);

    tuuvm_jit_x86_mov64Register(jit, TUUVM_X86_SYSV_ARG2, TUUVM_X86_RAX);
    for(size_t i = 0; i < captureCount; ++i)
    {
        tuuvm_jit_moveOperandToRegister(jit, TUUVM_X86_SYSV_ARG1, elementOperands[i]);
        tuuvm_jit_x86_mov64IntoMemoryWithOffset(jit, TUUVM_X86_SYSV_ARG2, sizeof(tuuvm_tuple_header_t) + i * sizeof(void*), TUUVM_X86_SYSV_ARG1);
    }

    // Now construct the actual closure
    tuuvm_jit_x86_jitLoadContextInRegister(jit, TUUVM_X86_SYSV_ARG0);
    tuuvm_jit_moveOperandToRegister(jit, TUUVM_X86_SYSV_ARG1, functionDefinitionOperand);
    tuuvm_jit_x86_call(jit, &tuuvm_function_createClosureWithCaptureVector);
    tuuvm_jit_moveRegisterToOperand(jit, resultOperand, TUUVM_X86_RAX);
}

static void tuuvm_jit_jumpRelative(tuuvm_bytecodeJit_t *jit, size_t targetPC)
{
    uint8_t instruction[] = {
        0xE9, 0x00, 0x00, 0x00, 0x00,
    };

    size_t relocationOffset = tuuvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction) - 4;
    tuuvm_bytecodeJitPCRelocation_t relocation = {
        .offset = relocationOffset,
        .targetPC = targetPC,
        .addend = -4,
    };
    tuuvm_bytecodeJit_addPCRelocation(jit, relocation);
}

static void tuuvm_jit_x86_cmpRAXWithImmediate32(tuuvm_bytecodeJit_t *jit, int32_t immediate)
{
    uint8_t instruction[] = {
        tuuvm_jit_x86_rex(true, false, false, false),
        0x3D,
        immediate & 0xFF, (immediate >> 8) & 0xFF, (immediate >> 16) & 0xFF, (immediate >> 24) & 0xFF,
    };

    tuuvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void tuuvm_jit_jumpRelativeIfTrue(tuuvm_bytecodeJit_t *jit, int16_t conditionOperand, size_t targetPC)
{
    tuuvm_jit_moveOperandToRegister(jit, TUUVM_X86_RAX, conditionOperand);
    tuuvm_jit_x86_cmpRAXWithImmediate32(jit, TUUVM_TRUE_TUPLE);

    uint8_t instruction[] = {
        // Jeq
        0x0F, 0x84, 0x00, 0x00, 0x00, 0x00,
    };

    size_t relocationOffset = tuuvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction) - 4;
    tuuvm_bytecodeJitPCRelocation_t relocation = {
        .offset = relocationOffset,
        .targetPC = targetPC,
        .addend = -4,
    };
    tuuvm_bytecodeJit_addPCRelocation(jit, relocation);
}

static void tuuvm_jit_jumpRelativeIfFalse(tuuvm_bytecodeJit_t *jit, int16_t conditionOperand, size_t targetPC)
{
    tuuvm_jit_moveOperandToRegister(jit, TUUVM_X86_RAX, conditionOperand);
    tuuvm_jit_x86_cmpRAXWithImmediate32(jit, TUUVM_TRUE_TUPLE);

    uint8_t instruction[] = {
        // Jne
        0x0F, 0x85, 0x00, 0x00, 0x00, 0x00,
    };

    size_t relocationOffset = tuuvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction) - 4;
    tuuvm_bytecodeJitPCRelocation_t relocation = {
        .offset = relocationOffset,
        .targetPC = targetPC,
        .addend = -4,
    };
    tuuvm_bytecodeJit_addPCRelocation(jit, relocation);
}

static void tuuvm_jit_prologue(tuuvm_bytecodeJit_t *jit)
{
    tuuvm_jit_x86_endbr64(jit);
    tuuvm_jit_x86_pushRegister(jit, TUUVM_X86_RBP);
    tuuvm_jit_x86_mov64Register(jit, TUUVM_X86_RBP, TUUVM_X86_RSP);

    //(tuuvm_context_t *context, tuuvm_tuple_t function, size_t argumentCount, tuuvm_tuple_t *arguments)

    size_t expectedStackSize = (1 + jit->localVectorSize) * sizeof(intptr_t) + sizeof(tuuvm_stackFrameBytecodeFunctionJitActivationRecord_t);
    size_t alignedStackSize = (expectedStackSize + 15) & (-16);
    size_t paddingRequired = alignedStackSize - expectedStackSize;

    // Push the context.
    intptr_t stackFrameIndex = 0;
    tuuvm_jit_x86_pushRegister(jit, TUUVM_X86_SYSV_ARG0); // Context
    --stackFrameIndex;

    // Make space for the locals.
    if(jit->localVectorSize > 0)
    {
        for(size_t i = 0; i < jit->localVectorSize; ++i)
        {
            tuuvm_jit_x86_pushImmediate32(jit, 0);
            --stackFrameIndex;
        }
    }

    jit->localVectorOffset = stackFrameIndex * sizeof(tuuvm_tuple_t);
    tuuvm_jit_x86_pushImmediate32(jit, jit->localVectorSize); // LocalSize
    --stackFrameIndex;

    // Push the argument vector.
    tuuvm_jit_x86_pushRegister(jit, TUUVM_X86_SYSV_ARG3); // Arguments
    --stackFrameIndex;
    jit->argumentVectorOffset = stackFrameIndex * sizeof(tuuvm_tuple_t);

    tuuvm_jit_x86_pushRegister(jit, TUUVM_X86_SYSV_ARG2); // Argument Count
    --stackFrameIndex;

    // Push the function.
    tuuvm_jit_x86_pushRegister(jit, TUUVM_X86_SYSV_ARG1); // Function
    --stackFrameIndex;

    // Push the capture vector.
    tuuvm_jit_x86_pushFromMemoryWithOffset(jit, TUUVM_X86_SYSV_ARG1, offsetof(tuuvm_function_t, captureVector));
    --stackFrameIndex;
    jit->captureVectorOffset = stackFrameIndex*sizeof(tuuvm_tuple_t);

    // Push the literal vector.
    tuuvm_jit_x86_mov64Absolute(jit, TUUVM_X86_RAX, (uintptr_t)jit->literalVectorGCRoot); // Pointer to GC root with the literal vector.
    tuuvm_jit_x86_pushFromMemoryWithOffset(jit, TUUVM_X86_RAX, 0);
    --stackFrameIndex;
    jit->literalVectorOffset = stackFrameIndex*sizeof(tuuvm_tuple_t);

    // Push the PC
    tuuvm_jit_x86_pushImmediate32(jit, 0);
    --stackFrameIndex;
    jit->pcOffset = stackFrameIndex*sizeof(tuuvm_tuple_t);

    // Type and the next record pointer.
    tuuvm_jit_x86_pushImmediate32(jit, TUUVM_STACK_FRAME_RECORD_TYPE_BYTECODE_JIT_FUNCTION_ACTIVATION);
    --stackFrameIndex;
    tuuvm_jit_x86_pushImmediate32(jit, 0);
    --stackFrameIndex;
    jit->stackFrameRecordOffset = stackFrameIndex*sizeof(tuuvm_tuple_t);
    
    // Connect with the stack unwinder.
    tuuvm_jit_x86_mov64Register(jit, TUUVM_X86_SYSV_ARG0, TUUVM_X86_RSP);
    tuuvm_jit_x86_subImmediate32(jit, TUUVM_X86_RSP, paddingRequired);

    tuuvm_jit_x86_call(jit, &tuuvm_stackFrame_pushRecord);
}

static void tuuvm_jit_epilogue(tuuvm_bytecodeJit_t *jit)
{
    // Return.
    tuuvm_jit_x86_mov64Register(jit, TUUVM_X86_RSP, TUUVM_X86_RBP);
    tuuvm_jit_x86_popRegister(jit, TUUVM_X86_RBP);
    tuuvm_jit_x86_ret(jit);
}

static void tuuvm_jit_moveRegisterToOperand(tuuvm_bytecodeJit_t *jit, int16_t operand, tuuvm_x86_register_t reg)
{
    tuuvm_operandVectorName_t vectorType = (tuuvm_operandVectorName_t) (operand & TUUVM_OPERAND_VECTOR_BITMASK);
    int16_t vectorIndex = operand >> TUUVM_OPERAND_VECTOR_BITS;
    if(vectorIndex < 0)
        return;

    int32_t vectorOffset = vectorIndex * sizeof(void*);
    switch(vectorType)
    {
    case TUUVM_OPERAND_VECTOR_LOCAL:
        tuuvm_jit_x86_mov64IntoMemoryWithOffset(jit, TUUVM_X86_RBP, jit->localVectorOffset + vectorOffset, reg);
        break;
    default:
        abort();
        break;
    }
}

static void tuuvm_jit_moveOperandToRegister(tuuvm_bytecodeJit_t *jit, tuuvm_x86_register_t reg, int16_t operand)
{
    tuuvm_operandVectorName_t vectorType = (tuuvm_operandVectorName_t) (operand & TUUVM_OPERAND_VECTOR_BITMASK);
    int16_t vectorIndex = operand >> TUUVM_OPERAND_VECTOR_BITS;
    if(vectorIndex < 0)
    {
        tuuvm_jit_x86_xorRegister(jit, reg, reg);
        return;
    }

    int32_t vectorOffset = (int32_t)vectorIndex * sizeof(void*);
    switch(vectorType)
    {
    case TUUVM_OPERAND_VECTOR_ARGUMENTS:
        tuuvm_jit_x86_mov64FromMemoryWithOffset(jit, reg, TUUVM_X86_RBP, jit->argumentVectorOffset);
        tuuvm_jit_x86_mov64FromMemoryWithOffset(jit, reg, reg, vectorOffset);
        break;
    case TUUVM_OPERAND_VECTOR_CAPTURES:
        tuuvm_jit_x86_mov64FromMemoryWithOffset(jit, reg, TUUVM_X86_RBP, jit->captureVectorOffset);
        tuuvm_jit_x86_mov64FromMemoryWithOffset(jit, reg, reg, sizeof(tuuvm_tuple_header_t) + vectorOffset);
        break;
    case TUUVM_OPERAND_VECTOR_LITERAL:
        tuuvm_jit_x86_mov64FromMemoryWithOffset(jit, reg, TUUVM_X86_RBP, jit->literalVectorOffset);
        tuuvm_jit_x86_mov64FromMemoryWithOffset(jit, reg, reg, sizeof(tuuvm_tuple_header_t) + vectorOffset);
        break;
    case TUUVM_OPERAND_VECTOR_LOCAL:
        tuuvm_jit_x86_mov64FromMemoryWithOffset(jit, reg, TUUVM_X86_RBP, jit->localVectorOffset + vectorOffset);
        break;
    }
}

static void tuuvm_jit_pushOperand(tuuvm_bytecodeJit_t *jit, int16_t operand)
{
    tuuvm_operandVectorName_t vectorType = (tuuvm_operandVectorName_t) (operand & TUUVM_OPERAND_VECTOR_BITMASK);
    int16_t vectorIndex = operand >> TUUVM_OPERAND_VECTOR_BITS;
    if(vectorIndex < 0)
    {
        tuuvm_jit_x86_pushImmediate32(jit, 0);
        return;
    }

    int32_t vectorOffset = vectorIndex * sizeof(void*);
    tuuvm_x86_register_t scratchRegister = TUUVM_X86_RAX;
    switch(vectorType)
    {
    case TUUVM_OPERAND_VECTOR_ARGUMENTS:
        tuuvm_jit_x86_mov64FromMemoryWithOffset(jit, scratchRegister, TUUVM_X86_RBP, jit->argumentVectorOffset);
        tuuvm_jit_x86_pushFromMemoryWithOffset(jit, scratchRegister, vectorOffset);
        break;
    case TUUVM_OPERAND_VECTOR_CAPTURES:
        tuuvm_jit_x86_mov64FromMemoryWithOffset(jit, scratchRegister, TUUVM_X86_RBP, jit->captureVectorOffset);
        tuuvm_jit_x86_pushFromMemoryWithOffset(jit, scratchRegister, sizeof(tuuvm_tuple_header_t) + vectorOffset);
        break;
    case TUUVM_OPERAND_VECTOR_LITERAL:
        tuuvm_jit_x86_mov64FromMemoryWithOffset(jit, scratchRegister, TUUVM_X86_RBP, jit->literalVectorOffset);
        tuuvm_jit_x86_pushFromMemoryWithOffset(jit, scratchRegister, sizeof(tuuvm_tuple_header_t) + vectorOffset);
        break;
    case TUUVM_OPERAND_VECTOR_LOCAL:
        tuuvm_jit_x86_pushFromMemoryWithOffset(jit, TUUVM_X86_RBP, jit->localVectorOffset + vectorOffset);
        break;
    }
}

static void tuuvm_jit_moveOperandToOperand(tuuvm_bytecodeJit_t *jit, int16_t destinationOperand, int16_t sourceOperand)
{
    if(destinationOperand < 0)
        return;

    tuuvm_jit_moveOperandToRegister(jit, TUUVM_X86_RAX, sourceOperand);
    tuuvm_jit_moveRegisterToOperand(jit, destinationOperand, TUUVM_X86_RAX);
}

static void tuuvm_jit_return(tuuvm_bytecodeJit_t *jit, int16_t operand)
{
    // Disconnect from the stack unwinder.
    tuuvm_jit_x86_mov64Register(jit, TUUVM_X86_SYSV_ARG0, TUUVM_X86_RSP);
    tuuvm_jit_x86_call(jit, &tuuvm_stackFrame_popRecord);

    tuuvm_jit_moveOperandToRegister(jit, TUUVM_X86_RAX, operand);
    tuuvm_jit_epilogue(jit);
}

static void tuuvm_jit_storePC(tuuvm_bytecodeJit_t *jit, uint16_t pc)
{
    tuuvm_jit_x86_movImmediateI32IntoMemory64WithOffset(jit, TUUVM_X86_RBP, jit->pcOffset, pc);
}

static void tuuvm_jit_finish(tuuvm_bytecodeJit_t *jit)
{
    // Apply the PC target relative relocations.
    for(size_t i = 0; i < jit->pcRelocationsSize; ++i)
    {
        tuuvm_bytecodeJitPCRelocation_t *relocation = jit->pcRelocations + i;
        *((int32_t*)(jit->instructions + relocation->offset)) = (int32_t)(jit->pcDestinations[relocation->targetPC] - (intptr_t)relocation->offset + relocation->addend);
    }
}

static void tuuvm_jit_installIn(tuuvm_bytecodeJit_t *jit, uint8_t *codeZonePointer)
{
    memcpy(codeZonePointer, jit->instructions, jit->instructionsSize);
    size_t constantsOffset = sizeAlignedTo(jit->instructionsSize, 16);
    uint8_t *constantZonePointer = codeZonePointer + constantsOffset;
    memcpy(constantZonePointer, jit->constants, jit->constantsSize);

    for(size_t i = 0; i < jit->relocationsSize; ++i)
    {
        tuuvm_bytecodeJitRelocation_t *relocation = jit->relocations + i;
        uint8_t *relocationTarget = codeZonePointer + relocation->offset;
        intptr_t relocationTargetAddress = (intptr_t)relocationTarget;

        intptr_t relativeValue = (intptr_t)constantZonePointer + relocation->value - relocationTargetAddress + relocation->addend;
        switch(relocation->type)
        {
        case TUUVM_BYTECODE_JIT_RELOCATION_RELATIVE32:
            *((int32_t*)relocationTarget) = (int32_t)relativeValue;
            break;
        case TUUVM_BYTECODE_JIT_RELOCATION_RELATIVE64:
            *((int64_t*)relocationTarget) = (int64_t)relativeValue;
            break;
        }
    }
}
