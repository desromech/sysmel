#include "tuuvm/io.h"
#include "tuuvm/array.h"
#include "tuuvm/context.h"
#include "tuuvm/errors.h"
#include "tuuvm/function.h"
#include "tuuvm/string.h"
#include "tuuvm/assert.h"
#include <stdio.h>

TUUVM_API tuuvm_tuple_t tuuvm_io_readWholeFileNamedAsString(tuuvm_context_t *context, tuuvm_tuple_t filename)
{
    char *inputFileName = tuuvm_tuple_bytesToCString(filename);
    FILE *inputFile = fopen(inputFileName, "rb");
    if(!inputFile)
        return TUUVM_NULL_TUPLE;
    tuuvm_tuple_bytesToCStringFree(inputFileName);

    fseek(inputFile, 0, SEEK_END);
    size_t fileSize = ftell(inputFile);
    fseek(inputFile, 0, SEEK_SET);

    tuuvm_tuple_t readString = tuuvm_string_createEmptyWithSize(context, fileSize);
    if(readString)
    {
        if(fread(TUUVM_CAST_OOP_TO_OBJECT_TUPLE(readString)->bytes, fileSize, 1, inputFile) != 1)
            readString = TUUVM_NULL_TUPLE;
    }

    fclose(inputFile);

    return readString;
}

TUUVM_API tuuvm_tuple_t tuuvm_io_readWholeFileNamedAsByteArray(tuuvm_context_t *context, tuuvm_tuple_t filename)
{
    char *inputFileName = tuuvm_tuple_bytesToCString(filename);
    FILE *inputFile = fopen(inputFileName, "rb");
    if(!inputFile)
        return TUUVM_NULL_TUPLE;
    tuuvm_tuple_bytesToCStringFree(inputFileName);

    fseek(inputFile, 0, SEEK_END);
    size_t fileSize = ftell(inputFile);
    fseek(inputFile, 0, SEEK_SET);

    tuuvm_tuple_t readByteArray = tuuvm_byteArray_create(context, fileSize);
    if(readByteArray)
    {
        if(fread(TUUVM_CAST_OOP_TO_OBJECT_TUPLE(readByteArray)->bytes, fileSize, 1, inputFile) != 1)
            readByteArray = TUUVM_NULL_TUPLE;
    }

    fclose(inputFile);

    return readByteArray;
}

TUUVM_API bool tuuvm_io_saveWholeFileNamed(tuuvm_tuple_t filename, tuuvm_tuple_t content)
{
    char *outputFileName = tuuvm_tuple_bytesToCString(filename);
    FILE *outputFile = fopen(outputFileName, "wb");
    if(!outputFile)
        return false;
    tuuvm_tuple_bytesToCStringFree(outputFileName);

    bool success = true;
    if(tuuvm_tuple_isBytes(content))
    {
        size_t contentSize = tuuvm_tuple_getSizeInBytes(content);
        success = fwrite(TUUVM_CAST_OOP_TO_OBJECT_TUPLE(content)->bytes, contentSize, 1, outputFile) == 1;
    }
    fclose(outputFile);

    return success;
}

static tuuvm_tuple_t tuuvm_io_primitive_printLine(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    size_t parameterCount = tuuvm_array_getSize(arguments[0]);
    for(size_t i = 0; i < parameterCount; ++i)
    {
        tuuvm_tuple_t string = tuuvm_tuple_asString(context, tuuvm_array_at(arguments[0], i));
        TUUVM_ASSERT(tuuvm_tuple_isBytes(string));
        fwrite(TUUVM_CAST_OOP_TO_OBJECT_TUPLE(string)->bytes, tuuvm_tuple_getSizeInBytes(string), 1, stdout);

    }
    fwrite("\n", 1, 1, stdout);

    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_io_primitive_print(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    size_t parameterCount = tuuvm_array_getSize(arguments[0]);
    for(size_t i = 0; i < parameterCount; ++i)
    {
        tuuvm_tuple_t string = tuuvm_tuple_asString(context, tuuvm_array_at(arguments[0], i));
        TUUVM_ASSERT(tuuvm_tuple_isBytes(string));
        fwrite(TUUVM_CAST_OOP_TO_OBJECT_TUPLE(string)->bytes, tuuvm_tuple_getSizeInBytes(string), 1, stdout);

    }

    return TUUVM_VOID_TUPLE;
}

static tuuvm_tuple_t tuuvm_io_primitive_readWholeFileNamedAsString(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_io_readWholeFileNamedAsString(context, arguments[0]);
}

static tuuvm_tuple_t tuuvm_io_primitive_readWholeFileNamedAsByteArray(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_io_readWholeFileNamedAsString(context, arguments[0]);
}

static tuuvm_tuple_t tuuvm_io_primitive_saveWholeFileNamed(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return tuuvm_tuple_boolean_encode(tuuvm_io_saveWholeFileNamed(arguments[0], arguments[1]));
}

static tuuvm_tuple_t tuuvm_io_primitive_halt(tuuvm_context_t *context, tuuvm_tuple_t closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 0) tuuvm_error_argumentCountMismatch(0, argumentCount);

    return TUUVM_VOID_TUPLE;
}

void tuuvm_io_registerPrimitives(void)
{
    tuuvm_primitiveTable_registerFunction(tuuvm_io_primitive_printLine, "printLine");
    tuuvm_primitiveTable_registerFunction(tuuvm_io_primitive_print, "printLine");

    tuuvm_primitiveTable_registerFunction(tuuvm_io_primitive_readWholeFileNamedAsString, "IO::readWholeFileNamedAsString");
    tuuvm_primitiveTable_registerFunction(tuuvm_io_primitive_readWholeFileNamedAsByteArray, "IO::readWholeFileNamedAsByteArray");
    tuuvm_primitiveTable_registerFunction(tuuvm_io_primitive_saveWholeFileNamed, "IO::saveWholeFileNamed");
    tuuvm_primitiveTable_registerFunction(tuuvm_io_primitive_halt, "halt");
}

void tuuvm_io_setupPrimitives(tuuvm_context_t *context)
{
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "printLine", 1, TUUVM_FUNCTION_FLAGS_VARIADIC | TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_io_primitive_printLine);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "print", 1, TUUVM_FUNCTION_FLAGS_VARIADIC | TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_io_primitive_print);

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "IO::readWholeFileNamedAsString", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_io_primitive_readWholeFileNamedAsString);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "IO::readWholeFileNamedAsByteArray", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_io_primitive_readWholeFileNamedAsByteArray);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "IO::saveWholeFileNamed", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_io_primitive_saveWholeFileNamed);

    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "halt", 0, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_io_primitive_halt);
}
