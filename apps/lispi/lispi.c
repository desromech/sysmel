#include <stdio.h>
#include <string.h>
#include <tuuvm/arrayList.h>
#include <tuuvm/context.h>
#include <tuuvm/environment.h>
#include <tuuvm/interpreter.h>
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

static tuuvm_tuple_t readWholeFileNamed(tuuvm_tuple_t inputFileNameTuple)
{
    char *inputFileName = tuuvm_tuple_bytesToCString(inputFileNameTuple);
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

static void processFileNamed(tuuvm_tuple_t inputFileNameTuple)
{
    tuuvm_tuple_t sourceString = readWholeFileNamed(inputFileNameTuple);
    tuuvm_interpreter_analyzeAndEvaluateStringWithEnvironment(context, tuuvm_environment_createDefaultForEvaluation(context), sourceString, inputFileNameTuple);
}

int main(int argc, const char *argv[])
{
    context = tuuvm_context_create();
    if(!context)
    {
        fprintf(stderr, "Failed to create tuuvm context.\n");
        return 1;
    }

    // Parse the command line.
    tuuvm_tuple_t filesToProcess = tuuvm_arrayList_create(context);
    tuuvm_tuple_t remainingArgs = tuuvm_arrayList_create(context);
    bool isParsingRemainingArgs = false;
    for(int i = 1; i < argc; ++i)
    {
        const char *arg = argv[i];
        if(isParsingRemainingArgs)
        {
            tuuvm_arrayList_add(context, remainingArgs, tuuvm_string_createWithCString(context, arg));
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
            tuuvm_arrayList_add(context, filesToProcess, tuuvm_string_createWithCString(context, arg));
        }
    }

    size_t inputFileSize = tuuvm_arrayList_getSize(filesToProcess);
    for(size_t i = 0; i < inputFileSize; ++i)
    {
        tuuvm_tuple_t inputFileName = tuuvm_arrayList_at(filesToProcess, i);
        processFileNamed(inputFileName);
    }

    tuuvm_context_destroy(context);
    
    return 0;
}
