#ifndef SYSBVM_IO_H
#define SYSBVM_IO_H

#include "tuple.h"

/**
 * Reads a whole file with the specified name as a string.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_io_readWholeFileNamedAsString(sysbvm_context_t *context, sysbvm_tuple_t filename);

/**
 * Reads a whole file with the specified name as a byte array.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_io_readWholeFileNamedAsByteArray(sysbvm_context_t *context, sysbvm_tuple_t filename);

/**
 * Saves a whole file with the specified name.
 */
SYSBVM_API bool sysbvm_io_saveWholeFileNamed(sysbvm_tuple_t filename, sysbvm_tuple_t content);

#endif //SYSBVM_IO_H
