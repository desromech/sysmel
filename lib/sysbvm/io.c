#include "sysbvm/io.h"
#include "sysbvm/array.h"
#include "sysbvm/context.h"
#include "sysbvm/errors.h"
#include "sysbvm/function.h"
#include "sysbvm/string.h"
#include "sysbvm/assert.h"
#include "sysbvm/backtrace.h"
#include "internal/context.h"
#include <stdio.h>

SYSBVM_API sysbvm_tuple_t sysbvm_io_readWholeFileNamedAsString(sysbvm_context_t *context, sysbvm_tuple_t filename)
{
    char *inputFileName = sysbvm_tuple_bytesToCString(filename);
#ifdef _WIN32
    FILE *inputFile = NULL;
    if(fopen_s(&inputFile, inputFileName, "rb"))
        return SYSBVM_NULL_TUPLE;
#else
    FILE *inputFile = fopen(inputFileName, "rb");
#endif
    if(!inputFile)
        return SYSBVM_NULL_TUPLE;
    sysbvm_tuple_bytesToCStringFree(inputFileName);

    fseek(inputFile, 0, SEEK_END);
    size_t fileSize = ftell(inputFile);
    fseek(inputFile, 0, SEEK_SET);

    sysbvm_tuple_t readString = sysbvm_string_createEmptyWithSize(context, fileSize);
    if(readString && fileSize > 0)
    {
        if(fread(SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(readString)->bytes, fileSize, 1, inputFile) != 1)
            readString = SYSBVM_NULL_TUPLE;
    }

    fclose(inputFile);

    return readString;
}

SYSBVM_API sysbvm_tuple_t sysbvm_io_readWholeFileNamedAsByteArray(sysbvm_context_t *context, sysbvm_tuple_t filename)
{
    char *inputFileName = sysbvm_tuple_bytesToCString(filename);
#ifdef _WIN32
    FILE *inputFile = NULL;
    if(fopen_s(&inputFile, inputFileName, "rb"))
        return SYSBVM_NULL_TUPLE;
#else
    FILE *inputFile = fopen(inputFileName, "rb");
#endif
    if(!inputFile)
        return SYSBVM_NULL_TUPLE;
    sysbvm_tuple_bytesToCStringFree(inputFileName);

    fseek(inputFile, 0, SEEK_END);
    size_t fileSize = ftell(inputFile);
    fseek(inputFile, 0, SEEK_SET);

    sysbvm_tuple_t readByteArray = sysbvm_byteArray_create(context, fileSize);
    if(readByteArray && fileSize > 0)
    {
        if(fread(SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(readByteArray)->bytes, fileSize, 1, inputFile) != 1)
            readByteArray = SYSBVM_NULL_TUPLE;
    }

    fclose(inputFile);

    return readByteArray;
}

SYSBVM_API bool sysbvm_io_writeWholeFileNamedWithByteArray(sysbvm_tuple_t filename, sysbvm_tuple_t content)
{
    char *outputFileName = sysbvm_tuple_bytesToCString(filename);
#ifdef _WIN32
    FILE *outputFile = NULL;
    if(fopen_s(&outputFile, outputFileName, "wb"))
        return false;
#else
    FILE *outputFile = fopen(outputFileName, "wb");
#endif
    if(!outputFile)
        return false;
    sysbvm_tuple_bytesToCStringFree(outputFileName);

    bool success = true;
    if(sysbvm_tuple_isBytes(content))
    {
        size_t contentSize = sysbvm_tuple_getSizeInBytes(content);
        success = fwrite(SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(content)->bytes, contentSize, 1, outputFile) == 1;
    }
    fclose(outputFile);

    return success;
}

static sysbvm_tuple_t sysbvm_io_primitive_printLine(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    size_t parameterCount = sysbvm_array_getSize(arguments[0]);
    for(size_t i = 0; i < parameterCount; ++i)
    {
        sysbvm_tuple_t string = sysbvm_tuple_asString(context, sysbvm_array_at(arguments[0], i));
        SYSBVM_ASSERT(sysbvm_tuple_isBytes(string));
        fwrite(SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(string)->bytes, sysbvm_tuple_getSizeInBytes(string), 1, stdout);

    }
    fwrite("\n", 1, 1, stdout);

    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_io_primitive_writeOntoStdout(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_tuple_t string = arguments[0];
    SYSBVM_ASSERT(sysbvm_tuple_isBytes(string));
    fwrite(SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(string)->bytes, sysbvm_tuple_getSizeInBytes(string), 1, stdout);

    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_io_primitive_writeOntoStderr(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_tuple_t string = arguments[0];
    SYSBVM_ASSERT(sysbvm_tuple_isBytes(string));
    fwrite(SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(string)->bytes, sysbvm_tuple_getSizeInBytes(string), 1, stdout);

    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_io_primitive_print(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    size_t parameterCount = sysbvm_array_getSize(arguments[0]);
    for(size_t i = 0; i < parameterCount; ++i)
    {
        sysbvm_tuple_t string = sysbvm_tuple_asString(context, sysbvm_array_at(arguments[0], i));
        SYSBVM_ASSERT(sysbvm_tuple_isBytes(string));
        fwrite(SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(string)->bytes, sysbvm_tuple_getSizeInBytes(string), 1, stdout);

    }

    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_io_primitive_readWholeFileNamedAsString(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_io_readWholeFileNamedAsString(context, arguments[0]);
}

static sysbvm_tuple_t sysbvm_io_primitive_readWholeFileNamedAsByteArray(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_io_readWholeFileNamedAsString(context, arguments[0]);
}

static sysbvm_tuple_t sysbvm_io_primitive_writeWholeFileNamedWithByteArray(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    return sysbvm_tuple_boolean_encode(sysbvm_io_writeWholeFileNamedWithByteArray(arguments[0], arguments[1]));
}

static sysbvm_tuple_t sysbvm_io_primitive_halt(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 0) sysbvm_error_argumentCountMismatch(0, argumentCount);

    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_io_primitive_haltPrintingBacktrace(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 0) sysbvm_error_argumentCountMismatch(0, argumentCount);

    sysbvm_backtrace_print();
    return SYSBVM_VOID_TUPLE;
}

void sysbvm_io_registerPrimitives(void)
{
    sysbvm_primitiveTable_registerFunction(sysbvm_io_primitive_printLine, "printLine");
    sysbvm_primitiveTable_registerFunction(sysbvm_io_primitive_print, "printLine");

    sysbvm_primitiveTable_registerFunction(sysbvm_io_primitive_writeOntoStdout, "IO::writeOntoStdout");
    sysbvm_primitiveTable_registerFunction(sysbvm_io_primitive_writeOntoStderr, "IO::writeOntoStderr");

    sysbvm_primitiveTable_registerFunction(sysbvm_io_primitive_readWholeFileNamedAsString, "IO::readWholeFileNamedAsString");
    sysbvm_primitiveTable_registerFunction(sysbvm_io_primitive_readWholeFileNamedAsByteArray, "IO::readWholeFileNamedAsByteArray");
    sysbvm_primitiveTable_registerFunction(sysbvm_io_primitive_writeWholeFileNamedWithByteArray, "IO::writeWholeFileNamedWithByteArray");
    sysbvm_primitiveTable_registerFunction(sysbvm_io_primitive_halt, "halt");
    sysbvm_primitiveTable_registerFunction(sysbvm_io_primitive_haltPrintingBacktrace, "haltPrintingBacktrace");
}

void sysbvm_io_setupPrimitives(sysbvm_context_t *context)
{
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "printLine", 1, SYSBVM_FUNCTION_FLAGS_VARIADIC, NULL, sysbvm_io_primitive_printLine);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "print", 1, SYSBVM_FUNCTION_FLAGS_VARIADIC, NULL, sysbvm_io_primitive_print);

    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.stringType, "writeOntoStdout", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_io_primitive_writeOntoStdout);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.stringType, "writeOntoStderr", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_io_primitive_writeOntoStderr);

    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "IO::readWholeFileNamedAsString", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_io_primitive_readWholeFileNamedAsString);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "IO::readWholeFileNamedAsByteArray", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_io_primitive_readWholeFileNamedAsByteArray);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "IO::writeWholeFileNamedWithByteArray", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_io_primitive_writeWholeFileNamedWithByteArray);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "IO::writeWholeFileNamedWithString", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_io_primitive_writeWholeFileNamedWithByteArray);

    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "halt", 0, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_io_primitive_halt);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "haltPrintingBacktrace", 0, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_io_primitive_haltPrintingBacktrace);
}
