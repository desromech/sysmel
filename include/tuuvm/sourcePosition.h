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
 * Creates a source position with the given source code and range.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_sourcePosition_create(tuuvm_context_t *context, tuuvm_tuple_t sourceCode, tuuvm_tuple_t startIndex, tuuvm_tuple_t endIndex);

/**
 * Creates a source position with the given source code and range specified as size_t.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_sourcePosition_createWithIndices(tuuvm_context_t *context, tuuvm_tuple_t sourceCode, size_t startIndex, size_t endIndex);

/**
 * Creates a source position with the union of the given source positions.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_sourcePosition_createWithUnion(tuuvm_context_t *context, tuuvm_tuple_t startSourcePosition, tuuvm_tuple_t endSourcePosition);


#endif //TUUVM_SOURCE_POSITION_H
