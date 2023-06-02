#ifndef _WIN32
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE // for realpath
#endif
#endif

#include "sysbvm/assert.h"
#include "sysbvm/filesystem.h"
#include "sysbvm/context.h"
#include "sysbvm/errors.h"
#include "sysbvm/function.h"
#include "sysbvm/string.h"
#include "internal/context.h"
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#else
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#endif

SYSBVM_API sysbvm_tuple_t sysbvm_filesystem_getWorkingDirectory(sysbvm_context_t *context)
{
#ifdef _WIN32
    DWORD requiredBufferSize = GetCurrentDirectoryW(0, NULL);
    wchar_t *buffer = (wchar_t *)malloc(requiredBufferSize*2);
    GetCurrentDirectoryW(requiredBufferSize, buffer);

    int resultStringBufferSize = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, NULL, 0, NULL, NULL);
    SYSBVM_ASSERT(resultStringBufferSize > 0);
    int resultStringSize = resultStringBufferSize - 1;

    sysbvm_tuple_t result = sysbvm_string_createEmptyWithSize(context, resultStringSize);
    WideCharToMultiByte(CP_UTF8, 0, buffer, -1, SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(result)->bytes, resultStringSize, NULL, NULL);
    free(buffer);

    return result;
#else
    char *buffer = malloc(PATH_MAX);
    char *result = getcwd(buffer, PATH_MAX);
    sysbvm_tuple_t resultTuple;
    if(result)
        resultTuple = sysbvm_string_createWithCString(context, result);
    else
        resultTuple = sysbvm_string_createEmptyWithSize(context, 0);
    free(buffer);
    return resultTuple;
#endif
}

SYSBVM_API void sysbvm_filesystem_setWorkingDirectory(sysbvm_tuple_t path)
{
#ifdef _WIN32
    char *pathCString = sysbvm_tuple_bytesToCString(path);
    int pathWStringSize = MultiByteToWideChar(CP_UTF8, 0, pathCString, -1, NULL, 0);
    wchar_t *pathWString = (wchar_t *)malloc(pathWStringSize*2);
    MultiByteToWideChar(CP_UTF8, 0, pathCString, -1, pathWString, pathWStringSize);
    BOOL result = SetCurrentDirectoryW(pathWString);
    free(pathWString);
    free(pathCString);
    if(!result)
        sysbvm_error("Failed to set working directory.");
#else
    char *pathCString = sysbvm_tuple_bytesToCString(path);
    int result = chdir(pathCString);
    free(pathCString);
    if(result)
        sysbvm_error("Failed to set working directory.");
#endif
}

SYSBVM_API sysbvm_tuple_t sysbvm_filesystem_absolute(sysbvm_context_t *context, sysbvm_tuple_t path)
{
    if(!sysbvm_tuple_isBytes(path) || sysbvm_filesystem_isAbsolute(path))
        return path;

#ifdef _WIN32
    return path;
#else
    char *pathCString = sysbvm_tuple_bytesToCString(path);
    char *absolutePath = realpath(pathCString, NULL);
    free(pathCString);
    if(!absolutePath)
        return sysbvm_filesystem_joinPath(context, sysbvm_filesystem_getWorkingDirectory(context), path);

    sysbvm_tuple_t result = sysbvm_string_createWithCString(context, absolutePath);
    free(absolutePath);
    return result;
#endif
}

SYSBVM_API bool sysbvm_filesystem_isAbsolute(sysbvm_tuple_t path)
{
    if(!sysbvm_tuple_isBytes(path))
        return false;

    size_t pathSize = sysbvm_tuple_getSizeInBytes(path);
    uint8_t *pathBytes = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(path)->bytes;

#ifdef _WIN32
    return pathSize >= 2 && pathBytes[1] == ':';
#else
    return pathSize >= 1 && pathBytes[0] == '/';
#endif
}

SYSBVM_API sysbvm_tuple_t sysbvm_filesystem_dirname(sysbvm_context_t *context, sysbvm_tuple_t path)
{
    if(!sysbvm_tuple_isBytes(path))
        return path;

    size_t pathSize = sysbvm_tuple_getSizeInBytes(path);
    uint8_t *pathBytes = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(path)->bytes;
    size_t dirnameEnd = 0;
    for(size_t i = 0; i < pathSize; ++i)
    {
        char separator = pathBytes[i];
        if(separator == '/')
            dirnameEnd = i + 1;
#ifdef _WIN32
        else if(separator == '\\')
            dirnameEnd = i + 1;
#endif
    }

    return sysbvm_string_createWithString(context, dirnameEnd, (const char*)pathBytes);
}

SYSBVM_API sysbvm_tuple_t sysbvm_filesystem_basename(sysbvm_context_t *context, sysbvm_tuple_t path)
{
    if(!sysbvm_tuple_isBytes(path))
        return path;

    size_t pathSize = sysbvm_tuple_getSizeInBytes(path);
    uint8_t *pathBytes = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(path)->bytes;
    size_t dirnameEnd = 0;
    for(size_t i = 0; i < pathSize; ++i)
    {
        char separator = pathBytes[i];
        if(separator == '/')
            dirnameEnd = i + 1;
#ifdef _WIN32
        else if(separator == '\\')
            dirnameEnd = i + 1;
#endif
    }

    return sysbvm_string_createWithString(context, pathSize - dirnameEnd, (const char*)(pathBytes + dirnameEnd));
}

SYSBVM_API sysbvm_tuple_t sysbvm_filesystem_extension(sysbvm_context_t *context, sysbvm_tuple_t path)
{
    if(!sysbvm_tuple_isBytes(path))
        return path;

    size_t pathSize = sysbvm_tuple_getSizeInBytes(path);
    uint8_t *pathBytes = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(path)->bytes;
    size_t extensionStart = pathSize;
    for(size_t i = 0; i < pathSize; ++i)
    {
        char separator = pathBytes[i];
        if(separator == '.')
            extensionStart = i;
    }

    return sysbvm_string_createWithString(context, pathSize - extensionStart, (const char*)(pathBytes + extensionStart));
}

SYSBVM_API sysbvm_tuple_t sysbvm_filesystem_joinPath(sysbvm_context_t *context, sysbvm_tuple_t left, sysbvm_tuple_t right)
{
    if(sysbvm_filesystem_isAbsolute(right) || !sysbvm_tuple_isBytes(left) || !sysbvm_tuple_isBytes(right))
        return right;

    size_t leftSize = sysbvm_tuple_getSizeInBytes(left);
    uint8_t *leftBytes = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(left)->bytes;

    size_t rightSize = sysbvm_tuple_getSizeInBytes(right);
    uint8_t *rightBytes = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(right)->bytes;

    bool hasSeparator = false;
    if(leftSize > 0)
    {
        uint8_t lastCharacter = leftBytes[leftSize - 1];
#ifdef _WIN32
        hasSeparator = lastCharacter == '/' || lastCharacter == '\\';
#else
        hasSeparator = lastCharacter == '/';
#endif
    }

    size_t resultSize = leftSize + (hasSeparator ? 0 : 1) + rightSize;
    sysbvm_tuple_t result = sysbvm_string_createEmptyWithSize(context, resultSize);

    size_t destIndex = 0;
    uint8_t *destBytes = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(result)->bytes;

    memcpy(destBytes, leftBytes, leftSize);
    destIndex += leftSize;

    if(!hasSeparator)
        destBytes[destIndex++] = '/';

    memcpy(destBytes + destIndex, rightBytes, rightSize);
    return result;
}

static sysbvm_tuple_t sysbvm_filesystem_primitive_getWorkingDirectory(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    (void)arguments;
    if(argumentCount != 0) sysbvm_error_argumentCountMismatch(0, argumentCount);

    return sysbvm_filesystem_getWorkingDirectory(context);
}

static sysbvm_tuple_t sysbvm_filesystem_primitive_setWorkingDirectory(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_filesystem_setWorkingDirectory(arguments[0]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_filesystem_primitive_absolute(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_filesystem_absolute(context, arguments[0]);
}

static sysbvm_tuple_t sysbvm_filesystem_primitive_isAbsolute(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_filesystem_isAbsolute(arguments[0]);
}

static sysbvm_tuple_t sysbvm_filesystem_primitive_dirname(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_filesystem_dirname(context, arguments[0]);
}

static sysbvm_tuple_t sysbvm_filesystem_primitive_basename(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_filesystem_basename(context, arguments[0]);
}

static sysbvm_tuple_t sysbvm_filesystem_primitive_extension(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_filesystem_extension(context, arguments[0]);
}

static sysbvm_tuple_t sysbvm_filesystem_primitive_joinPath(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    return sysbvm_filesystem_joinPath(context, arguments[0], arguments[1]);
}

void sysbvm_filesystem_registerPrimitives(void)
{
    sysbvm_primitiveTable_registerFunction(sysbvm_filesystem_primitive_getWorkingDirectory, "FileSystem::workingDirectory");
    sysbvm_primitiveTable_registerFunction(sysbvm_filesystem_primitive_setWorkingDirectory, "FileSystem::workingDirectory:");

    sysbvm_primitiveTable_registerFunction(sysbvm_filesystem_primitive_absolute, "FileSystem::absolute");
    sysbvm_primitiveTable_registerFunction(sysbvm_filesystem_primitive_isAbsolute, "FileSystem::isAbsolute");
    sysbvm_primitiveTable_registerFunction(sysbvm_filesystem_primitive_dirname, "FileSystem::dirname");
    sysbvm_primitiveTable_registerFunction(sysbvm_filesystem_primitive_basename, "FileSystem::basename");
    sysbvm_primitiveTable_registerFunction(sysbvm_filesystem_primitive_extension, "FileSystem::extension");
    sysbvm_primitiveTable_registerFunction(sysbvm_filesystem_primitive_joinPath, "FileSystem::joinPath:");
}

void sysbvm_filesystem_setupPrimitives(sysbvm_context_t *context)
{
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "FileSystem::workingDirectory", 0, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_filesystem_primitive_getWorkingDirectory);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "FileSystem::workingDirectory:", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_filesystem_primitive_setWorkingDirectory);
 
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "FileSystem::absolute", context->roots.stringType, "FileSystem::absolute", 1, SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_filesystem_primitive_absolute);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "FileSystem::isAbsolute", context->roots.stringType, "FileSystem::isAbsolute", 1, SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_filesystem_primitive_isAbsolute);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "FileSystem::dirname", context->roots.stringType, "FileSystem::dirname", 1, SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_filesystem_primitive_dirname);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "FileSystem::basename", context->roots.stringType, "FileSystem::basename", 1, SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_filesystem_primitive_basename);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "FileSystem::extension", context->roots.stringType, "FileSystem::extension", 1, SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_filesystem_primitive_extension);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "FileSystem::joinPath:", context->roots.stringType, "FileSystem::joinPath:", 2, SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_filesystem_primitive_joinPath);
}
