#include <stdio.h>
#include <string.h>
#include <tuuvm/arrayList.h>
#include <tuuvm/context.h>
#include <tuuvm/environment.h>
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

static tuuvm_tuple_t readWholeFileNamed(tuuvm_tuple_t *inputFileNameTuple)
{
    char *inputFileName = tuuvm_tuple_bytesToCString(*inputFileNameTuple);
    FILE *inputFile = fopen(inputFileName, "rb");
    if(!inputFile)
        return TUUVM_NULL_TUPLE;
    tuuvm_tuple_bytesToCStringFree(inputFileName);

    fseek(inputFile, 0, SEEK_END);
    size_t fileSize = ftell(inputFile);
    fseek(inputFile, 0, SEEK_SET);

    tuuvm_tuple_t sourceString = tuuvm_string_createEmptyWithSize(context, fileSize);
    if(sourceString)
    {
        if(fread(TUUVM_CAST_OOP_TO_OBJECT_TUPLE(sourceString)->bytes, fileSize, 1, inputFile) != 1)
            return TUUVM_NULL_TUPLE;
    }

    fclose(inputFile);

    return sourceString;
}

static tuuvm_tuple_t languageForFileName(tuuvm_tuple_t *inputFileNameTuple)
{
    if(tuuvm_string_endsWithCString(*inputFileNameTuple, ".sysmel"))
        return tuuvm_symbol_internWithCString(context, "sysmel");
    return tuuvm_symbol_internWithCString(context, "tlisp");
}

static void processFileNamed(tuuvm_tuple_t *inputFileNameTuple)
{
    struct {
        tuuvm_tuple_t sourceString;
    } gcFrame = {};

    TUUVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.sourceString = readWholeFileNamed(inputFileNameTuple);
    tuuvm_interpreter_analyzeAndEvaluateStringWithEnvironment(context, tuuvm_environment_createDefaultForEvaluation(context), gcFrame.sourceString, *inputFileNameTuple, languageForFileName(inputFileNameTuple));
    TUUVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
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
            tuuvm_arrayList_add(context, gcFrame.filesToProcess, tuuvm_string_createWithCString(context, arg));
        }
    }

    size_t inputFileSize = tuuvm_arrayList_getSize(gcFrame.filesToProcess);
    for(size_t i = 0; i < inputFileSize; ++i)
    {
        gcFrame.inputFileName = tuuvm_arrayList_at(gcFrame.filesToProcess, i);
        processFileNamed(&gcFrame.inputFileName);
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
        tuuvm_tuple_t errorString = tuuvm_tuple_toString(context, topLevelFrame.exception);
        fprintf(stderr, "Unhandled exception: " TUUVM_STRING_PRINTF_FORMAT "\n", TUUVM_STRING_PRINTF_ARG(errorString));
        tuuvm_stackFrame_printStackTrace(context, topLevelFrame.stackTrace);
        exitCode = 1;
    }

    tuuvm_stackFrame_leaveContext();
    tuuvm_context_destroy(context);
    
    return exitCode;
}
