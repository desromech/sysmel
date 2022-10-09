#ifndef TUUVM_SOURCE_POSITION_H
#define TUUVM_SOURCE_POSITION_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef struct tuuvm_sourcePosition_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t sourceCode;

    tuuvm_tuple_t startIndex;
    tuuvm_tuple_t startLine;
    tuuvm_tuple_t startColumn;

    tuuvm_tuple_t endIndex;
    tuuvm_tuple_t endLine;
    tuuvm_tuple_t endColumn;
} tuuvm_sourcePosition_t;

/**
 * Creates a source code with the given text and name.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_sourcePosition_create(tuuvm_context_t *context, tuuvm_tuple_t sourceCode, tuuvm_tuple_t startIndex, tuuvm_tuple_t endIndex);

/**
 * Creates a source code with the given text and name.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_sourcePosition_createWithIndices(tuuvm_context_t *context, tuuvm_tuple_t sourceCode, size_t startIndex, size_t endIndex);

#endif //TUUVM_SOURCE_POSITION_H
