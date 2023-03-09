#ifndef _WIN32
#define _DEFAULT_SOURCE // for realpath
#endif

#include "tuuvm/filesystem.h"
#include "tuuvm/context.h"
#include "tuuvm/errors.h"
#include "tuuvm/function.h"
#include "tuuvm/string.h"
#include "internal/context.h"
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#else
#include <limits.h>
#include <stdlib.h>
#endif

TUUVM_API tuuvm_tuple_t tuuvm_filesystem_absolute(tuuvm_context_t *context, tuuvm_tuple_t path)
{
    if(!tuuvm_tuple_isBytes(path))
        return path;

#ifdef _WIN32
    return path;
#else
    char *pathCString = tuuvm_tuple_bytesToCString(path);
    char *absolutePath = realpath(pathCString, NULL);
    tuuvm_tuple_t result = tuuvm_string_createWithCString(context, absolutePath);
    free(absolutePath);
    free(pathCString);
    return result;
#endif
}

TUUVM_API bool tuuvm_filesystem_isAbsolute(tuuvm_tuple_t path)
{
    if(!tuuvm_tuple_isBytes(path))
        return false;

    size_t pathSize = tuuvm_tuple_getSizeInBytes(path);
    uint8_t *pathBytes = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(path)->bytes;

#ifdef _WIN32
    return pathSize >= 2 && pathBytes[1] == ':';
#else
    return pathSize >= 1 && pathBytes[0] == '/';
#endif
}

TUUVM_API tuuvm_tuple_t tuuvm_filesystem_dirname(tuuvm_context_t *context, tuuvm_tuple_t path)
{
    if(!tuuvm_tuple_isBytes(path))
        return path;

    size_t pathSize = tuuvm_tuple_getSizeInBytes(path);
    uint8_t *pathBytes = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(path)->bytes;
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

    return tuuvm_string_createWithString(context, dirnameEnd, (const char*)pathBytes);
}

TUUVM_API tuuvm_tuple_t tuuvm_filesystem_basename(tuuvm_context_t *context, tuuvm_tuple_t path)
{
    if(!tuuvm_tuple_isBytes(path))
        return path;

    size_t pathSize = tuuvm_tuple_getSizeInBytes(path);
    uint8_t *pathBytes = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(path)->bytes;
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

    return tuuvm_string_createWithString(context, pathSize - dirnameEnd, (const char*)(pathBytes + dirnameEnd));
}

TUUVM_API tuuvm_tuple_t tuuvm_filesystem_extension(tuuvm_context_t *context, tuuvm_tuple_t path)
{
    if(!tuuvm_tuple_isBytes(path))
        return path;

    size_t pathSize = tuuvm_tuple_getSizeInBytes(path);
    uint8_t *pathBytes = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(path)->bytes;
    size_t extensionStart = pathSize;
    for(size_t i = 0; i < pathSize; ++i)
    {
        char separator = pathBytes[i];
        if(separator == '.')
            extensionStart = i;
    }

    return tuuvm_string_createWithString(context, pathSize - extensionStart, (const char*)(pathBytes + extensionStart));
}

TUUVM_API tuuvm_tuple_t tuuvm_filesystem_joinPath(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right)
{
    if(tuuvm_filesystem_isAbsolute(right) || !tuuvm_tuple_isBytes(left) || !tuuvm_tuple_isBytes(right))
        return right;

    size_t leftSize = tuuvm_tuple_getSizeInBytes(left);
    uint8_t *leftBytes = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(left)->bytes;

    size_t rightSize = tuuvm_tuple_getSizeInBytes(right);
    uint8_t *rightBytes = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(right)->bytes;

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
    tuuvm_tuple_t result = tuuvm_string_createEmptyWithSize(context, resultSize);

    size_t destIndex = 0;
    uint8_t *destBytes = TUUVM_CAST_OOP_TO_OBJECT_TUPLE(result)->bytes;

    memcpy(destBytes, leftBytes, leftSize);
    destIndex += leftSize;

    if(!hasSeparator)
        destBytes[destIndex++] = '/';

    memcpy(destBytes + destIndex, rightBytes, rightSize);
    return result;
}

static tuuvm_tuple_t tuuvm_filesystem_primitive_absolute(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_filesystem_absolute(context, arguments[0]);
}

static tuuvm_tuple_t tuuvm_filesystem_primitive_isAbsolute(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_filesystem_isAbsolute(arguments[0]);
}

static tuuvm_tuple_t tuuvm_filesystem_primitive_dirname(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_filesystem_dirname(context, arguments[0]);
}

static tuuvm_tuple_t tuuvm_filesystem_primitive_basename(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_filesystem_basename(context, arguments[0]);
}

static tuuvm_tuple_t tuuvm_filesystem_primitive_extension(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 1) tuuvm_error_argumentCountMismatch(1, argumentCount);

    return tuuvm_filesystem_extension(context, arguments[0]);
}

static tuuvm_tuple_t tuuvm_filesystem_primitive_joinPath(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) tuuvm_error_argumentCountMismatch(2, argumentCount);

    return tuuvm_filesystem_joinPath(context, arguments[0], arguments[1]);
}

void tuuvm_filesystem_setupPrimitives(tuuvm_context_t *context)
{
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveMethod(context, "FileSystem::absolute", context->roots.stringType, "FileSystem::absolute", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_filesystem_primitive_absolute);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveMethod(context, "FileSystem::isAbsolute", context->roots.stringType, "FileSystem::isAbsolute", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_filesystem_primitive_isAbsolute);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveMethod(context, "FileSystem::dirname", context->roots.stringType, "FileSystem::dirname", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_filesystem_primitive_dirname);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveMethod(context, "FileSystem::basename", context->roots.stringType, "FileSystem::basename", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_filesystem_primitive_basename);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveMethod(context, "FileSystem::extension", context->roots.stringType, "FileSystem::extension", 1, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_filesystem_primitive_extension);
    tuuvm_context_setIntrinsicSymbolBindingWithPrimitiveMethod(context, "FileSystem::joinPath:", context->roots.stringType, "FileSystem::joinPath:", 2, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_filesystem_primitive_joinPath);
}
