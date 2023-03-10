#ifndef TUUVM_STRING_STREAM_H
#define TUUVM_STRING_STREAM_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef struct tuuvm_stringStream_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t size;
    tuuvm_tuple_t storage;
} tuuvm_stringStream_t;

/**
 * Creates a string stream
 */
TUUVM_API tuuvm_tuple_t tuuvm_stringStream_create(tuuvm_context_t *context);

/**
 * Adds character to the string stream
 */
TUUVM_API void tuuvm_stringStream_nextPut(tuuvm_context_t *context, tuuvm_tuple_t stringStream, uint8_t character);

/**
 * Adds a string to the string stream
 */
TUUVM_API void tuuvm_stringStream_nextPutString(tuuvm_context_t *context, tuuvm_tuple_t stringStream, tuuvm_tuple_t string);

/**
 * Adds a C string to the string stream
 */
TUUVM_API void tuuvm_stringStream_nextPutStringWithSize(tuuvm_context_t *context, tuuvm_tuple_t stringStream, size_t stringSize, const char *string);

/**
 * Adds a C string to the string stream
 */
TUUVM_API void tuuvm_stringStream_nextPutCString(tuuvm_context_t *context, tuuvm_tuple_t stringStream, const char *cstring);

/**
 * Construct the final string from the string stream.
 */
TUUVM_API tuuvm_tuple_t tuuvm_stringStream_asString(tuuvm_context_t *context, tuuvm_tuple_t stringStream);

/**
 * Construct the final string as a symbol from the string stream.
 */
TUUVM_API tuuvm_tuple_t tuuvm_stringStream_asSymbol(tuuvm_context_t *context, tuuvm_tuple_t stringStream);

/**
 * Gets the size of a string stream.
 */
TUUVM_API size_t tuuvm_stringStream_getSize(tuuvm_tuple_t stringStream);

/**
 * Gets the capacity a string stream.
 */
TUUVM_API size_t tuuvm_stringStream_getCapacity(tuuvm_tuple_t stringStream);

#endif //TUUVM_STRING_STREAM_H