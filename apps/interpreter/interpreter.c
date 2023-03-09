#include <stdio.h>
#include <string.h>
#include <tuuvm/arrayList.h>
#include <tuuvm/context.h>
#include <tuuvm/environment.h>
#include <tuuvm/filesystem.h>
#include <tuuvm/sourceCode.h>
#include <tuuvm/interpreter.h>
#include <tuuvm/stackFrame.h>
#include <tuuvm/string.h>

static tuuvm_context_t *context;

static void printHelp()
{
    printf("tuuvm-lispi <inputfile> <inputfile> -- args\n");
}

static void printVersion()
{
    printf("tuuvm-lispi version 0.1\n");
}

int doMain(int argc, const char *argv[])
{
    struct {
        tuuvm_tuple_t filesToProcess;
        tuuvm_tuple_t remainingArgs;
        tuuvm_tuple_t inputFileName;
    } gcFrame = {};
    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    // Parse the command line.
    gcFrame.filesToProcess = tuuvm_arrayList_create(context);
    gcFrame.remainingArgs = tuuvm_arrayList_create(context);
    bool isParsingRemainingArgs = false;
    for(int i = 1; i < argc; ++i)
    {
        const char *arg = argv[i];
        if(isParsingRemainingArgs)
        {
            tuuvm_arrayList_add(context, gcFrame.remainingArgs, tuuvm_string_createWithCString(context, arg));
            continue;
        }
        
        if(*arg == '-')
        {
            if(!strcmp(arg, "-help"))
            {
                printHelp();
                return 0;
            }
            else if(!strcmp(arg, "-version"))
            {
                printVersion();
                return 0;
            }
            else if(!strcmp(arg, "--"))
            {
                isParsingRemainingArgs = true;
            }
        }
        else
        {
            gcFrame.inputFileName = tuuvm_string_createWithCString(context, arg);
            tuuvm_arrayList_add(context, gcFrame.filesToProcess, gcFrame.inputFileName);
        }
    }

    size_t inputFileSize = tuuvm_arrayList_getSize(gcFrame.filesToProcess);
    for(size_t i = 0; i < inputFileSize; ++i)
    {
        gcFrame.inputFileName = tuuvm_arrayList_at(gcFrame.filesToProcess, i);
        gcFrame.inputFileName = tuuvm_filesystem_absolute(context, gcFrame.inputFileName);
        tuuvm_interpreter_loadSourceNamedWithSolvedPath(context, gcFrame.inputFileName);
    }

    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return 0;
}

int main(int argc, const char *argv[])
{
    context = tuuvm_context_create();
    if(!context)
    {
        fprintf(stderr, "Failed to create tuuvm context.\n");
        return 1;
    }

    tuuvm_stackFrameLandingPadRecord_t topLevelFrame = {
        .type = TUUVM_STACK_FRAME_RECORD_TYPE_LANDING_PAD,
        .keepStackTrace = true
    };

    tuuvm_stackFrame_enterContext(context, (tuuvm_stackFrameRecord_t*)&topLevelFrame);
    int exitCode = 0;
    if(!setjmp(topLevelFrame.jmpbuffer))
    {
        doMain(argc, argv);
    }
    else
    {
        tuuvm_tuple_t errorString = tuuvm_tuple_asString(context, topLevelFrame.exception);
        fprintf(stderr, "Unhandled exception: " TUUVM_STRING_PRINTF_FORMAT "\n", TUUVM_STRING_PRINTF_ARG(errorString));
        tuuvm_stackFrame_printStackTrace(context, topLevelFrame.stackTrace);
        exitCode = 1;
    }

    tuuvm_stackFrame_leaveContext();
    tuuvm_context_destroy(context);
    
    return exitCode;
}
