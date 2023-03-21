#ifndef TUUVM_STACK_FRAME_H
#define TUUVM_STACK_FRAME_H

#include "tuple.h"
#include <setjmp.h>

typedef void (tuuvm_GCRootIterationFunction_t) (void *userdata, tuuvm_tuple_t *root);

typedef enum tuuvm_stackFrameRecordType_e
{
    TUUVM_STACK_FRAME_RECORD_TYPE_GC_ROOTS = 0,
    TUUVM_STACK_FRAME_RECORD_TYPE_FUNCTION_ACTIVATION,
    TUUVM_STACK_FRAME_RECORD_TYPE_BYTECODE_FUNCTION_ACTIVATION,
    TUUVM_STACK_FRAME_RECORD_TYPE_BREAK_TARGET,
    TUUVM_STACK_FRAME_RECORD_TYPE_CONTINUE_TARGET,
    TUUVM_STACK_FRAME_RECORD_TYPE_SOURCE_POSITION,
    TUUVM_STACK_FRAME_RECORD_TYPE_LANDING_PAD,
    TUUVM_STACK_FRAME_RECORD_TYPE_CLEANUP,
} tuuvm_stackFrameRecordType_t;

typedef struct tuuvm_stackFrameRecord_s
{
    struct tuuvm_stackFrameRecord_s *previous;
    tuuvm_stackFrameRecordType_t type;
} tuuvm_stackFrameRecord_t;

typedef struct tuuvm_stackFrameGCRootsRecord_s
{
    tuuvm_stackFrameRecord_t *previous;
    tuuvm_stackFrameRecordType_t type;
    size_t rootCount;
    tuuvm_tuple_t *roots;
} tuuvm_stackFrameGCRootsRecord_t;

typedef struct tuuvm_stackFrameFunctionActivationRecord_s
{
    tuuvm_stackFrameRecord_t *previous;
    tuuvm_stackFrameRecordType_t type;
    tuuvm_tuple_t function;
    tuuvm_tuple_t functionDefinition;
    tuuvm_tuple_t applicationEnvironment;
    tuuvm_tuple_t result;
    jmp_buf jmpbuffer;
} tuuvm_stackFrameFunctionActivationRecord_t;

#define TUUVM_BYTECODE_FUNCTION_OPERAND_REGISTER_FILE_SIZE 20

typedef struct tuuvm_stackFrameBytecodeFunctionActivationRecord_s
{
    tuuvm_stackFrameRecord_t *previous;
    tuuvm_stackFrameRecordType_t type;
    tuuvm_tuple_t function;
    tuuvm_tuple_t functionDefinition;
    tuuvm_tuple_t functionBytecode;

    tuuvm_tuple_t captureVector;
    tuuvm_tuple_t literalVector;
    tuuvm_tuple_t instructions;

    size_t argumentCount;
    tuuvm_tuple_t *arguments;

    size_t inlineLocalVectorSize;
    tuuvm_tuple_t *inlineLocalVector;

    tuuvm_tuple_t result;
    size_t pc;
    jmp_buf jmpbuffer;
} tuuvm_stackFrameBytecodeFunctionActivationRecord_t;

typedef struct tuuvm_stackFrameBreakTargetRecord_s
{
    tuuvm_stackFrameRecord_t *previous;
    tuuvm_stackFrameRecordType_t type;
    tuuvm_tuple_t environment;
    jmp_buf jmpbuffer;
} tuuvm_stackFrameBreakTargetRecord_t;

typedef struct tuuvm_stackFrameContinueTargetRecord_s
{
    tuuvm_stackFrameRecord_t *previous;
    tuuvm_stackFrameRecordType_t type;
    tuuvm_tuple_t environment;
    jmp_buf jmpbuffer;
} tuuvm_stackFrameContinueTargetRecord_t;

typedef struct tuuvm_stackFrameSourcePositionRecord_s
{
    tuuvm_stackFrameRecord_t *previous;
    tuuvm_stackFrameRecordType_t type;
    tuuvm_tuple_t sourcePosition;
} tuuvm_stackFrameSourcePositionRecord_t;

typedef struct tuuvm_stackFrameLandingPadRecord_s
{
    tuuvm_stackFrameRecord_t *previous;
    tuuvm_stackFrameRecordType_t type;
    bool keepStackTrace;
    tuuvm_tuple_t exceptionFilter;
    tuuvm_tuple_t stackTrace;
    tuuvm_tuple_t exception;
    jmp_buf jmpbuffer;
} tuuvm_stackFrameLandingPadRecord_t;

typedef struct tuuvm_stackFrameCleanupRecord_s
{
    tuuvm_stackFrameRecord_t *previous;
    tuuvm_stackFrameRecordType_t type;
    tuuvm_tuple_t action;
} tuuvm_stackFrameCleanupRecord_t;

#define TUUVM_STACKFRAME_PUSH_GC_ROOTS(recordName, gcStackFrameRoots) \
    tuuvm_stackFrameGCRootsRecord_t recordName = { \
        NULL, TUUVM_STACK_FRAME_RECORD_TYPE_GC_ROOTS, sizeof(gcStackFrameRoots) / sizeof(tuuvm_tuple_t), (tuuvm_tuple_t*)(&gcStackFrameRoots) \
    }; \
    tuuvm_stackFrame_pushRecord((tuuvm_stackFrameRecord_t*)&recordName)

#define TUUVM_STACKFRAME_POP_GC_ROOTS(recordName) \
    tuuvm_stackFrame_popRecord((tuuvm_stackFrameRecord_t*)&recordName)

#define TUUVM_STACKFRAME_PUSH_SOURCE_POSITION(recordName, sourcePosition) \
    tuuvm_stackFrameSourcePositionRecord_t recordName = { \
        NULL, TUUVM_STACK_FRAME_RECORD_TYPE_SOURCE_POSITION, sourcePosition \
    }; \
    tuuvm_stackFrame_pushRecord((tuuvm_stackFrameRecord_t*)&recordName)

#define TUUVM_STACKFRAME_POP_SOURCE_POSITION(recordName) \
    tuuvm_stackFrame_popRecord((tuuvm_stackFrameRecord_t*)&recordName)

TUUVM_API void tuuvm_stackFrame_enterContext(tuuvm_context_t *context, tuuvm_stackFrameRecord_t *topLevelStackRecord);
TUUVM_API void tuuvm_stackFrame_leaveContext();
TUUVM_API tuuvm_context_t *tuuvm_stackFrame_getActiveContext();
TUUVM_API tuuvm_stackFrameRecord_t *tuuvm_stackFrame_getActiveRecord();

TUUVM_API void tuuvm_stackFrame_pushRecord(tuuvm_stackFrameRecord_t *record);
TUUVM_API void tuuvm_stackFrame_popRecord(tuuvm_stackFrameRecord_t *record);

TUUVM_API void tuuvm_stackFrame_iterateGCRootsInRecordWith(tuuvm_stackFrameRecord_t *record, void *userdata, tuuvm_GCRootIterationFunction_t iterationFunction);
TUUVM_API void tuuvm_stackFrame_iterateGCRootsInStackWith(tuuvm_stackFrameRecord_t *stackBottomRecord, void *userdata, tuuvm_GCRootIterationFunction_t iterationFunction);

TUUVM_API void tuuvm_stackFrame_raiseException(tuuvm_tuple_t exception);

TUUVM_API bool tuuvm_stackFrame_isValidRecordInThisContext(tuuvm_stackFrameRecord_t *targetRecord);
TUUVM_API void tuuvm_stackFrame_returnValueInto(tuuvm_tuple_t value, tuuvm_stackFrameRecord_t *targetRecord);
TUUVM_API void tuuvm_stackFrame_breakInto(tuuvm_stackFrameRecord_t *targetRecord);
TUUVM_API void tuuvm_stackFrame_continueInto(tuuvm_stackFrameRecord_t *targetRecord);

TUUVM_API tuuvm_tuple_t tuuvm_stackFrame_buildStackTraceUpTo(tuuvm_stackFrameRecord_t *targetRecord);
TUUVM_API void tuuvm_stackFrame_printStackTrace(tuuvm_context_t *context, tuuvm_tuple_t stackTrace);

#endif //TUUVM_STACK_FRAME_H
