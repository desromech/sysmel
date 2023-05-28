#ifndef SYSBVM_BYTE_STREAM_H
#define SYSBVM_BYTE_STREAM_H

#pragma once

#include "tuple.h"

typedef struct sysbvm_context_s sysbvm_context_t;

typedef struct sysbvm_byteStream_s
{
    sysbvm_tuple_header_t header;
    sysbvm_tuple_t size;
    sysbvm_tuple_t storage;
} sysbvm_byteStream_t;

/**
 * Creates a byte stream
 */
SYSBVM_API sysbvm_tuple_t sysbvm_byteStream_create(sysbvm_context_t *context);

/**
 * Adds character to the byte stream
 */
SYSBVM_API void sysbvm_byteStream_nextPut(sysbvm_context_t *context, sysbvm_tuple_t byteStream, uint8_t character);

/**
 * Adds a byte to the byte stream
 */
SYSBVM_API void sysbvm_byteStream_nextPutString(sysbvm_context_t *context, sysbvm_tuple_t byteStream, sysbvm_tuple_t byte);

/**
 * Adds a C byte to the byte stream
 */
SYSBVM_API void sysbvm_byteStream_nextPutStringWithSize(sysbvm_context_t *context, sysbvm_tuple_t byteStream, size_t byteSize, const char *byte);

/**
 * Adds a C byte to the byte stream
 */
SYSBVM_API void sysbvm_byteStream_nextPutCString(sysbvm_context_t *context, sysbvm_tuple_t byteStream, const char *cbyte);

/**
 * Construct a byte array with the contents from the byte stream.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_byteStream_asByteArray(sysbvm_context_t *context, sysbvm_tuple_t byteStream);

/**
 * Construct the final byte from the byte stream.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_byteStream_asString(sysbvm_context_t *context, sysbvm_tuple_t byteStream);

/**
 * Construct the final byte as a symbol from the byte stream.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_byteStream_asSymbol(sysbvm_context_t *context, sysbvm_tuple_t byteStream);

/**
 * Gets the size of a byte stream.
 */
SYSBVM_API size_t sysbvm_byteStream_getSize(sysbvm_tuple_t byteStream);

/**
 * Gets the capacity a byte stream.
 */
SYSBVM_API size_t sysbvm_byteStream_getCapacity(sysbvm_tuple_t byteStream);

#endif //SYSBVM_BYTE_STREAM_H