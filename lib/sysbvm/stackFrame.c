#include "sysbvm/stackFrame.h"
#include "sysbvm/array.h"
#include "sysbvm/orderedCollection.h"
#include "sysbvm/assert.h"
#include "sysbvm/bytecode.h"
#include "sysbvm/errors.h"
#include "sysbvm/environment.h"
#include "sysbvm/function.h"
#include "sysbvm/tuple.h"
#include "sysbvm/string.h"
#include <stdio.h>
#include <stdlib.h>

SYSBVM_THREAD_LOCAL sysbvm_context_t *sysbvm_stackFrame_activeContext;
SYSBVM_THREAD_LOCAL sysbvm_stackFrameRecord_t *sysbvm_stackFrame_activeRecord;
static void sysbvm_stackFrame_prepareUnwindingUntil(sysbvm_stackFrameRecord_t *targetRecord);

SYSBVM_API void sysbvm_stackFrame_enterContext(sysbvm_context_t *context, sysbvm_stackFrameRecord_t *topLevelStackRecord)
{
    sysbvm_stackFrame_activeContext = context;
    sysbvm_stackFrame_activeRecord = topLevelStackRecord;
}

SYSBVM_API void sysbvm_stackFrame_leaveContext()
{
    sysbvm_stackFrame_activeContext = NULL;
    sysbvm_stackFrame_activeRecord = NULL;
}

SYSBVM_API sysbvm_context_t *sysbvm_stackFrame_getActiveContext()
{
    return sysbvm_stackFrame_activeContext;
}

SYSBVM_API sysbvm_stackFrameRecord_t *sysbvm_stackFrame_getActiveRecord()
{
    return sysbvm_stackFrame_activeRecord;
}

SYSBVM_API void sysbvm_stackFrame_pushRecord(sysbvm_stackFrameRecord_t *record)
{
    SYSBVM_DASSERT(record->type < SYSBVM_STACK_FRAME_RECORD_TYPE_COUNT);
    record->previous = sysbvm_stackFrame_activeRecord;
    sysbvm_stackFrame_activeRecord = record;
}

SYSBVM_API void sysbvm_stackFrame_popRecord(sysbvm_stackFrameRecord_t *record)
{
    SYSBVM_DASSERT(record->type < SYSBVM_STACK_FRAME_RECORD_TYPE_COUNT);
    sysbvm_stackFrame_activeRecord = record->previous;

    SYSBVM_DASSERT(!sysbvm_stackFrame_activeRecord || sysbvm_stackFrame_activeRecord->type < SYSBVM_STACK_FRAME_RECORD_TYPE_COUNT);
}

SYSBVM_API void sysbvm_stackFrame_iterateGCRootsInRecordWith(sysbvm_stackFrameRecord_t *record, void *userdata, sysbvm_GCRootIterationFunction_t iterationFunction)
{
    switch(record->type)
    {
    case SYSBVM_STACK_FRAME_RECORD_TYPE_GC_ROOTS:
        {
            sysbvm_stackFrameGCRootsRecord_t *rootRecord = (sysbvm_stackFrameGCRootsRecord_t*)record;
            for(size_t i = 0; i < rootRecord->rootCount; ++i)
                iterationFunction(userdata, &rootRecord->roots[i]);
        }
        break;
    case SYSBVM_STACK_FRAME_RECORD_TYPE_FUNCTION_ACTIVATION:
        {
            sysbvm_stackFrameFunctionActivationRecord_t *functionRecord = (sysbvm_stackFrameFunctionActivationRecord_t*)record;
            iterationFunction(userdata, &functionRecord->function);
            iterationFunction(userdata, &functionRecord->functionDefinition);
            iterationFunction(userdata, &functionRecord->applicationEnvironment);
            iterationFunction(userdata, &functionRecord->result);
        }   
        break;
    case SYSBVM_STACK_FRAME_RECORD_TYPE_BYTECODE_FUNCTION_ACTIVATION:
        {
            sysbvm_stackFrameBytecodeFunctionActivationRecord_t *functionRecord = (sysbvm_stackFrameBytecodeFunctionActivationRecord_t*)record;
            iterationFunction(userdata, &functionRecord->function);
            iterationFunction(userdata, &functionRecord->functionDefinition);
            iterationFunction(userdata, &functionRecord->functionBytecode);

            iterationFunction(userdata, &functionRecord->captureVector);
            iterationFunction(userdata, &functionRecord->literalVector);
            iterationFunction(userdata, &functionRecord->instructions);

            for(size_t i = 0; i < functionRecord->argumentCount; ++i)
                iterationFunction(userdata, functionRecord->arguments + i);

            for(size_t i = 0; i < functionRecord->inlineLocalVectorSize; ++i)
                iterationFunction(userdata, functionRecord->inlineLocalVector + i);

            for(size_t i = 0; i < SYSBVM_BYTECODE_FUNCTION_OPERAND_REGISTER_FILE_SIZE; ++i)
                iterationFunction(userdata, functionRecord->operandRegisterFile + i);

            iterationFunction(userdata, &functionRecord->result);
        }   
        break;
    case SYSBVM_STACK_FRAME_RECORD_TYPE_BYTECODE_JIT_FUNCTION_ACTIVATION:
        {
            sysbvm_stackFrameBytecodeFunctionJitActivationRecord_t *functionRecord = (sysbvm_stackFrameBytecodeFunctionJitActivationRecord_t*)record;

            iterationFunction(userdata, &functionRecord->literalVector);
            iterationFunction(userdata, &functionRecord->captureVector);
            iterationFunction(userdata, &functionRecord->function);

            for(size_t i = 0; i < functionRecord->argumentCount; ++i)
                iterationFunction(userdata, functionRecord->arguments + i);

            for(size_t i = 0; i < functionRecord->callArgumentVectorSize; ++i)
                iterationFunction(userdata, functionRecord->callArgumentVector + i);

            for(size_t i = 0; i < functionRecord->inlineLocalVectorSize; ++i)
                iterationFunction(userdata, functionRecord->inlineLocalVector + i);
        }   
        break;
    case SYSBVM_STACK_FRAME_RECORD_TYPE_BREAK_TARGET:
        {
            sysbvm_stackFrameBreakTargetRecord_t *breakRecord = (sysbvm_stackFrameBreakTargetRecord_t*)record;
            iterationFunction(userdata, &breakRecord->environment);
        }   
        break;
    case SYSBVM_STACK_FRAME_RECORD_TYPE_CONTINUE_TARGET:
        {
            sysbvm_stackFrameContinueTargetRecord_t *continueRecord = (sysbvm_stackFrameContinueTargetRecord_t*)record;
            iterationFunction(userdata, &continueRecord->environment);
        }   
        break;
    case SYSBVM_STACK_FRAME_RECORD_TYPE_SOURCE_POSITION:
        {
            sysbvm_stackFrameSourcePositionRecord_t *sourcePositionRecord = (sysbvm_stackFrameSourcePositionRecord_t*)record;
            iterationFunction(userdata, &sourcePositionRecord->sourcePosition);
        }   
        break;
    case SYSBVM_STACK_FRAME_RECORD_TYPE_LANDING_PAD:
        {
            sysbvm_stackFrameLandingPadRecord_t *landingPadRecord = (sysbvm_stackFrameLandingPadRecord_t*)record;
            iterationFunction(userdata, &landingPadRecord->exceptionFilter);
            iterationFunction(userdata, &landingPadRecord->stackTrace);
            iterationFunction(userdata, &landingPadRecord->exception);
            iterationFunction(userdata, &landingPadRecord->action);
            iterationFunction(userdata, &landingPadRecord->actionResult);
        }   
        break;
    case SYSBVM_STACK_FRAME_RECORD_TYPE_CLEANUP:
        {
            sysbvm_stackFrameCleanupRecord_t *cleanupRecord = (sysbvm_stackFrameCleanupRecord_t*)record;
            iterationFunction(userdata, &cleanupRecord->action);
        }   
        break;
    default:
        // Should not reach here.
        abort();
        break;
    }
}

SYSBVM_API void sysbvm_stackFrame_iterateGCRootsInStackWith(sysbvm_stackFrameRecord_t *stackBottomRecord, void *userdata, sysbvm_GCRootIterationFunction_t iterationFunction)
{
    for(sysbvm_stackFrameRecord_t *record = stackBottomRecord; record; record = record->previous)
        sysbvm_stackFrame_iterateGCRootsInRecordWith(record, userdata, iterationFunction);
}

SYSBVM_API void sysbvm_stackFrame_raiseException(sysbvm_tuple_t exception)
{
    sysbvm_context_t *context = sysbvm_stackFrame_activeContext;
    sysbvm_stackFrameRecord_t *exceptionRecord = sysbvm_stackFrame_activeRecord;

    if(!exceptionRecord)
    {
        fprintf(stderr, "Cannot raise an exception without an active context.\n");
        abort();
    }


    // Find the landing pad record.
    sysbvm_stackFrameRecord_t *stackFrameRecord = exceptionRecord;
    while(stackFrameRecord)
    {
        if(stackFrameRecord->type == SYSBVM_STACK_FRAME_RECORD_TYPE_LANDING_PAD)
        {
            sysbvm_stackFrameLandingPadRecord_t *landingPadRecord = (sysbvm_stackFrameLandingPadRecord_t*)stackFrameRecord;
            if(!landingPadRecord->exceptionFilter || sysbvm_tuple_isKindOf(context, exception, landingPadRecord->exceptionFilter))
                break;
        }
        stackFrameRecord = stackFrameRecord->previous;
    }

    // Did we find it?
    if(!stackFrameRecord)
    {
        sysbvm_tuple_t errorString = sysbvm_tuple_asString(context, exception);
        fprintf(stderr, "Unhandled exception: " SYSBVM_STRING_PRINTF_FORMAT "\n", SYSBVM_STRING_PRINTF_ARG(errorString));
        sysbvm_stackFrame_printStackTrace(context, sysbvm_stackFrame_buildStackTraceUpTo(NULL));
        abort();
    }

    // We found it, transfer the control flow onto it.
    sysbvm_stackFrameLandingPadRecord_t *landingPadRecord = (sysbvm_stackFrameLandingPadRecord_t*)stackFrameRecord;
    landingPadRecord->exception = exception;

    // Do we have an action on it?
    if(landingPadRecord->action)
    {
        // Invoke the action.
        landingPadRecord->actionResult = sysbvm_function_apply1(context, landingPadRecord->action, landingPadRecord->exception);
        exception = landingPadRecord->exception;
    }

    if(landingPadRecord->keepStackTrace)
        landingPadRecord->stackTrace = sysbvm_stackFrame_buildStackTraceUpTo((sysbvm_stackFrameRecord_t*)landingPadRecord);

    sysbvm_stackFrame_prepareUnwindingUntil((sysbvm_stackFrameRecord_t*)landingPadRecord);
    sysbvm_stackFrame_activeRecord = (sysbvm_stackFrameRecord_t*)landingPadRecord;
    longjmp(landingPadRecord->jmpbuffer, 1);
}

SYSBVM_API bool sysbvm_stackFrame_isValidRecordInThisContext(sysbvm_stackFrameRecord_t *targetRecord)
{
    for(sysbvm_stackFrameRecord_t *position = sysbvm_stackFrame_activeRecord; position; position = position->previous)
    {
        if(position == targetRecord)
            return true;
    }

    return false;
}

static void sysbvm_stackFrame_prepareUnwinding(sysbvm_stackFrameRecord_t *targetRecord)
{
    sysbvm_context_t *context = sysbvm_stackFrame_activeContext;
    sysbvm_stackFrame_activeRecord = targetRecord;

    switch(targetRecord->type)
    {
    case SYSBVM_STACK_FRAME_RECORD_TYPE_FUNCTION_ACTIVATION:
        {
            sysbvm_stackFrameFunctionActivationRecord_t *activationRecord = (sysbvm_stackFrameFunctionActivationRecord_t*)targetRecord;
            if(activationRecord->applicationEnvironment)
                sysbvm_analysisAndEvaluationEnvironment_clearUnwindingRecords(context, activationRecord->applicationEnvironment);
        }
        break;
    case SYSBVM_STACK_FRAME_RECORD_TYPE_BREAK_TARGET:
        {
            sysbvm_stackFrameBreakTargetRecord_t *breakRecord = (sysbvm_stackFrameBreakTargetRecord_t*)targetRecord;
            if(breakRecord->environment)
                sysbvm_analysisAndEvaluationEnvironment_clearUnwindingRecords(context, breakRecord->environment);
        }
        break;
    case SYSBVM_STACK_FRAME_RECORD_TYPE_CONTINUE_TARGET:
        {
            sysbvm_stackFrameContinueTargetRecord_t *continueRecord = (sysbvm_stackFrameContinueTargetRecord_t*)targetRecord;
            if(continueRecord->environment)
                sysbvm_analysisAndEvaluationEnvironment_clearUnwindingRecords(context, continueRecord->environment);
        }
        break;
    case SYSBVM_STACK_FRAME_RECORD_TYPE_CLEANUP:
        {
            sysbvm_stackFrameCleanupRecord_t *cleanupRecord = (sysbvm_stackFrameCleanupRecord_t*)targetRecord;
            if(cleanupRecord->action)
            {
                sysbvm_stackFrame_activeRecord = targetRecord->previous;
                sysbvm_function_applyNoCheck0(sysbvm_stackFrame_activeContext, cleanupRecord->action);
            }
        }
        break;
    default:
        // Nothing special is required here.
        break;
    }
}

static void sysbvm_stackFrame_prepareUnwindingUntil(sysbvm_stackFrameRecord_t *targetRecord)
{
    for(sysbvm_stackFrameRecord_t *position = sysbvm_stackFrame_activeRecord; position; position = position->previous)
    {
        sysbvm_stackFrame_prepareUnwinding(targetRecord);
        if(position == targetRecord)
            return;
    }
}

SYSBVM_API void sysbvm_stackFrame_returnValueInto(sysbvm_tuple_t value, sysbvm_stackFrameRecord_t *targetRecord)
{
    if(!sysbvm_stackFrame_isValidRecordInThisContext(targetRecord)
        || targetRecord->type != SYSBVM_STACK_FRAME_RECORD_TYPE_FUNCTION_ACTIVATION)
        sysbvm_error("Cannot unwind for return into invalid target.");

    sysbvm_stackFrame_prepareUnwindingUntil(targetRecord);
    sysbvm_stackFrameFunctionActivationRecord_t *activationRecord = (sysbvm_stackFrameFunctionActivationRecord_t*)targetRecord;
    activationRecord->result = value;
    sysbvm_stackFrame_activeRecord = targetRecord;
    longjmp(activationRecord->jmpbuffer, 1);
}

SYSBVM_API void sysbvm_stackFrame_breakInto(sysbvm_stackFrameRecord_t *targetRecord)
{
    if(!sysbvm_stackFrame_isValidRecordInThisContext(targetRecord)
        || targetRecord->type != SYSBVM_STACK_FRAME_RECORD_TYPE_BREAK_TARGET)
        sysbvm_error("Cannot unwind for break into invalid target.");

    sysbvm_stackFrame_prepareUnwindingUntil(targetRecord);
    sysbvm_stackFrameBreakTargetRecord_t *breakRecord = (sysbvm_stackFrameBreakTargetRecord_t*)targetRecord;
    sysbvm_stackFrame_activeRecord = targetRecord;
    longjmp(breakRecord->jmpbuffer, 1);
}

SYSBVM_API void sysbvm_stackFrame_continueInto(sysbvm_stackFrameRecord_t *targetRecord)
{
    if(!sysbvm_stackFrame_isValidRecordInThisContext(targetRecord)
        || targetRecord->type != SYSBVM_STACK_FRAME_RECORD_TYPE_CONTINUE_TARGET)
        sysbvm_error("Cannot unwind for continue into invalid target.");

    sysbvm_stackFrame_prepareUnwindingUntil(targetRecord);
    sysbvm_stackFrameContinueTargetRecord_t *continueRecord = (sysbvm_stackFrameContinueTargetRecord_t*)targetRecord;
    sysbvm_stackFrame_activeRecord = targetRecord;
    longjmp(continueRecord->jmpbuffer, 1);
}

SYSBVM_API sysbvm_tuple_t sysbvm_stackFrame_buildStackTraceUpTo(sysbvm_stackFrameRecord_t *targetRecord)
{
    sysbvm_context_t *context = sysbvm_stackFrame_activeContext;
    sysbvm_tuple_t orderedCollection = sysbvm_orderedCollection_create(context);
    
    sysbvm_stackFrameRecord_t *stackFrameRecord = sysbvm_stackFrame_activeRecord;
    //sysbvm_tuple_t currentFunction = SYSBVM_NULL_TUPLE;
    sysbvm_tuple_t currentSourcePosition = SYSBVM_NULL_TUPLE;
    while(stackFrameRecord && stackFrameRecord != targetRecord)
    {
        if(stackFrameRecord->type == SYSBVM_STACK_FRAME_RECORD_TYPE_SOURCE_POSITION)
        {
            if(!currentSourcePosition)
                currentSourcePosition = ((sysbvm_stackFrameSourcePositionRecord_t*)stackFrameRecord)->sourcePosition;
        }
        else if(stackFrameRecord->type == SYSBVM_STACK_FRAME_RECORD_TYPE_FUNCTION_ACTIVATION)
        {
            if(currentSourcePosition)
            {
                sysbvm_orderedCollection_add(context, orderedCollection, currentSourcePosition);
                currentSourcePosition = SYSBVM_NULL_TUPLE;
            }
            //currentFunction = ((sysbvm_stackFrameFunctionActivationRecord_t*)stackFrameRecord)->function;
        }
        else if(stackFrameRecord->type == SYSBVM_STACK_FRAME_RECORD_TYPE_BYTECODE_FUNCTION_ACTIVATION)
        {
            if(currentSourcePosition)
                sysbvm_orderedCollection_add(context, orderedCollection, currentSourcePosition);
            currentSourcePosition = sysbvm_bytecodeInterpreter_getSourcePositionForActivationRecord(context, (sysbvm_stackFrameBytecodeFunctionActivationRecord_t*)stackFrameRecord);
            if(currentSourcePosition)
                sysbvm_orderedCollection_add(context, orderedCollection, currentSourcePosition);
            currentSourcePosition = SYSBVM_NULL_TUPLE;
        }
        else if(stackFrameRecord->type == SYSBVM_STACK_FRAME_RECORD_TYPE_BYTECODE_JIT_FUNCTION_ACTIVATION)
        {
            if(currentSourcePosition)
                sysbvm_orderedCollection_add(context, orderedCollection, currentSourcePosition);
            currentSourcePosition = sysbvm_bytecodeInterpreter_getSourcePositionForJitActivationRecord(context, (sysbvm_stackFrameBytecodeFunctionJitActivationRecord_t*)stackFrameRecord);
            if(currentSourcePosition)
                sysbvm_orderedCollection_add(context, orderedCollection, currentSourcePosition);
            currentSourcePosition = SYSBVM_NULL_TUPLE;
        }

        stackFrameRecord = stackFrameRecord->previous;
    }

    if(currentSourcePosition)
        sysbvm_orderedCollection_add(context, orderedCollection, currentSourcePosition);

    return sysbvm_orderedCollection_asArray(context, orderedCollection);
}

SYSBVM_API void sysbvm_stackFrame_printStackTrace(sysbvm_context_t *context, sysbvm_tuple_t stackTrace)
{
    size_t stackTraceSize = sysbvm_array_getSize(stackTrace);
    for(size_t i = 0; i < stackTraceSize; ++i)
    {
        sysbvm_tuple_t stackTraceRecord = sysbvm_array_at(stackTrace, i);
        sysbvm_tuple_t stackTraceRecordString = sysbvm_tuple_asString(context, stackTraceRecord);
        fprintf(stderr, SYSBVM_STRING_PRINTF_FORMAT "\n", SYSBVM_STRING_PRINTF_ARG(stackTraceRecordString));
    }
}

SYSBVM_API void sysbvm_stackFrame_printStackTraceHere()
{
    sysbvm_context_t *context = sysbvm_stackFrame_getActiveContext();
    sysbvm_tuple_t stackTrace = sysbvm_stackFrame_buildStackTraceUpTo(NULL);
    sysbvm_stackFrame_printStackTrace(context, stackTrace);
}

static void sysbvm_stackFrame_dumpStackGCRoots_iteration(void *userdata, sysbvm_tuple_t *root)
{
    fprintf((FILE*)userdata, "%p\n", root);
}

SYSBVM_API void sysbvm_stackFrame_dumpStackGCRootsToFile(FILE *file)
{
    sysbvm_stackFrame_iterateGCRootsInStackWith(sysbvm_stackFrame_getActiveRecord(), file,  sysbvm_stackFrame_dumpStackGCRoots_iteration);
}

SYSBVM_API void sysbvm_stackFrame_dumpStackGCRootsToFileNamed(const char *filename)
{
#ifdef _WIN32
    FILE *file = NULL;
    if(fopen_s(&file, filename, "wb"))
        return;
#else
    FILE *file = fopen(filename, "wb");
#endif
    sysbvm_stackFrame_iterateGCRootsInStackWith(sysbvm_stackFrame_getActiveRecord(), file,  sysbvm_stackFrame_dumpStackGCRoots_iteration);
    fclose(file);
}

SYSBVM_API void sysbvm_stackFrame_dumpStackGCRoots(void)
{
    sysbvm_stackFrame_dumpStackGCRootsToFile(stdout);
}