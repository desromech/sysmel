#ifndef SYSBVM_STRING_STREAM_H
#define SYSBVM_STRING_STREAM_H

#pragma once

#include "tuple.h"

typedef struct sysbvm_context_s sysbvm_context_t;

typedef struct sysbvm_stringStream_s
{
    sysbvm_tuple_header_t header;
    sysbvm_tuple_t size;
    sysbvm_tuple_t storage;
} sysbvm_stringStream_t;

/**
 * Creates a string stream
 */
SYSBVM_API sysbvm_tuple_t sysbvm_stringStream_create(sysbvm_context_t *context);

/**
 * Adds character to the string stream
 */
SYSBVM_API void sysbvm_stringStream_nextPut(sysbvm_context_t *context, sysbvm_tuple_t stringStream, uint8_t character);

/**
 * Adds a string to the string stream
 */
SYSBVM_API void sysbvm_stringStream_nextPutString(sysbvm_context_t *context, sysbvm_tuple_t stringStream, sysbvm_tuple_t string);

/**
 * Adds a C string to the string stream
 */
SYSBVM_API void sysbvm_stringStream_nextPutStringWithSize(sysbvm_context_t *context, sysbvm_tuple_t stringStream, size_t stringSize, const char *string);

/**
 * Adds a C string to the string stream
 */
SYSBVM_API void sysbvm_stringStream_nextPutCString(sysbvm_context_t *context, sysbvm_tuple_t stringStream, const char *cstring);

/**
 * Construct a byte array with the contents from the string stream.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_stringStream_asByteArray(sysbvm_context_t *context, sysbvm_tuple_t stringStream);

/**
 * Construct the final string from the string stream.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_stringStream_asString(sysbvm_context_t *context, sysbvm_tuple_t stringStream);

/**
 * Construct the final string as a symbol from the string stream.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_stringStream_asSymbol(sysbvm_context_t *context, sysbvm_tuple_t stringStream);

/**
 * Gets the size of a string stream.
 */
SYSBVM_API size_t sysbvm_stringStream_getSize(sysbvm_tuple_t stringStream);

/**
 * Gets the capacity a string stream.
 */
SYSBVM_API size_t sysbvm_stringStream_getCapacity(sysbvm_tuple_t stringStream);

#endif //SYSBVM_STRING_STREAM_H