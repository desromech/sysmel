#include <stdio.h>
#include <string.h>
#include <sysbvm/arrayList.h>
#include <sysbvm/context.h>
#include <sysbvm/environment.h>
#include <sysbvm/filesystem.h>
#include <sysbvm/sourceCode.h>
#include <sysbvm/interpreter.h>
#include <sysbvm/stackFrame.h>
#include <sysbvm/string.h>

static sysbvm_context_t *context;

const char *destinationImageFilename;

static void printHelp()
{
    printf("sysbvm-lispi <inputfile> <inputfile> -- args\n");
}

static void printVersion()
{
    printf("sysbvm-lispi version 0.1\n");
}

int doMain(int startArgumentIndex, int argc, const char *argv[])
{
    struct {
        sysbvm_tuple_t filesToProcess;
        sysbvm_tuple_t remainingArgs;
        sysbvm_tuple_t inputFileName;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    // Parse the command line.
    gcFrame.filesToProcess = sysbvm_arrayList_create(context);
    gcFrame.remainingArgs = sysbvm_arrayList_create(context);
    bool isParsingRemainingArgs = false;
    for(int i = startArgumentIndex; i < argc; ++i)
    {
        const char *arg = argv[i];
        if(isParsingRemainingArgs)
        {
            sysbvm_arrayList_add(context, gcFrame.remainingArgs, sysbvm_string_createWithCString(context, arg));
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
            else if(!strcmp(arg, "-save-image"))
            {
                arg = argv[++i];
                destinationImageFilename = arg;
            }
            else if(!strcmp(arg, "--"))
            {
                isParsingRemainingArgs = true;
            }
        }
        else
        {
            gcFrame.inputFileName = sysbvm_string_createWithCString(context, arg);
            sysbvm_arrayList_add(context, gcFrame.filesToProcess, gcFrame.inputFileName);
        }
    }

    size_t inputFileSize = sysbvm_arrayList_getSize(gcFrame.filesToProcess);
    for(size_t i = 0; i < inputFileSize; ++i)
    {
        gcFrame.inputFileName = sysbvm_arrayList_at(gcFrame.filesToProcess, i);
        gcFrame.inputFileName = sysbvm_filesystem_absolute(context, gcFrame.inputFileName);
        sysbvm_interpreter_loadSourceNamedWithSolvedPath(context, gcFrame.inputFileName);
    }

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return 0;
}

int mainWithContext(int startArgumentIndex, int argc, const char *argv[])
{
    sysbvm_stackFrameLandingPadRecord_t topLevelFrame = {
        .type = SYSBVM_STACK_FRAME_RECORD_TYPE_LANDING_PAD,
        .keepStackTrace = true
    };

    sysbvm_stackFrame_enterContext(context, (sysbvm_stackFrameRecord_t*)&topLevelFrame);
    int exitCode = 0;
    if(!_setjmp(topLevelFrame.jmpbuffer))
    {
        doMain(startArgumentIndex, argc, argv);
    }
    else
    {
        sysbvm_tuple_t errorString = sysbvm_tuple_asString(context, topLevelFrame.exception);
        fprintf(stderr, "Unhandled exception: " SYSBVM_STRING_PRINTF_FORMAT "\n", SYSBVM_STRING_PRINTF_ARG(errorString));
        sysbvm_stackFrame_printStackTrace(context, topLevelFrame.stackTrace);
        exitCode = 1;
    }

    sysbvm_stackFrame_leaveContext();
    return exitCode;
}

int main(int argc, const char *argv[])
{
    // Allow creating the context by loading it from an image.
    int startArgumentIndex = 1;
    if(argc >= 3 && !strcmp(argv[1], "-load-image"))
    {
        context = sysbvm_context_loadImageFromFileNamed(argv[2]);
        startArgumentIndex = 3;
    }
    else
    {
        context = sysbvm_context_create();
    }

    if(!context)
    {
        fprintf(stderr, "Failed to create sysbvm context.\n");
        return 1;
    }

    int exitCode = mainWithContext(startArgumentIndex, argc, argv);

    // Allow saving the context as an image.
    if(destinationImageFilename)
        sysbvm_context_saveImageToFileNamed(context, destinationImageFilename);
    sysbvm_context_destroy(context);
    
    return exitCode;
}
