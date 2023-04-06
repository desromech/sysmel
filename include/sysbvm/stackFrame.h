#ifndef SYSBVM_STACK_FRAME_H
#define SYSBVM_STACK_FRAME_H

#include "tuple.h"
#include <setjmp.h>

typedef void (sysbvm_GCRootIterationFunction_t) (void *userdata, sysbvm_tuple_t *root);

typedef enum sysbvm_stackFrameRecordType_e
{
    SYSBVM_STACK_FRAME_RECORD_TYPE_GC_ROOTS = 0,
    SYSBVM_STACK_FRAME_RECORD_TYPE_FUNCTION_ACTIVATION,
    SYSBVM_STACK_FRAME_RECORD_TYPE_BYTECODE_FUNCTION_ACTIVATION,
    SYSBVM_STACK_FRAME_RECORD_TYPE_BYTECODE_JIT_FUNCTION_ACTIVATION,
    SYSBVM_STACK_FRAME_RECORD_TYPE_BREAK_TARGET,
    SYSBVM_STACK_FRAME_RECORD_TYPE_CONTINUE_TARGET,
    SYSBVM_STACK_FRAME_RECORD_TYPE_SOURCE_POSITION,
    SYSBVM_STACK_FRAME_RECORD_TYPE_LANDING_PAD,
    SYSBVM_STACK_FRAME_RECORD_TYPE_CLEANUP,
} sysbvm_stackFrameRecordType_t;

typedef struct sysbvm_stackFrameRecord_s
{
    struct sysbvm_stackFrameRecord_s *previous;
    sysbvm_stackFrameRecordType_t type;
} sysbvm_stackFrameRecord_t;

typedef struct sysbvm_stackFrameGCRootsRecord_s
{
    sysbvm_stackFrameRecord_t *previous;
    sysbvm_stackFrameRecordType_t type;
    size_t rootCount;
    sysbvm_tuple_t *roots;
} sysbvm_stackFrameGCRootsRecord_t;

typedef struct sysbvm_stackFrameFunctionActivationRecord_s
{
    sysbvm_stackFrameRecord_t *previous;
    sysbvm_stackFrameRecordType_t type;
    sysbvm_tuple_t function;
    sysbvm_tuple_t functionDefinition;
    sysbvm_tuple_t applicationEnvironment;
    sysbvm_tuple_t result;
    jmp_buf jmpbuffer;
} sysbvm_stackFrameFunctionActivationRecord_t;

#define SYSBVM_BYTECODE_FUNCTION_OPERAND_REGISTER_FILE_SIZE 20

typedef struct sysbvm_stackFrameBytecodeFunctionActivationRecord_s
{
    sysbvm_stackFrameRecord_t *previous;
    sysbvm_stackFrameRecordType_t type;
    sysbvm_tuple_t function;
    sysbvm_tuple_t functionDefinition;
    sysbvm_tuple_t functionBytecode;

    sysbvm_tuple_t captureVector;
    sysbvm_tuple_t literalVector;
    sysbvm_tuple_t instructions;

    size_t argumentCount;
    sysbvm_tuple_t *arguments;

    size_t inlineLocalVectorSize;
    sysbvm_tuple_t *inlineLocalVector;

    sysbvm_tuple_t operandRegisterFile[SYSBVM_BYTECODE_FUNCTION_OPERAND_REGISTER_FILE_SIZE];

    sysbvm_tuple_t result;
    size_t pc;
    jmp_buf jmpbuffer;
} sysbvm_stackFrameBytecodeFunctionActivationRecord_t;

typedef struct sysbvm_stackFrameBytecodeFunctionJitActivationRecord_s
{
    sysbvm_stackFrameRecord_t *previous;
    sysbvm_stackFrameRecordType_t type;

    size_t pc;

    sysbvm_tuple_t literalVector;
    sysbvm_tuple_t captureVector;
    sysbvm_tuple_t function;
    size_t argumentCount;
    sysbvm_tuple_t *arguments;

    size_t inlineLocalVectorSize;
    sysbvm_tuple_t inlineLocalVector[];
} sysbvm_stackFrameBytecodeFunctionJitActivationRecord_t;

typedef struct sysbvm_stackFrameBreakTargetRecord_s
{
    sysbvm_stackFrameRecord_t *previous;
    sysbvm_stackFrameRecordType_t type;
    sysbvm_tuple_t environment;
    jmp_buf jmpbuffer;
} sysbvm_stackFrameBreakTargetRecord_t;

typedef struct sysbvm_stackFrameContinueTargetRecord_s
{
    sysbvm_stackFrameRecord_t *previous;
    sysbvm_stackFrameRecordType_t type;
    sysbvm_tuple_t environment;
    jmp_buf jmpbuffer;
} sysbvm_stackFrameContinueTargetRecord_t;

typedef struct sysbvm_stackFrameSourcePositionRecord_s
{
    sysbvm_stackFrameRecord_t *previous;
    sysbvm_stackFrameRecordType_t type;
    sysbvm_tuple_t sourcePosition;
} sysbvm_stackFrameSourcePositionRecord_t;

typedef struct sysbvm_stackFrameLandingPadRecord_s
{
    sysbvm_stackFrameRecord_t *previous;
    sysbvm_stackFrameRecordType_t type;
    bool keepStackTrace;
    sysbvm_tuple_t exceptionFilter;
    sysbvm_tuple_t stackTrace;
    sysbvm_tuple_t exception;
    jmp_buf jmpbuffer;
} sysbvm_stackFrameLandingPadRecord_t;

typedef struct sysbvm_stackFrameCleanupRecord_s
{
    sysbvm_stackFrameRecord_t *previous;
    sysbvm_stackFrameRecordType_t type;
    sysbvm_tuple_t action;
} sysbvm_stackFrameCleanupRecord_t;

#define SYSBVM_STACKFRAME_PUSH_GC_ROOTS(recordName, gcStackFrameRoots) \
    sysbvm_stackFrameGCRootsRecord_t recordName = { \
        NULL, SYSBVM_STACK_FRAME_RECORD_TYPE_GC_ROOTS, sizeof(gcStackFrameRoots) / sizeof(sysbvm_tuple_t), (sysbvm_tuple_t*)(&gcStackFrameRoots) \
    }; \
    sysbvm_stackFrame_pushRecord((sysbvm_stackFrameRecord_t*)&recordName)

#define SYSBVM_STACKFRAME_POP_GC_ROOTS(recordName) \
    sysbvm_stackFrame_popRecord((sysbvm_stackFrameRecord_t*)&recordName)

#define SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(recordName, sourcePosition) \
    sysbvm_stackFrameSourcePositionRecord_t recordName = { \
        NULL, SYSBVM_STACK_FRAME_RECORD_TYPE_SOURCE_POSITION, sourcePosition \
    }; \
    sysbvm_stackFrame_pushRecord((sysbvm_stackFrameRecord_t*)&recordName)

#define SYSBVM_STACKFRAME_POP_SOURCE_POSITION(recordName) \
    sysbvm_stackFrame_popRecord((sysbvm_stackFrameRecord_t*)&recordName)

SYSBVM_API void sysbvm_stackFrame_enterContext(sysbvm_context_t *context, sysbvm_stackFrameRecord_t *topLevelStackRecord);
SYSBVM_API void sysbvm_stackFrame_leaveContext();
SYSBVM_API sysbvm_context_t *sysbvm_stackFrame_getActiveContext();
SYSBVM_API sysbvm_stackFrameRecord_t *sysbvm_stackFrame_getActiveRecord();

SYSBVM_API void sysbvm_stackFrame_pushRecord(sysbvm_stackFrameRecord_t *record);
SYSBVM_API void sysbvm_stackFrame_popRecord(sysbvm_stackFrameRecord_t *record);

SYSBVM_API void sysbvm_stackFrame_iterateGCRootsInRecordWith(sysbvm_stackFrameRecord_t *record, void *userdata, sysbvm_GCRootIterationFunction_t iterationFunction);
SYSBVM_API void sysbvm_stackFrame_iterateGCRootsInStackWith(sysbvm_stackFrameRecord_t *stackBottomRecord, void *userdata, sysbvm_GCRootIterationFunction_t iterationFunction);

SYSBVM_API void sysbvm_stackFrame_raiseException(sysbvm_tuple_t exception);

SYSBVM_API bool sysbvm_stackFrame_isValidRecordInThisContext(sysbvm_stackFrameRecord_t *targetRecord);
SYSBVM_API void sysbvm_stackFrame_returnValueInto(sysbvm_tuple_t value, sysbvm_stackFrameRecord_t *targetRecord);
SYSBVM_API void sysbvm_stackFrame_breakInto(sysbvm_stackFrameRecord_t *targetRecord);
SYSBVM_API void sysbvm_stackFrame_continueInto(sysbvm_stackFrameRecord_t *targetRecord);

SYSBVM_API sysbvm_tuple_t sysbvm_stackFrame_buildStackTraceUpTo(sysbvm_stackFrameRecord_t *targetRecord);
SYSBVM_API void sysbvm_stackFrame_printStackTrace(sysbvm_context_t *context, sysbvm_tuple_t stackTrace);

#endif //SYSBVM_STACK_FRAME_H
