#ifndef SYSBVM_SOURCE_POSITION_H
#define SYSBVM_SOURCE_POSITION_H

#pragma once

#include "tuple.h"

typedef struct sysbvm_context_s sysbvm_context_t;

typedef struct sysbvm_sourcePosition_s
{
    sysbvm_tuple_header_t header;
    sysbvm_tuple_t sourceCode;

    sysbvm_tuple_t startIndex;
    sysbvm_tuple_t endIndex;
} sysbvm_sourcePosition_t;

/**
 * Creates a source position with the given source code and range.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_sourcePosition_create(sysbvm_context_t *context, sysbvm_tuple_t sourceCode, sysbvm_tuple_t startIndex, sysbvm_tuple_t endIndex);

/**
 * Creates a source position with the given source code and range specified as size_t.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_sourcePosition_createWithIndices(sysbvm_context_t *context, sysbvm_tuple_t sourceCode, size_t startIndex, size_t endIndex);

/**
 * Creates a source position with the union of the given source positions.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_sourcePosition_createWithUnion(sysbvm_context_t *context, sysbvm_tuple_t startSourcePosition, sysbvm_tuple_t endSourcePosition);

/**
 * Retrieves the source position start line and column.
 */
SYSBVM_API bool sysbvm_sourcePosition_getStartLineAndColumn(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, uint32_t *outLine, uint32_t *outColumn);

/**
 * Dumps a source position.
 */ 
SYSBVM_API void sysbvm_sourcePosition_dump(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition);


#endif //SYSBVM_SOURCE_POSITION_H
