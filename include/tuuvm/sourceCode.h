#ifndef TUUVM_SOURCE_CODE_H
#define TUUVM_SOURCE_CODE_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef struct tuuvm_sourceCode_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t text;
    tuuvm_tuple_t name;
    tuuvm_tuple_t lineStartIndexTable;
} tuuvm_sourceCode_t;

/**
 * Creates a source code with the given text and name.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_sourceCode_create(tuuvm_context_t *context, tuuvm_tuple_t text, tuuvm_tuple_t name);

/**
 * Creates a source code with the given text and name, provided as C strings.
 */
TUUVM_API tuuvm_tuple_t tuuvm_sourceCode_createWithCStrings(tuuvm_context_t *context, const char *text, const char *name);

/**
 * Gets the corresponding line and column for the specified index.
 */
TUUVM_API void tuuvm_sourceCode_computeLineAndColumnForIndex(tuuvm_context_t *context, tuuvm_tuple_t sourceCode, tuuvm_tuple_t index, tuuvm_tuple_t *outLine, tuuvm_tuple_t *outColumn);

#endif //TUUVM_SOURCE_CODE_H
