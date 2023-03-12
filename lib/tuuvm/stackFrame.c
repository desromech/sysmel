#include "tuuvm/stackFrame.h"
#include "tuuvm/arrayList.h"
#include "tuuvm/arraySlice.h"
#include "tuuvm/errors.h"
#include "tuuvm/environment.h"
#include "tuuvm/function.h"
#include "tuuvm/tuple.h"
#include "tuuvm/string.h"
#include <stdio.h>
#include <stdlib.h>

TUUVM_THREAD_LOCAL tuuvm_context_t *tuuvm_stackFrame_activeContext;
TUUVM_THREAD_LOCAL tuuvm_stackFrameRecord_t *tuuvm_stackFrame_activeRecord;
static void tuuvm_stackFrame_prepareUnwindingUntil(tuuvm_stackFrameRecord_t *targetRecord);

TUUVM_API void tuuvm_stackFrame_enterContext(tuuvm_context_t *context, tuuvm_stackFrameRecord_t *topLevelStackRecord)
{
    tuuvm_stackFrame_activeContext = context;
    tuuvm_stackFrame_activeRecord = topLevelStackRecord;
}

TUUVM_API void tuuvm_stackFrame_leaveContext()
{
    tuuvm_stackFrame_activeContext = NULL;
    tuuvm_stackFrame_activeRecord = NULL;
}

TUUVM_API tuuvm_context_t *tuuvm_stackFrame_getActiveContext()
{
    return tuuvm_stackFrame_activeContext;
}

TUUVM_API tuuvm_stackFrameRecord_t *tuuvm_stackFrame_getActiveRecord()
{
    return tuuvm_stackFrame_activeRecord;
}

TUUVM_API void tuuvm_stackFrame_pushRecord(tuuvm_stackFrameRecord_t *record)
{
    record->previous = tuuvm_stackFrame_activeRecord;
    tuuvm_stackFrame_activeRecord = record;
}

TUUVM_API void tuuvm_stackFrame_popRecord(tuuvm_stackFrameRecord_t *record)
{
    tuuvm_stackFrame_activeRecord = record->previous;
}

TUUVM_API void tuuvm_stackFrame_iterateGCRootsInRecordWith(tuuvm_stackFrameRecord_t *record, void *userdata, tuuvm_GCRootIterationFunction_t iterationFunction)
{
    switch(record->type)
    {
    case TUUVM_STACK_FRAME_RECORD_TYPE_GC_ROOTS:
        {
            tuuvm_stackFrameGCRootsRecord_t *rootRecord = (tuuvm_stackFrameGCRootsRecord_t*)record;
            for(size_t i = 0; i < rootRecord->rootCount; ++i)
                iterationFunction(userdata, &rootRecord->roots[i]);
        }
        break;
    case TUUVM_STACK_FRAME_RECORD_TYPE_FUNCTION_ACTIVATION:
        {
            tuuvm_stackFrameFunctionActivationRecord_t *functionRecord = (tuuvm_stackFrameFunctionActivationRecord_t*)record;
            iterationFunction(userdata, &functionRecord->function);
            iterationFunction(userdata, &functionRecord->functionDefinition);
            iterationFunction(userdata, &functionRecord->applicationEnvironment);
            iterationFunction(userdata, &functionRecord->result);
        }   
        break;
    case TUUVM_STACK_FRAME_RECORD_TYPE_BREAK_TARGET:
        {
            tuuvm_stackFrameBreakTargetRecord_t *breakRecord = (tuuvm_stackFrameBreakTargetRecord_t*)record;
            iterationFunction(userdata, &breakRecord->environment);
        }   
        break;
    case TUUVM_STACK_FRAME_RECORD_TYPE_CONTINUE_TARGET:
        {
            tuuvm_stackFrameContinueTargetRecord_t *continueRecord = (tuuvm_stackFrameContinueTargetRecord_t*)record;
            iterationFunction(userdata, &continueRecord->environment);
        }   
        break;
    case TUUVM_STACK_FRAME_RECORD_TYPE_SOURCE_POSITION:
        {
            tuuvm_stackFrameSourcePositionRecord_t *sourcePositionRecord = (tuuvm_stackFrameSourcePositionRecord_t*)record;
            iterationFunction(userdata, &sourcePositionRecord->sourcePosition);
        }   
        break;
    case TUUVM_STACK_FRAME_RECORD_TYPE_LANDING_PAD:
        {
            tuuvm_stackFrameLandingPadRecord_t *landingPadRecord = (tuuvm_stackFrameLandingPadRecord_t*)record;
            iterationFunction(userdata, &landingPadRecord->exceptionFilter);
            iterationFunction(userdata, &landingPadRecord->stackTrace);
            iterationFunction(userdata, &landingPadRecord->exception);
        }   
        break;
    case TUUVM_STACK_FRAME_RECORD_TYPE_CLEANUP:
        {
            tuuvm_stackFrameCleanupRecord_t *cleanupRecord = (tuuvm_stackFrameCleanupRecord_t*)record;
            iterationFunction(userdata, &cleanupRecord->action);
        }   
        break;
    default:
        // Should not reach here.
        abort();
        break;
    }
}

TUUVM_API void tuuvm_stackFrame_iterateGCRootsInStackWith(tuuvm_stackFrameRecord_t *stackBottomRecord, void *userdata, tuuvm_GCRootIterationFunction_t iterationFunction)
{
    for(tuuvm_stackFrameRecord_t *record = stackBottomRecord; record; record = record->previous)
        tuuvm_stackFrame_iterateGCRootsInRecordWith(record, userdata, iterationFunction);
}

TUUVM_API void tuuvm_stackFrame_raiseException(tuuvm_tuple_t exception)
{
    tuuvm_context_t *context = tuuvm_stackFrame_activeContext;
    tuuvm_stackFrameRecord_t *exceptionRecord = tuuvm_stackFrame_activeRecord;

    if(!exceptionRecord)
    {
        fprintf(stderr, "Cannot raise an exception without an active context.\n");
        abort();
    }


    // Find the landing pad record.
    tuuvm_stackFrameRecord_t *stackFrameRecord = exceptionRecord;
    while(stackFrameRecord && stackFrameRecord->type != TUUVM_STACK_FRAME_RECORD_TYPE_LANDING_PAD)
        stackFrameRecord = stackFrameRecord->previous;

    // Did we find it?
    if(!stackFrameRecord)
    {
        tuuvm_tuple_t errorString = tuuvm_tuple_asString(context, exception);
        fprintf(stderr, "Unhandled exception: " TUUVM_STRING_PRINTF_FORMAT "\n", TUUVM_STRING_PRINTF_ARG(errorString));
        tuuvm_stackFrame_printStackTrace(context, tuuvm_stackFrame_buildStackTraceUpTo(NULL));
        abort();
    }

    // We found it, transfer the control flow onto it.
    tuuvm_stackFrameLandingPadRecord_t *landingPadRecord = (tuuvm_stackFrameLandingPadRecord_t*)stackFrameRecord;
    landingPadRecord->exception = exception;
    if(landingPadRecord->keepStackTrace)
        landingPadRecord->stackTrace = tuuvm_stackFrame_buildStackTraceUpTo((tuuvm_stackFrameRecord_t*)landingPadRecord);

    tuuvm_stackFrame_prepareUnwindingUntil((tuuvm_stackFrameRecord_t*)landingPadRecord);
    tuuvm_stackFrame_activeRecord = (tuuvm_stackFrameRecord_t*)landingPadRecord;
    longjmp(landingPadRecord->jmpbuffer, 1);
}

TUUVM_API bool tuuvm_stackFrame_isValidRecordInThisContext(tuuvm_stackFrameRecord_t *targetRecord)
{
    for(tuuvm_stackFrameRecord_t *position = tuuvm_stackFrame_activeRecord; position; position = position->previous)
    {
        if(position == targetRecord)
            return true;
    }

    return false;
}

static void tuuvm_stackFrame_prepareUnwinding(tuuvm_stackFrameRecord_t *targetRecord)
{
    tuuvm_stackFrame_activeRecord = targetRecord;

    switch(targetRecord->type)
    {
    case TUUVM_STACK_FRAME_RECORD_TYPE_FUNCTION_ACTIVATION:
        {
            tuuvm_stackFrameFunctionActivationRecord_t *activationRecord = (tuuvm_stackFrameFunctionActivationRecord_t*)targetRecord;
            if(activationRecord->applicationEnvironment)
                tuuvm_environment_clearUnwindingRecords(activationRecord->applicationEnvironment);
        }
        break;
    case TUUVM_STACK_FRAME_RECORD_TYPE_BREAK_TARGET:
        {
            tuuvm_stackFrameBreakTargetRecord_t *breakRecord = (tuuvm_stackFrameBreakTargetRecord_t*)targetRecord;
            if(breakRecord->environment)
                tuuvm_environment_clearUnwindingRecords(breakRecord->environment);
        }
        break;
    case TUUVM_STACK_FRAME_RECORD_TYPE_CONTINUE_TARGET:
        {
            tuuvm_stackFrameContinueTargetRecord_t *continueRecord = (tuuvm_stackFrameContinueTargetRecord_t*)targetRecord;
            if(continueRecord->environment)
                tuuvm_environment_clearUnwindingRecords(continueRecord->environment);
        }
        break;
    case TUUVM_STACK_FRAME_RECORD_TYPE_CLEANUP:
        {
            tuuvm_stackFrameCleanupRecord_t *cleanupRecord = (tuuvm_stackFrameCleanupRecord_t*)targetRecord;
            if(cleanupRecord->action)
            {
                tuuvm_stackFrame_activeRecord = targetRecord->previous;
                tuuvm_function_apply0(tuuvm_stackFrame_activeContext, cleanupRecord->action);
            }
        }
        break;
    default:
        // Nothing special is required here.
        break;
    }
}

static void tuuvm_stackFrame_prepareUnwindingUntil(tuuvm_stackFrameRecord_t *targetRecord)
{
    for(tuuvm_stackFrameRecord_t *position = tuuvm_stackFrame_activeRecord; position; position = position->previous)
    {
        tuuvm_stackFrame_prepareUnwinding(targetRecord);
        if(position == targetRecord)
            return;
    }
}

TUUVM_API void tuuvm_stackFrame_returnValueInto(tuuvm_tuple_t value, tuuvm_stackFrameRecord_t *targetRecord)
{
    if(!tuuvm_stackFrame_isValidRecordInThisContext(targetRecord)
        || targetRecord->type != TUUVM_STACK_FRAME_RECORD_TYPE_FUNCTION_ACTIVATION)
        tuuvm_error("Cannot unwind for return into invalid target.");

    tuuvm_stackFrame_prepareUnwindingUntil(targetRecord);
    tuuvm_stackFrameFunctionActivationRecord_t *activationRecord = (tuuvm_stackFrameFunctionActivationRecord_t*)targetRecord;
    activationRecord->result = value;
    tuuvm_stackFrame_activeRecord = targetRecord;
    longjmp(activationRecord->jmpbuffer, 1);
}

TUUVM_API void tuuvm_stackFrame_breakInto(tuuvm_stackFrameRecord_t *targetRecord)
{
    if(!tuuvm_stackFrame_isValidRecordInThisContext(targetRecord)
        || targetRecord->type != TUUVM_STACK_FRAME_RECORD_TYPE_BREAK_TARGET)
        tuuvm_error("Cannot unwind for break into invalid target.");

    tuuvm_stackFrame_prepareUnwindingUntil(targetRecord);
    tuuvm_stackFrameBreakTargetRecord_t *breakRecord = (tuuvm_stackFrameBreakTargetRecord_t*)targetRecord;
    tuuvm_stackFrame_activeRecord = targetRecord;
    longjmp(breakRecord->jmpbuffer, 1);
}

TUUVM_API void tuuvm_stackFrame_continueInto(tuuvm_stackFrameRecord_t *targetRecord)
{
    if(!tuuvm_stackFrame_isValidRecordInThisContext(targetRecord)
        || targetRecord->type != TUUVM_STACK_FRAME_RECORD_TYPE_CONTINUE_TARGET)
        tuuvm_error("Cannot unwind for continue into invalid target.");

    tuuvm_stackFrame_prepareUnwindingUntil(targetRecord);
    tuuvm_stackFrameContinueTargetRecord_t *continueRecord = (tuuvm_stackFrameContinueTargetRecord_t*)targetRecord;
    tuuvm_stackFrame_activeRecord = targetRecord;
    longjmp(continueRecord->jmpbuffer, 1);
}

TUUVM_API tuuvm_tuple_t tuuvm_stackFrame_buildStackTraceUpTo(tuuvm_stackFrameRecord_t *targetRecord)
{
    tuuvm_context_t *context = tuuvm_stackFrame_activeContext;
    tuuvm_tuple_t arrayList = tuuvm_arrayList_create(context);
    
    tuuvm_stackFrameRecord_t *stackFrameRecord = tuuvm_stackFrame_activeRecord;
    //tuuvm_tuple_t currentFunction = TUUVM_NULL_TUPLE;
    tuuvm_tuple_t currentSourcePosition = TUUVM_NULL_TUPLE;
    while(stackFrameRecord && stackFrameRecord != targetRecord)
    {
        if(stackFrameRecord->type == TUUVM_STACK_FRAME_RECORD_TYPE_SOURCE_POSITION)
        {
            if(!currentSourcePosition)
                currentSourcePosition = ((tuuvm_stackFrameSourcePositionRecord_t*)stackFrameRecord)->sourcePosition;
        }
        else if(stackFrameRecord->type == TUUVM_STACK_FRAME_RECORD_TYPE_FUNCTION_ACTIVATION)
        {
            if(currentSourcePosition)
            {
                tuuvm_arrayList_add(context, arrayList, currentSourcePosition);
                currentSourcePosition = TUUVM_NULL_TUPLE;
            }
            //currentFunction = ((tuuvm_stackFrameFunctionActivationRecord_t*)stackFrameRecord)->function;
        }

        stackFrameRecord = stackFrameRecord->previous;
    }

    if(currentSourcePosition)
        tuuvm_arrayList_add(context, arrayList, currentSourcePosition);

    return tuuvm_arrayList_asArraySlice(context, arrayList);
}

TUUVM_API void tuuvm_stackFrame_printStackTrace(tuuvm_context_t *context, tuuvm_tuple_t stackTrace)
{
    size_t stackTraceSize = tuuvm_arraySlice_getSize(stackTrace);
    for(size_t i = 0; i < stackTraceSize; ++i)
    {
        tuuvm_tuple_t stackTraceRecord = tuuvm_arraySlice_at(stackTrace, i);
        tuuvm_tuple_t stackTraceRecordString = tuuvm_tuple_asString(context, stackTraceRecord);
        fprintf(stderr, TUUVM_STRING_PRINTF_FORMAT "\n", TUUVM_STRING_PRINTF_ARG(stackTraceRecordString));
    }
}

TUUVM_API void tuuvm_stackFrame_printStackTraceHere()
{
    tuuvm_context_t *context = tuuvm_stackFrame_getActiveContext();
    tuuvm_tuple_t stackTrace = tuuvm_stackFrame_buildStackTraceUpTo(NULL);
    tuuvm_stackFrame_printStackTrace(context, stackTrace);
}

static void tuuvm_stackFrame_dumpStackGCRoots_iteration(void *userdata, tuuvm_tuple_t *root)
{
    fprintf((FILE*)userdata, "%p\n", root);
}

TUUVM_API void tuuvm_stackFrame_dumpStackGCRootsToFile(FILE *file)
{
    tuuvm_stackFrame_iterateGCRootsInStackWith(tuuvm_stackFrame_getActiveRecord(), file,  tuuvm_stackFrame_dumpStackGCRoots_iteration);
}

TUUVM_API void tuuvm_stackFrame_dumpStackGCRootsToFileNamed(const char *filename)
{
    FILE *file = fopen(filename, "wb");
    tuuvm_stackFrame_iterateGCRootsInStackWith(tuuvm_stackFrame_getActiveRecord(), file,  tuuvm_stackFrame_dumpStackGCRoots_iteration);
    fclose(file);
}

TUUVM_API void tuuvm_stackFrame_dumpStackGCRoots(void)
{
    tuuvm_stackFrame_dumpStackGCRootsToFile(stdout);
}