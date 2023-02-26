#ifndef TUUVM_STRING_BUILDER_H
#define TUUVM_STRING_BUILDER_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef struct tuuvm_stringBuilder_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t size;
    tuuvm_tuple_t storage;
} tuuvm_stringBuilder_t;

/**
 * Creates a string builder
 */
TUUVM_API tuuvm_tuple_t tuuvm_stringBuilder_create(tuuvm_context_t *context);

/**
 * Adds character to the string builder
 */
TUUVM_API void tuuvm_stringBuilder_add(tuuvm_context_t *context, tuuvm_tuple_t stringBuilder, uint8_t character);

/**
 * Adds a string to the string builder
 */
TUUVM_API void tuuvm_stringBuilder_addString(tuuvm_context_t *context, tuuvm_tuple_t stringBuilder, tuuvm_tuple_t string);

/**
 * Adds a C string to the string builder
 */
TUUVM_API void tuuvm_stringBuilder_addStringWithSize(tuuvm_context_t *context, tuuvm_tuple_t stringBuilder, size_t stringSize, const char *string);

/**
 * Adds a C string to the string builder
 */
TUUVM_API void tuuvm_stringBuilder_addCString(tuuvm_context_t *context, tuuvm_tuple_t stringBuilder, const char *cstring);

/**
 * Construct the final string from the string builder.
 */
TUUVM_API tuuvm_tuple_t tuuvm_stringBuilder_asString(tuuvm_context_t *context, tuuvm_tuple_t stringBuilder);

/**
 * Gets the size of a string builder.
 */
TUUVM_API size_t tuuvm_stringBuilder_getSize(tuuvm_tuple_t stringBuilder);

/**
 * Gets the capacity a string builder.
 */
TUUVM_API size_t tuuvm_stringBuilder_getCapacity(tuuvm_tuple_t stringBuilder);

#endif //TUUVM_STRING_BUILDER_H