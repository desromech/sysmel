#ifndef TUUVM_IO_H
#define TUUVM_IO_H

#include "tuple.h"

/**
 * Reads a whole file with the specified name as a string.
 */
TUUVM_API tuuvm_tuple_t tuuvm_io_readWholeFileNamedAsString(tuuvm_context_t *context, tuuvm_tuple_t filename);

/**
 * Reads a whole file with the specified name as a byte array.
 */
TUUVM_API tuuvm_tuple_t tuuvm_io_readWholeFileNamedAsByteArray(tuuvm_context_t *context, tuuvm_tuple_t filename);

/**
 * Saves a whole file with the specified name.
 */
TUUVM_API bool tuuvm_io_saveWholeFileNamed(tuuvm_tuple_t filename, tuuvm_tuple_t content);

#endif //TUUVM_IO_H
