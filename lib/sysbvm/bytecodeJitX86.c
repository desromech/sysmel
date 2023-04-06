typedef enum sysbvm_x86_register_e
{
#if defined(__x86_64__)
    SYSBVM_X86_RAX = 0,
    SYSBVM_X86_RCX = 1,
    SYSBVM_X86_RDX = 2,
    SYSBVM_X86_RBX = 3,
    SYSBVM_X86_RSP = 4,
    SYSBVM_X86_RBP = 5,
    SYSBVM_X86_RSI = 6,
    SYSBVM_X86_RDI = 7,

    SYSBVM_X86_R8 = 8,
    SYSBVM_X86_R9 = 9,
    SYSBVM_X86_R10 = 10,
    SYSBVM_X86_R11 = 11,
    SYSBVM_X86_R12 = 12,
    SYSBVM_X86_R13 = 13,
    SYSBVM_X86_R14 = 14,
    SYSBVM_X86_R15 = 15,
#endif

    SYSBVM_X86_EAX = 0,
    SYSBVM_X86_ECX = 1,
    SYSBVM_X86_EDX = 2,
    SYSBVM_X86_EBX = 3,
    SYSBVM_X86_ESP = 4,
    SYSBVM_X86_EBP = 5,
    SYSBVM_X86_ESI = 6,
    SYSBVM_X86_EDI = 7,

    SYSBVM_X86_REG_HALF_MASK = 7,

#if defined(__x86_64__)
#ifdef _WIN32
    SYSBVM_X86_WIN64_ARG0 = SYSBVM_X86_RCX,
    SYSBVM_X86_WIN64_ARG1 = SYSBVM_X86_RDX,
    SYSBVM_X86_WIN64_ARG2 = SYSBVM_X86_R8,
    SYSBVM_X86_WIN64_ARG3 = SYSBVM_X86_R9,
#else
    SYSBVM_X86_SYSV_ARG0 = SYSBVM_X86_RDI,
    SYSBVM_X86_SYSV_ARG1 = SYSBVM_X86_RSI,
    SYSBVM_X86_SYSV_ARG2 = SYSBVM_X86_RDX,
    SYSBVM_X86_SYSV_ARG3 = SYSBVM_X86_RCX,
    SYSBVM_X86_SYSV_ARG4 = SYSBVM_X86_R8,
    SYSBVM_X86_SYSV_ARG5 = SYSBVM_X86_R9,
#endif
#endif
} sysbvm_x86_register_t;

static void sysbvm_jit_x86_mov64Absolute(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t destination, uint64_t value);
static void sysbvm_jit_moveRegisterToOperand(sysbvm_bytecodeJit_t *jit, int16_t operand, sysbvm_x86_register_t reg);
static void sysbvm_jit_moveOperandToRegister(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t reg, int16_t operand);
static void sysbvm_jit_moveOperandToOperand(sysbvm_bytecodeJit_t *jit, int16_t destinationOperand, int16_t sourceOperand);
static void sysbvm_jit_pushOperand(sysbvm_bytecodeJit_t *jit, int16_t operand);

static uint8_t sysbvm_jit_x86_modRM(int8_t rm, uint8_t regOpcode, uint8_t mod)
{
    return (rm & SYSBVM_X86_REG_HALF_MASK) | ((regOpcode & SYSBVM_X86_REG_HALF_MASK) << 3) | (mod << 6);
}

static uint8_t sysbvm_jit_x86_modRMRegister(sysbvm_x86_register_t rm, sysbvm_x86_register_t reg)
{
    return sysbvm_jit_x86_modRM(rm, reg, 3);
}

static uint8_t sysbvm_jit_x86_rex(bool W, bool R, bool X, bool B)
{
    return 0x40 | ((W ? 1 : 0) << 3) | ((R ? 1 : 0) << 2) | ((X ? 1 : 0) << 1) | (B ? 1 : 0);
}

#if 0
static void sysbvm_jit_x86_callRelative32(sysbvm_bytecodeJit_t *jit, void *functionPointer)
{
    uint8_t instruction[] = {
        0xE8, 0x00, 0x00, 0x00, 0x00
    };

    size_t relocationOffset = sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction) - 4;
    
    // This direct copy is placed here for debugging purposes.
    int32_t disp = (intptr_t) functionPointer - (intptr_t)(jit->instructions + jit->instructionsSize);
    memcpy(jit->instructions + relocationOffset, &disp, 4);

    sysbvm_bytecodeJitRelocation_t relocation = {
        .offset = relocationOffset,
        .type = SYSBVM_BYTECODE_JIT_RELOCATION_RELATIVE32,
        .value = (intptr_t)functionPointer,
        .addend = -4
    };
    sysbvm_bytecodeJit_addRelocation(jit, relocation);
}

static void sysbvm_jit_x86_callAbsolute64InlineConstant(sysbvm_bytecodeJit_t *jit, void *functionPointer)
{
    sysbvm_jit_x86_mov64Absolute(jit, SYSBVM_X86_RAX, (uint64_t)functionPointer);
    uint8_t instruction[] = {
        0xFF,
        sysbvm_jit_x86_modRMRegister(SYSBVM_X86_RAX, 2),
    };

    sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}
#endif

static void sysbvm_jit_x86_callAbsoluteNon64InlineConstant(sysbvm_bytecodeJit_t *jit, void *functionPointer)
{
    size_t constantOffset = sysbvm_bytecodeJit_addConstantsBytes(jit, sizeof(functionPointer), (uint8_t*)&functionPointer);
    uint8_t instruction[] = {
        0xFF,
        sysbvm_jit_x86_modRM(5, 2, 0),
        0x00, 0x00, 0x00, 0x00,
    };

    size_t relocationOffset = sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction) - 4;

    sysbvm_bytecodeJitRelocation_t relocation = {
        .offset = relocationOffset,
        .type = SYSBVM_BYTECODE_JIT_RELOCATION_RELATIVE32,
        .value = constantOffset,
        .addend = -4
    };
    sysbvm_bytecodeJit_addRelocation(jit, relocation);
}

static void sysbvm_jit_x86_call(sysbvm_bytecodeJit_t *jit, void *functionPointer)
{
    sysbvm_jit_x86_callAbsoluteNon64InlineConstant(jit, functionPointer);
}

static void sysbvm_jit_x86_pushRegister(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t reg)
{
    if(reg > SYSBVM_X86_REG_HALF_MASK)
        sysbvm_bytecodeJit_addByte(jit, sysbvm_jit_x86_rex(false, false, false, true));
    sysbvm_bytecodeJit_addByte(jit, 0x50 + (reg & SYSBVM_X86_REG_HALF_MASK));
}

static void sysbvm_jit_x86_pushImmediate32(sysbvm_bytecodeJit_t *jit, int32_t immediate)
{
    uint8_t instruction[] = {
        0x68,
        immediate & 0xFF, (immediate >> 8) & 0xFF, (immediate >> 16) & 0xFF, (immediate >> 24) & 0xFF,
    };

    sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void sysbvm_jit_x86_pushFromMemoryWithOffset(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t reg, int32_t offset)
{
    uint8_t instruction[] = {
        0xFF,
        sysbvm_jit_x86_modRM(reg, 6, 2),
        offset & 0xFF, (offset >> 8) & 0xFF, (offset >> 16) & 0xFF, (offset >> 24) & 0xFF,
    };

    sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void sysbvm_jit_x86_popRegister(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t reg)
{
    if(reg > SYSBVM_X86_REG_HALF_MASK)
        sysbvm_bytecodeJit_addByte(jit, sysbvm_jit_x86_rex(false, false, false, true));
    sysbvm_bytecodeJit_addByte(jit, 0x58 + (reg & SYSBVM_X86_REG_HALF_MASK));
}

static void sysbvm_jit_x86_endbr64(sysbvm_bytecodeJit_t *jit)
{
    uint8_t instruction[] = {
        0xF3, 0x0F, 0x1E, 0xFA,
    };

    sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void sysbvm_jit_x86_ret(sysbvm_bytecodeJit_t *jit)
{
    sysbvm_bytecodeJit_addByte(jit, 0xc3);
}

static void sysbvm_jit_x86_mov64Register(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t destination, sysbvm_x86_register_t source)
{
    uint8_t instruction[] = {
        sysbvm_jit_x86_rex(true, destination > SYSBVM_X86_REG_HALF_MASK, false, source > SYSBVM_X86_REG_HALF_MASK),
        0x8B,
        sysbvm_jit_x86_modRMRegister(source, destination),
    };

    sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void sysbvm_jit_x86_mov64Absolute(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t destination, uint64_t value)
{
    uint8_t instruction[] = {
        sysbvm_jit_x86_rex(true, false, false, destination > SYSBVM_X86_REG_HALF_MASK),
        0xB8 + (destination & SYSBVM_X86_REG_HALF_MASK),
        value & 0xFF, (value >> 8) & 0xFF, (value >> 16) & 0xFF, (value >> 24) & 0xFF,
        (value >> 32) & 0xFF, (value >> 40) & 0xFF, (value >> 48) & 0xFF, (value >> 56) & 0xFF,
    };

    sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void sysbvm_jit_x86_addImmediate32(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t destination, int32_t value)
{
    if(value == 0)
        return;

    uint8_t instruction[] = {
        sysbvm_jit_x86_rex(true, false, false, destination > SYSBVM_X86_REG_HALF_MASK),
        0x81,
        sysbvm_jit_x86_modRMRegister(destination, 0),
        value & 0xFF, (value >> 8) & 0xFF, (value >> 16) & 0xFF, (value >> 24) & 0xFF,
    };

    sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void sysbvm_jit_x86_subImmediate32(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t destination, int32_t value)
{
    if(value == 0)
        return;

    uint8_t instruction[] = {
        sysbvm_jit_x86_rex(true, false, false, destination > SYSBVM_X86_REG_HALF_MASK),
        0x81,
        sysbvm_jit_x86_modRMRegister(destination, 5),
        value & 0xFF, (value >> 8) & 0xFF, (value >> 16) & 0xFF, (value >> 24) & 0xFF,
    };

    sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void sysbvm_jit_x86_movImmediate32(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t destination, int32_t value)
{
    uint8_t instruction[] = {
        sysbvm_jit_x86_rex(true, false, false, destination > SYSBVM_X86_REG_HALF_MASK),
        0xC7,
        sysbvm_jit_x86_modRMRegister(destination, 0),
        value & 0xFF, (value >> 8) & 0xFF, (value >> 16) & 0xFF, (value >> 24) & 0xFF,
    };

    sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void sysbvm_jit_x86_leaRegisterWithOffset(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t destination, sysbvm_x86_register_t source, int32_t offset)
{
    uint8_t instruction[] = {
        sysbvm_jit_x86_rex(true, source > SYSBVM_X86_REG_HALF_MASK, false, destination > SYSBVM_X86_REG_HALF_MASK),
        0x8D,
        sysbvm_jit_x86_modRM(source, destination, 2),
        offset & 0xFF, (offset >> 8) & 0xFF, (offset >> 16) & 0xFF, (offset >> 24) & 0xFF,
    };

    sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void sysbvm_jit_x86_mov64FromMemoryWithOffset(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t destination, sysbvm_x86_register_t source, int32_t offset)
{
    if(offset == 0 && source != SYSBVM_X86_RBP)
    {
        uint8_t instruction[] = {
            sysbvm_jit_x86_rex(true, source > SYSBVM_X86_REG_HALF_MASK, false, destination > SYSBVM_X86_REG_HALF_MASK),
            0x8B,
            sysbvm_jit_x86_modRM(source, destination, 0),
        };

        sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
    }
    else
    {
        uint8_t instruction[] = {
            sysbvm_jit_x86_rex(true, source > SYSBVM_X86_REG_HALF_MASK, false, destination > SYSBVM_X86_REG_HALF_MASK),
            0x8B,
            sysbvm_jit_x86_modRM(source, destination, 2),
            offset & 0xFF, (offset >> 8) & 0xFF, (offset >> 16) & 0xFF, (offset >> 24) & 0xFF,
        };

        sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
    }
}

static void sysbvm_jit_x86_mov64IntoMemoryWithOffset(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t destination, int32_t offset, sysbvm_x86_register_t source)
{
    if(offset == 0 && destination != SYSBVM_X86_RBP)
    {
        uint8_t instruction[] = {
            sysbvm_jit_x86_rex(true, destination > SYSBVM_X86_REG_HALF_MASK, false, source > SYSBVM_X86_REG_HALF_MASK),
            0x89,
            sysbvm_jit_x86_modRM(destination, source, 0),
        };

        sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
    }
    else
    {
        uint8_t instruction[] = {
            sysbvm_jit_x86_rex(true, destination > SYSBVM_X86_REG_HALF_MASK, false, source > SYSBVM_X86_REG_HALF_MASK),
            0x89,
            sysbvm_jit_x86_modRM(destination, source, 2),
            offset & 0xFF, (offset >> 8) & 0xFF, (offset >> 16) & 0xFF, (offset >> 24) & 0xFF,
        };

        sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
    }
}


static void sysbvm_jit_x86_logicalShiftRightImmediate(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t destination, uint8_t shiftAmount)
{
    uint8_t instruction[] = {
        sysbvm_jit_x86_rex(true, destination > SYSBVM_X86_REG_HALF_MASK, false, false),
        0xC1,
        sysbvm_jit_x86_modRMRegister(destination, 5),
        shiftAmount
    };

    sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void sysbvm_jit_x86_mov8IntoMemoryWithOffset(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t destination, int32_t offset, sysbvm_x86_register_t source)
{
    if(offset == 0 && destination != SYSBVM_X86_RBP)
    {
        uint8_t instruction[] = {
            sysbvm_jit_x86_rex(false, destination > SYSBVM_X86_REG_HALF_MASK, false, source > SYSBVM_X86_REG_HALF_MASK),
            0x88,
            sysbvm_jit_x86_modRM(destination, source, 0),
        };

        sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
    }
    else
    {
        uint8_t instruction[] = {
            sysbvm_jit_x86_rex(false, destination > SYSBVM_X86_REG_HALF_MASK, false, source > SYSBVM_X86_REG_HALF_MASK),
            0x88,
            sysbvm_jit_x86_modRM(destination, source, 2),
            offset & 0xFF, (offset >> 8) & 0xFF, (offset >> 16) & 0xFF, (offset >> 24) & 0xFF,
        };

        sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
    }
}

static void sysbvm_jit_x86_movImmediateI32IntoMemory64WithOffset(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t destination, int32_t offset, int16_t immediate)
{
    uint8_t instruction[] = {
        sysbvm_jit_x86_rex(true, destination > SYSBVM_X86_REG_HALF_MASK, false, false),
        0xC7,
        sysbvm_jit_x86_modRM(destination, 0, 2),
        offset & 0xFF, (offset >> 8) & 0xFF, (offset >> 16) & 0xFF, (offset >> 24) & 0xFF,
        immediate & 0xFF, (immediate >> 8) & 0xFF, (immediate >> 16) & 0xFF, (immediate >> 24) & 0xFF,
    };

    sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}


static void sysbvm_jit_x86_xorRegister(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t destination, sysbvm_x86_register_t source)
{
    uint8_t instruction[] = {
        sysbvm_jit_x86_rex(true, destination > SYSBVM_X86_REG_HALF_MASK, false, destination > SYSBVM_X86_REG_HALF_MASK),
        0x33,
        sysbvm_jit_x86_modRMRegister(source, destination),
    };

    sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void sysbvm_jit_x86_jitLoadContextInRegister(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t reg)
{
    sysbvm_jit_x86_mov64FromMemoryWithOffset(jit, reg, SYSBVM_X86_RBP, -(int32_t)sizeof(void*));
}

static void sysbvm_jit_callNoResult0(sysbvm_bytecodeJit_t *jit, void *functionPointer)
{
    sysbvm_jit_x86_call(jit, functionPointer);
}

static void sysbvm_jit_callWithContextNoResult0(sysbvm_bytecodeJit_t *jit, void *functionPointer)
{
    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_SYSV_ARG0);
    sysbvm_jit_x86_call(jit, functionPointer);
}

static void sysbvm_jit_callWithContext1(sysbvm_bytecodeJit_t *jit, void *functionPointer, int16_t resultOperand, int16_t argumentOperand0)
{
    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_SYSV_ARG0);
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_SYSV_ARG1, argumentOperand0);
    sysbvm_jit_x86_call(jit, functionPointer);
    sysbvm_jit_moveRegisterToOperand(jit, resultOperand, SYSBVM_X86_RAX);
}

static void sysbvm_jit_callWithContext2(sysbvm_bytecodeJit_t *jit, void *functionPointer, int16_t resultOperand, int16_t argumentOperand0, int16_t argumentOperand1)
{
    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_SYSV_ARG0);
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_SYSV_ARG1, argumentOperand0);
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_SYSV_ARG2, argumentOperand1);
    sysbvm_jit_x86_call(jit, functionPointer);
    sysbvm_jit_moveRegisterToOperand(jit, resultOperand, SYSBVM_X86_RAX);
}

static void sysbvm_jit_callWithContextNoResult2(sysbvm_bytecodeJit_t *jit, void *functionPointer, int16_t argumentOperand0, int16_t argumentOperand1)
{
    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_SYSV_ARG0);
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_SYSV_ARG1, argumentOperand0);
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_SYSV_ARG2, argumentOperand1);
    sysbvm_jit_x86_call(jit, functionPointer);
}

static void sysbvm_jit_callWithContextNoResult3(sysbvm_bytecodeJit_t *jit, void *functionPointer, int16_t argumentOperand0, int16_t argumentOperand1, int16_t argumentOperand2)
{
    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_SYSV_ARG0);
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_SYSV_ARG1, argumentOperand0);
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_SYSV_ARG2, argumentOperand1);
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_SYSV_ARG3, argumentOperand2);
    sysbvm_jit_x86_call(jit, functionPointer);
}

static void sysbvm_jit_functionApplyVia(sysbvm_bytecodeJit_t *jit, int16_t resultOperand, int16_t functionOperand, size_t argumentCount, int16_t *argumentOperands, int applicationFlags, void *calledFunctionPointer)
{
    size_t stackSize = argumentCount * sizeof(void*);
    size_t alignedStackSize = (stackSize + 15) & (-16);
    size_t paddingSize = alignedStackSize - stackSize;
    sysbvm_jit_x86_subImmediate32(jit, SYSBVM_X86_RSP, paddingSize);

    // Push all of the arguments in the stack.
    for(size_t i = 0; i < argumentCount; ++i)
        sysbvm_jit_pushOperand(jit, argumentOperands[argumentCount - i - 1]);

    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_SYSV_ARG0);
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_SYSV_ARG1, functionOperand);
    sysbvm_jit_x86_movImmediate32(jit, SYSBVM_X86_SYSV_ARG2, argumentCount);
    sysbvm_jit_x86_mov64Register(jit, SYSBVM_X86_SYSV_ARG3, SYSBVM_X86_RSP);
    sysbvm_jit_x86_movImmediate32(jit, SYSBVM_X86_SYSV_ARG4, applicationFlags);
    sysbvm_jit_x86_call(jit, calledFunctionPointer);
    sysbvm_jit_x86_addImmediate32(jit, SYSBVM_X86_RSP, alignedStackSize);

    sysbvm_jit_moveRegisterToOperand(jit, resultOperand, SYSBVM_X86_RAX);
}

static void sysbvm_jit_functionApplyDirectVia(sysbvm_bytecodeJit_t *jit, int16_t resultOperand, int16_t functionOperand, size_t argumentCount, int16_t *argumentOperands, void *calledFunctionPointer)
{
    size_t stackSize = argumentCount * sizeof(void*);
    size_t alignedStackSize = (stackSize + 15) & (-16);
    size_t paddingSize = alignedStackSize - stackSize;
    sysbvm_jit_x86_subImmediate32(jit, SYSBVM_X86_RSP, paddingSize);

    // Push all of the arguments in the stack.
    for(size_t i = 0; i < argumentCount; ++i)
        sysbvm_jit_pushOperand(jit, argumentOperands[argumentCount - i - 1]);

    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_SYSV_ARG0);
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_SYSV_ARG1, functionOperand);
    sysbvm_jit_x86_movImmediate32(jit, SYSBVM_X86_SYSV_ARG2, argumentCount);
    sysbvm_jit_x86_mov64Register(jit, SYSBVM_X86_SYSV_ARG3, SYSBVM_X86_RSP);
    sysbvm_jit_x86_call(jit, calledFunctionPointer);
    sysbvm_jit_x86_addImmediate32(jit, SYSBVM_X86_RSP, alignedStackSize);

    sysbvm_jit_moveRegisterToOperand(jit, resultOperand, SYSBVM_X86_RAX);
}

static void sysbvm_jit_functionApplyDirectWithArgumentsRecord(sysbvm_bytecodeJit_t *jit, int16_t resultOperand, int16_t functionOperand, size_t argumentCount, int16_t *argumentOperands, void *calledFunctionPointer)
{
    size_t stackSize = argumentCount * sizeof(void*) + sizeof(sysbvm_stackFrameGCRootsRecord_t);
    size_t alignedStackSize = (stackSize + 15) & (-16);
    size_t paddingSize = alignedStackSize - stackSize;
    sysbvm_jit_x86_subImmediate32(jit, SYSBVM_X86_RSP, paddingSize);

    // Push all of the arguments in the stack.
    for(size_t i = 0; i < argumentCount; ++i)
        sysbvm_jit_pushOperand(jit, argumentOperands[argumentCount - i - 1]);

    // Arguments gc frame record.
    sysbvm_jit_x86_pushRegister(jit, SYSBVM_X86_RSP);
    sysbvm_jit_x86_pushImmediate32(jit, argumentCount);
    sysbvm_jit_x86_pushImmediate32(jit, SYSBVM_STACK_FRAME_RECORD_TYPE_GC_ROOTS);
    sysbvm_jit_x86_pushImmediate32(jit, 0);

    sysbvm_jit_x86_mov64Register(jit, SYSBVM_X86_SYSV_ARG0, SYSBVM_X86_RSP);
    sysbvm_jit_x86_call(jit, &sysbvm_stackFrame_pushRecord);

    // Call the primitive.
    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_SYSV_ARG0);
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_SYSV_ARG1, functionOperand);
    sysbvm_jit_x86_movImmediate32(jit, SYSBVM_X86_SYSV_ARG2, argumentCount);
    sysbvm_jit_x86_mov64Register(jit, SYSBVM_X86_SYSV_ARG3, SYSBVM_X86_RSP);
    sysbvm_jit_x86_addImmediate32(jit, SYSBVM_X86_SYSV_ARG3, sizeof(sysbvm_stackFrameGCRootsRecord_t));
    sysbvm_jit_x86_call(jit, calledFunctionPointer);

    sysbvm_jit_moveRegisterToOperand(jit, resultOperand, SYSBVM_X86_RAX);

    // Pop the arguments GC record.
    sysbvm_jit_x86_mov64Register(jit, SYSBVM_X86_SYSV_ARG0, SYSBVM_X86_RSP);
    sysbvm_jit_x86_call(jit, &sysbvm_stackFrame_popRecord);

    sysbvm_jit_x86_addImmediate32(jit, SYSBVM_X86_RSP, alignedStackSize);
}

static void *sysbvm_jit_getTrampolineOrEntryPointForBytecode(sysbvm_bytecodeJit_t *jit, sysbvm_functionBytecode_t *bytecode)
{
    // Attempt direct entry first.
    if(bytecode->jittedCode && bytecode->jittedCodeSessionToken == jit->context->roots.sessionToken)
        return (void*)sysbvm_tuple_systemHandle_decode(bytecode->jittedCode);

    if(bytecode->jittedCodeTrampoline && bytecode->jittedCodeTrampolineSessionToken == jit->context->roots.sessionToken)
        return (void*)sysbvm_tuple_systemHandle_decode(bytecode->jittedCodeTrampoline);

    uint64_t trampolineTargetAddress = (uint64_t)(uintptr_t)&sysbvm_bytecodeInterpreter_applyJitTrampolineDestination;

    uint8_t trampolineCode[] = {
        // Endbr64
        0xF3, 0x0F, 0x1E, 0xFA,

        // Mov64
        sysbvm_jit_x86_rex(true, false, false, false),
        0xB8 + SYSBVM_X86_RAX,
        trampolineTargetAddress & 0xFF, (trampolineTargetAddress >> 8) & 0xFF, (trampolineTargetAddress >> 16) & 0xFF, (trampolineTargetAddress >> 24) & 0xFF,
        (trampolineTargetAddress >> 32) & 0xFF, (trampolineTargetAddress >> 40) & 0xFF, (trampolineTargetAddress >> 48) & 0xFF, (trampolineTargetAddress >> 56) & 0xFF,

        // Jmp RAX
        0xFF,
        sysbvm_jit_x86_modRMRegister(SYSBVM_X86_RAX, 4),
    };

    // Install the trampoline in the code zone.
    size_t trampolineCodeSize = sizeof(trampolineCode);
    size_t requiredCodeSize = sizeAlignedTo(trampolineCodeSize, 16);
    uint8_t *codeZonePointer = sysbvm_heap_allocateAndLockCodeZone(&jit->context->heap, requiredCodeSize, 16);
    memset(codeZonePointer, 0xcc, requiredCodeSize); // int3;
    memcpy(codeZonePointer, trampolineCode, trampolineCodeSize);
    sysbvm_heap_unlockCodeZone(&jit->context->heap, codeZonePointer, requiredCodeSize);

    bytecode->jittedCodeTrampoline = sysbvm_tuple_systemHandle_encode(jit->context, (sysbvm_systemHandle_t)(uintptr_t)codeZonePointer);
    bytecode->jittedCodeTrampolineSessionToken = jit->context->roots.sessionToken;

    return codeZonePointer;
}

static void sysbvm_jit_patchTrampolineWithRealEntryPoint(sysbvm_bytecodeJit_t *jit, sysbvm_functionBytecode_t *bytecode)
{
    if(bytecode->jittedCodeTrampoline && bytecode->jittedCodeTrampolineSessionToken == jit->context->roots.sessionToken)
    {
        uint8_t *realEntryPoint = (uint8_t*)sysbvm_tuple_systemHandle_decode(bytecode->jittedCode);
        uint8_t *trampolineEntryPoint = (uint8_t*)sysbvm_tuple_systemHandle_decode(bytecode->jittedCodeTrampoline);

        size_t trampolineCodeSize = 16;
        size_t targetAddressOffset = trampolineCodeSize - 2 - sizeof(void*);
        sysbvm_heap_lockCodeZone(&jit->context->heap, trampolineEntryPoint, trampolineCodeSize);
        memcpy(trampolineEntryPoint +targetAddressOffset, &realEntryPoint, sizeof(realEntryPoint));
        sysbvm_heap_unlockCodeZone(&jit->context->heap, trampolineEntryPoint, trampolineCodeSize);
    }
}

static void sysbvm_jit_functionApply(sysbvm_bytecodeJit_t *jit, int16_t resultOperand, int16_t functionOperand, size_t argumentCount, int16_t *argumentOperands, int applicationFlags)
{
    bool isNoTypecheck = applicationFlags & SYSBVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK;

    // Are we calling a literal function?
    sysbvm_tuple_t literalFunction = SYSBVM_NULL_TUPLE;

    if(isNoTypecheck
        && sysbvm_bytecodeJit_getLiteralValueForOperand(jit, functionOperand, &literalFunction)
        && sysbvm_tuple_isFunction(jit->context, literalFunction)
        && !sysbvm_function_isMemoized(jit->context, literalFunction)
        && !sysbvm_function_isVariadic(jit->context, literalFunction))
    {
        // Is this a numbered primitive.
        sysbvm_function_t *literalFunctionObject = (sysbvm_function_t*)literalFunction;
        if(literalFunctionObject->primitiveTableIndex)
        {
            uint32_t primitiveNumber = sysbvm_tuple_uint32_decode(literalFunctionObject->primitiveTableIndex);
            sysbvm_functionEntryPoint_t entryPoint = sysbvm_function_getNumberedPrimitiveEntryPoint(jit->context, primitiveNumber);
            if(entryPoint)
            {
                sysbvm_jit_functionApplyDirectWithArgumentsRecord(jit, resultOperand, functionOperand, argumentCount, argumentOperands, entryPoint);
                return;
            }
        }
        
        if(literalFunctionObject->definition)
        {
            sysbvm_functionDefinition_t *literalFunctionDefinitionObject = (sysbvm_functionDefinition_t*)literalFunctionObject->definition;
            if(literalFunctionDefinitionObject->bytecode && literalFunctionDefinitionObject->bytecode != SYSBVM_PENDING_MEMOIZATION_VALUE)
            {
                void *trampolineOrEntryPoint = sysbvm_jit_getTrampolineOrEntryPointForBytecode(jit, (sysbvm_functionBytecode_t*)literalFunctionDefinitionObject->bytecode);
                if(trampolineOrEntryPoint)
                {
                    //printf("trampolineOrEntryPoint %p\n", trampolineOrEntryPoint);
                    sysbvm_jit_functionApplyDirectVia(jit, resultOperand, functionOperand, argumentCount, argumentOperands, trampolineOrEntryPoint);
                    return;
                }
            }
        }
    }

    sysbvm_jit_functionApplyVia(jit, resultOperand, functionOperand, argumentCount, argumentOperands, applicationFlags, &sysbvm_bytecodeInterpreter_functionApplyNoCopyArguments);
}

static void sysbvm_jit_send(sysbvm_bytecodeJit_t *jit, int16_t resultOperand, int16_t selectorOperand, size_t argumentCount, int16_t *argumentOperands, int applicationFlags)
{
    size_t stackSize = (argumentCount + 1) * sizeof(void*);
    size_t alignedStackSize = (stackSize + 15) & (-16);
    size_t paddingSize = alignedStackSize - stackSize;
    sysbvm_jit_x86_subImmediate32(jit, SYSBVM_X86_RSP, paddingSize);

    // Push all of the arguments in the stack.
    size_t pushCount = argumentCount + 1;
    for(size_t i = 0; i < pushCount; ++i)
        sysbvm_jit_pushOperand(jit, argumentOperands[pushCount - i - 1]);

    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_SYSV_ARG0);
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_SYSV_ARG1, selectorOperand);
    sysbvm_jit_x86_movImmediate32(jit, SYSBVM_X86_SYSV_ARG2, argumentCount);
    sysbvm_jit_x86_mov64Register(jit, SYSBVM_X86_SYSV_ARG3, SYSBVM_X86_RSP);
    sysbvm_jit_x86_movImmediate32(jit, SYSBVM_X86_SYSV_ARG4, applicationFlags);
    sysbvm_jit_x86_call(jit, &sysbvm_bytecodeInterpreter_interpretSendNoCopyArguments);
    sysbvm_jit_x86_addImmediate32(jit, SYSBVM_X86_RSP, alignedStackSize);

    sysbvm_jit_moveRegisterToOperand(jit, resultOperand, SYSBVM_X86_RAX);
}

static void sysbvm_jit_sendWithReceiverType(sysbvm_bytecodeJit_t *jit, int16_t resultOperand, int16_t receiverTypeOperand, int16_t selectorOperand, size_t argumentCount, int16_t *argumentOperands, int applicationFlags)
{
    size_t stackSize = (argumentCount + 1) * sizeof(void*);
    size_t alignedStackSize = (stackSize + 15) & (-16);
    size_t paddingSize = alignedStackSize - stackSize;
    sysbvm_jit_x86_subImmediate32(jit, SYSBVM_X86_RSP, paddingSize);

    // Push all of the arguments in the stack.
    size_t pushCount = argumentCount + 1;
    for(size_t i = 0; i < pushCount; ++i)
        sysbvm_jit_pushOperand(jit, argumentOperands[pushCount - i - 1]);

    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_SYSV_ARG0);
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_SYSV_ARG1, receiverTypeOperand);
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_SYSV_ARG2, selectorOperand);
    sysbvm_jit_x86_movImmediate32(jit, SYSBVM_X86_SYSV_ARG3, argumentCount);
    sysbvm_jit_x86_mov64Register(jit, SYSBVM_X86_SYSV_ARG4, SYSBVM_X86_RSP);
    sysbvm_jit_x86_movImmediate32(jit, SYSBVM_X86_SYSV_ARG5, applicationFlags);
    sysbvm_jit_x86_call(jit, &sysbvm_bytecodeInterpreter_interpretSendWithReceiverTypeNoCopyArguments);
    sysbvm_jit_x86_addImmediate32(jit, SYSBVM_X86_RSP, alignedStackSize);

    sysbvm_jit_moveRegisterToOperand(jit, resultOperand, SYSBVM_X86_RAX);
}

static void sysbvm_jit_makeArray(sysbvm_bytecodeJit_t *jit, int16_t resultOperand, size_t elementCount, int16_t *elementOperands)
{
    if(resultOperand < 0)
        return;

    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_SYSV_ARG0);
    sysbvm_jit_x86_movImmediate32(jit, SYSBVM_X86_SYSV_ARG1, elementCount);
    sysbvm_jit_x86_call(jit, &sysbvm_array_create);

    for(size_t i = 0; i < elementCount; ++i)
    {
        sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_RSI, elementOperands[i]);
        sysbvm_jit_x86_mov64IntoMemoryWithOffset(jit, SYSBVM_X86_RAX, sizeof(sysbvm_tuple_header_t) + i * sizeof(void*), SYSBVM_X86_RSI);
    }

    sysbvm_jit_moveRegisterToOperand(jit, resultOperand, SYSBVM_X86_RAX);
}

static void sysbvm_jit_makeByteArray(sysbvm_bytecodeJit_t *jit, int16_t resultOperand, size_t elementCount, int16_t *elementOperands)
{
    if(resultOperand < 0)
        return;

    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_SYSV_ARG0);
    sysbvm_jit_x86_movImmediate32(jit, SYSBVM_X86_SYSV_ARG1, elementCount);
    sysbvm_jit_x86_call(jit, &sysbvm_byteArray_create);

    for(size_t i = 0; i < elementCount; ++i)
    {
        sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_RSI, elementOperands[i]);
        sysbvm_jit_x86_logicalShiftRightImmediate(jit, SYSBVM_X86_RSI, SYSBVM_TUPLE_TAG_BIT_COUNT);
        sysbvm_jit_x86_mov8IntoMemoryWithOffset(jit, SYSBVM_X86_RAX, sizeof(sysbvm_tuple_header_t) + i, SYSBVM_X86_RSI);
    }

    sysbvm_jit_moveRegisterToOperand(jit, resultOperand, SYSBVM_X86_RAX);
}

static void sysbvm_jit_makeDictionary(sysbvm_bytecodeJit_t *jit, int16_t resultOperand, size_t elementCount, int16_t *elementOperands)
{
    if(resultOperand < 0)
        return;

    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_SYSV_ARG0);
    sysbvm_jit_x86_movImmediate32(jit, SYSBVM_X86_SYSV_ARG1, elementCount);
    sysbvm_jit_x86_call(jit, &sysbvm_dictionary_createWithCapacity);
    sysbvm_jit_moveRegisterToOperand(jit, resultOperand, SYSBVM_X86_RAX);

    for(size_t i = 0; i < elementCount; ++i)
    {
        sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_SYSV_ARG0);
        sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_SYSV_ARG1, resultOperand);
        sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_SYSV_ARG2, elementOperands[i]);
        sysbvm_jit_x86_call(jit, &sysbvm_dictionary_add);
    }
}

static void sysbvm_jit_makeClosureWithCaptures(sysbvm_bytecodeJit_t *jit, int16_t resultOperand, int16_t functionDefinitionOperand, size_t captureCount, int16_t *elementOperands)
{
    if(resultOperand < 0)
        return;

    // Make the capture vector.
    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_SYSV_ARG0);
    sysbvm_jit_x86_movImmediate32(jit, SYSBVM_X86_SYSV_ARG1, captureCount);
    sysbvm_jit_x86_call(jit, &sysbvm_array_create);

    sysbvm_jit_x86_mov64Register(jit, SYSBVM_X86_SYSV_ARG2, SYSBVM_X86_RAX);
    for(size_t i = 0; i < captureCount; ++i)
    {
        sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_SYSV_ARG1, elementOperands[i]);
        sysbvm_jit_x86_mov64IntoMemoryWithOffset(jit, SYSBVM_X86_SYSV_ARG2, sizeof(sysbvm_tuple_header_t) + i * sizeof(void*), SYSBVM_X86_SYSV_ARG1);
    }

    // Now construct the actual closure
    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_SYSV_ARG0);
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_SYSV_ARG1, functionDefinitionOperand);
    sysbvm_jit_x86_call(jit, &sysbvm_function_createClosureWithCaptureVector);
    sysbvm_jit_moveRegisterToOperand(jit, resultOperand, SYSBVM_X86_RAX);
}

static void sysbvm_jit_jumpRelative(sysbvm_bytecodeJit_t *jit, size_t targetPC)
{
    uint8_t instruction[] = {
        0xE9, 0x00, 0x00, 0x00, 0x00,
    };

    size_t relocationOffset = sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction) - 4;
    sysbvm_bytecodeJitPCRelocation_t relocation = {
        .offset = relocationOffset,
        .targetPC = targetPC,
        .addend = -4,
    };
    sysbvm_bytecodeJit_addPCRelocation(jit, relocation);
}

static void sysbvm_jit_x86_cmpRAXWithImmediate32(sysbvm_bytecodeJit_t *jit, int32_t immediate)
{
    uint8_t instruction[] = {
        sysbvm_jit_x86_rex(true, false, false, false),
        0x3D,
        immediate & 0xFF, (immediate >> 8) & 0xFF, (immediate >> 16) & 0xFF, (immediate >> 24) & 0xFF,
    };

    sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void sysbvm_jit_jumpRelativeIfTrue(sysbvm_bytecodeJit_t *jit, int16_t conditionOperand, size_t targetPC)
{
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_RAX, conditionOperand);
    sysbvm_jit_x86_cmpRAXWithImmediate32(jit, SYSBVM_TRUE_TUPLE);

    uint8_t instruction[] = {
        // Jeq
        0x0F, 0x84, 0x00, 0x00, 0x00, 0x00,
    };

    size_t relocationOffset = sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction) - 4;
    sysbvm_bytecodeJitPCRelocation_t relocation = {
        .offset = relocationOffset,
        .targetPC = targetPC,
        .addend = -4,
    };
    sysbvm_bytecodeJit_addPCRelocation(jit, relocation);
}

static void sysbvm_jit_jumpRelativeIfFalse(sysbvm_bytecodeJit_t *jit, int16_t conditionOperand, size_t targetPC)
{
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_RAX, conditionOperand);
    sysbvm_jit_x86_cmpRAXWithImmediate32(jit, SYSBVM_TRUE_TUPLE);

    uint8_t instruction[] = {
        // Jne
        0x0F, 0x85, 0x00, 0x00, 0x00, 0x00,
    };

    size_t relocationOffset = sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction) - 4;
    sysbvm_bytecodeJitPCRelocation_t relocation = {
        .offset = relocationOffset,
        .targetPC = targetPC,
        .addend = -4,
    };
    sysbvm_bytecodeJit_addPCRelocation(jit, relocation);
}

static void sysbvm_jit_prologue(sysbvm_bytecodeJit_t *jit)
{
    sysbvm_jit_x86_endbr64(jit);
    sysbvm_jit_x86_pushRegister(jit, SYSBVM_X86_RBP);
    sysbvm_jit_x86_mov64Register(jit, SYSBVM_X86_RBP, SYSBVM_X86_RSP);

    //(sysbvm_context_t *context, sysbvm_tuple_t function, size_t argumentCount, sysbvm_tuple_t *arguments)

    size_t expectedStackSize = (1 + jit->localVectorSize) * sizeof(intptr_t) + sizeof(sysbvm_stackFrameBytecodeFunctionJitActivationRecord_t);
    size_t alignedStackSize = (expectedStackSize + 15) & (-16);
    size_t paddingRequired = alignedStackSize - expectedStackSize;

    // Push the context.
    intptr_t stackFrameIndex = 0;
    sysbvm_jit_x86_pushRegister(jit, SYSBVM_X86_SYSV_ARG0); // Context
    --stackFrameIndex;

    // Make space for the locals.
    if(jit->localVectorSize > 0)
    {
        for(size_t i = 0; i < jit->localVectorSize; ++i)
        {
            sysbvm_jit_x86_pushImmediate32(jit, 0);
            --stackFrameIndex;
        }
    }

    jit->localVectorOffset = stackFrameIndex * sizeof(sysbvm_tuple_t);
    sysbvm_jit_x86_pushImmediate32(jit, jit->localVectorSize); // LocalSize
    --stackFrameIndex;

    // Push the argument vector.
    sysbvm_jit_x86_pushRegister(jit, SYSBVM_X86_SYSV_ARG3); // Arguments
    --stackFrameIndex;
    jit->argumentVectorOffset = stackFrameIndex * sizeof(sysbvm_tuple_t);

    sysbvm_jit_x86_pushRegister(jit, SYSBVM_X86_SYSV_ARG2); // Argument Count
    --stackFrameIndex;

    // Push the function.
    sysbvm_jit_x86_pushRegister(jit, SYSBVM_X86_SYSV_ARG1); // Function
    --stackFrameIndex;

    // Push the capture vector.
    sysbvm_jit_x86_pushFromMemoryWithOffset(jit, SYSBVM_X86_SYSV_ARG1, offsetof(sysbvm_function_t, captureVector));
    --stackFrameIndex;
    jit->captureVectorOffset = stackFrameIndex*sizeof(sysbvm_tuple_t);

    // Push the literal vector.
    sysbvm_jit_x86_mov64Absolute(jit, SYSBVM_X86_RAX, (uintptr_t)jit->literalVectorGCRoot); // Pointer to GC root with the literal vector.
    sysbvm_jit_x86_pushFromMemoryWithOffset(jit, SYSBVM_X86_RAX, 0);
    --stackFrameIndex;
    jit->literalVectorOffset = stackFrameIndex*sizeof(sysbvm_tuple_t);

    // Push the PC
    sysbvm_jit_x86_pushImmediate32(jit, 0);
    --stackFrameIndex;
    jit->pcOffset = stackFrameIndex*sizeof(sysbvm_tuple_t);

    // Type and the next record pointer.
    sysbvm_jit_x86_pushImmediate32(jit, SYSBVM_STACK_FRAME_RECORD_TYPE_BYTECODE_JIT_FUNCTION_ACTIVATION);
    --stackFrameIndex;
    sysbvm_jit_x86_pushImmediate32(jit, 0);
    --stackFrameIndex;
    jit->stackFrameRecordOffset = stackFrameIndex*sizeof(sysbvm_tuple_t);
    
    // Connect with the stack unwinder.
    sysbvm_jit_x86_mov64Register(jit, SYSBVM_X86_SYSV_ARG0, SYSBVM_X86_RSP);
    sysbvm_jit_x86_subImmediate32(jit, SYSBVM_X86_RSP, paddingRequired);

    sysbvm_jit_x86_call(jit, &sysbvm_stackFrame_pushRecord);
}

static void sysbvm_jit_epilogue(sysbvm_bytecodeJit_t *jit)
{
    // Return.
    sysbvm_jit_x86_mov64Register(jit, SYSBVM_X86_RSP, SYSBVM_X86_RBP);
    sysbvm_jit_x86_popRegister(jit, SYSBVM_X86_RBP);
    sysbvm_jit_x86_ret(jit);
}

static void sysbvm_jit_moveRegisterToOperand(sysbvm_bytecodeJit_t *jit, int16_t operand, sysbvm_x86_register_t reg)
{
    sysbvm_operandVectorName_t vectorType = (sysbvm_operandVectorName_t) (operand & SYSBVM_OPERAND_VECTOR_BITMASK);
    int16_t vectorIndex = operand >> SYSBVM_OPERAND_VECTOR_BITS;
    if(vectorIndex < 0)
        return;

    int32_t vectorOffset = vectorIndex * sizeof(void*);
    switch(vectorType)
    {
    case SYSBVM_OPERAND_VECTOR_LOCAL:
        sysbvm_jit_x86_mov64IntoMemoryWithOffset(jit, SYSBVM_X86_RBP, jit->localVectorOffset + vectorOffset, reg);
        break;
    default:
        abort();
        break;
    }
}

static void sysbvm_jit_moveOperandToRegister(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t reg, int16_t operand)
{
    sysbvm_operandVectorName_t vectorType = (sysbvm_operandVectorName_t) (operand & SYSBVM_OPERAND_VECTOR_BITMASK);
    int16_t vectorIndex = operand >> SYSBVM_OPERAND_VECTOR_BITS;
    if(vectorIndex < 0)
    {
        sysbvm_jit_x86_xorRegister(jit, reg, reg);
        return;
    }

    int32_t vectorOffset = (int32_t)vectorIndex * sizeof(void*);
    switch(vectorType)
    {
    case SYSBVM_OPERAND_VECTOR_ARGUMENTS:
        sysbvm_jit_x86_mov64FromMemoryWithOffset(jit, reg, SYSBVM_X86_RBP, jit->argumentVectorOffset);
        sysbvm_jit_x86_mov64FromMemoryWithOffset(jit, reg, reg, vectorOffset);
        break;
    case SYSBVM_OPERAND_VECTOR_CAPTURES:
        sysbvm_jit_x86_mov64FromMemoryWithOffset(jit, reg, SYSBVM_X86_RBP, jit->captureVectorOffset);
        sysbvm_jit_x86_mov64FromMemoryWithOffset(jit, reg, reg, sizeof(sysbvm_tuple_header_t) + vectorOffset);
        break;
    case SYSBVM_OPERAND_VECTOR_LITERAL:
        sysbvm_jit_x86_mov64FromMemoryWithOffset(jit, reg, SYSBVM_X86_RBP, jit->literalVectorOffset);
        sysbvm_jit_x86_mov64FromMemoryWithOffset(jit, reg, reg, sizeof(sysbvm_tuple_header_t) + vectorOffset);
        break;
    case SYSBVM_OPERAND_VECTOR_LOCAL:
        sysbvm_jit_x86_mov64FromMemoryWithOffset(jit, reg, SYSBVM_X86_RBP, jit->localVectorOffset + vectorOffset);
        break;
    }
}

static void sysbvm_jit_pushOperand(sysbvm_bytecodeJit_t *jit, int16_t operand)
{
    sysbvm_operandVectorName_t vectorType = (sysbvm_operandVectorName_t) (operand & SYSBVM_OPERAND_VECTOR_BITMASK);
    int16_t vectorIndex = operand >> SYSBVM_OPERAND_VECTOR_BITS;
    if(vectorIndex < 0)
    {
        sysbvm_jit_x86_pushImmediate32(jit, 0);
        return;
    }

    int32_t vectorOffset = vectorIndex * sizeof(void*);
    sysbvm_x86_register_t scratchRegister = SYSBVM_X86_RAX;
    switch(vectorType)
    {
    case SYSBVM_OPERAND_VECTOR_ARGUMENTS:
        sysbvm_jit_x86_mov64FromMemoryWithOffset(jit, scratchRegister, SYSBVM_X86_RBP, jit->argumentVectorOffset);
        sysbvm_jit_x86_pushFromMemoryWithOffset(jit, scratchRegister, vectorOffset);
        break;
    case SYSBVM_OPERAND_VECTOR_CAPTURES:
        sysbvm_jit_x86_mov64FromMemoryWithOffset(jit, scratchRegister, SYSBVM_X86_RBP, jit->captureVectorOffset);
        sysbvm_jit_x86_pushFromMemoryWithOffset(jit, scratchRegister, sizeof(sysbvm_tuple_header_t) + vectorOffset);
        break;
    case SYSBVM_OPERAND_VECTOR_LITERAL:
        sysbvm_jit_x86_mov64FromMemoryWithOffset(jit, scratchRegister, SYSBVM_X86_RBP, jit->literalVectorOffset);
        sysbvm_jit_x86_pushFromMemoryWithOffset(jit, scratchRegister, sizeof(sysbvm_tuple_header_t) + vectorOffset);
        break;
    case SYSBVM_OPERAND_VECTOR_LOCAL:
        sysbvm_jit_x86_pushFromMemoryWithOffset(jit, SYSBVM_X86_RBP, jit->localVectorOffset + vectorOffset);
        break;
    }
}

static void sysbvm_jit_moveOperandToOperand(sysbvm_bytecodeJit_t *jit, int16_t destinationOperand, int16_t sourceOperand)
{
    if(destinationOperand < 0)
        return;

    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_RAX, sourceOperand);
    sysbvm_jit_moveRegisterToOperand(jit, destinationOperand, SYSBVM_X86_RAX);
}

static void sysbvm_jit_return(sysbvm_bytecodeJit_t *jit, int16_t operand)
{
    // Disconnect from the stack unwinder.
    sysbvm_jit_x86_leaRegisterWithOffset(jit, SYSBVM_X86_SYSV_ARG0, SYSBVM_X86_RBP, jit->stackFrameRecordOffset);
    sysbvm_jit_x86_call(jit, &sysbvm_stackFrame_popRecord);

    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_RAX, operand);
    sysbvm_jit_epilogue(jit);
}

static void sysbvm_jit_storePC(sysbvm_bytecodeJit_t *jit, uint16_t pc)
{
    sysbvm_jit_x86_movImmediateI32IntoMemory64WithOffset(jit, SYSBVM_X86_RBP, jit->pcOffset, pc);
}

static void sysbvm_jit_finish(sysbvm_bytecodeJit_t *jit)
{
    // Apply the PC target relative relocations.
    for(size_t i = 0; i < jit->pcRelocationsSize; ++i)
    {
        sysbvm_bytecodeJitPCRelocation_t *relocation = jit->pcRelocations + i;
        *((int32_t*)(jit->instructions + relocation->offset)) = (int32_t)(jit->pcDestinations[relocation->targetPC] - (intptr_t)relocation->offset + relocation->addend);
    }
}

static void sysbvm_jit_installIn(sysbvm_bytecodeJit_t *jit, uint8_t *codeZonePointer)
{
    memcpy(codeZonePointer, jit->instructions, jit->instructionsSize);
    size_t constantsOffset = sizeAlignedTo(jit->instructionsSize, 16);
    uint8_t *constantZonePointer = codeZonePointer + constantsOffset;
    memcpy(constantZonePointer, jit->constants, jit->constantsSize);

    for(size_t i = 0; i < jit->relocationsSize; ++i)
    {
        sysbvm_bytecodeJitRelocation_t *relocation = jit->relocations + i;
        uint8_t *relocationTarget = codeZonePointer + relocation->offset;
        intptr_t relocationTargetAddress = (intptr_t)relocationTarget;

        intptr_t relativeValue = (intptr_t)constantZonePointer + relocation->value - relocationTargetAddress + relocation->addend;
        switch(relocation->type)
        {
        case SYSBVM_BYTECODE_JIT_RELOCATION_RELATIVE32:
            *((int32_t*)relocationTarget) = (int32_t)relativeValue;
            break;
        case SYSBVM_BYTECODE_JIT_RELOCATION_RELATIVE64:
            *((int64_t*)relocationTarget) = (int64_t)relativeValue;
            break;
        }
    }
}
