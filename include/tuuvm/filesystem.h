#ifndef TUUVM_FILESYSTEM_H
#define TUUVM_FILESYSTEM_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

/**
 * Makes an absolute path.
 */
TUUVM_API tuuvm_tuple_t tuuvm_filesystem_absolute(tuuvm_context_t *context, tuuvm_tuple_t path);

/**
 * Is the path an absolute?
 */
TUUVM_API bool tuuvm_filesystem_isAbsolute(tuuvm_tuple_t path);

/**
 * Joins a path by adding a separator if needed.
 */
TUUVM_API tuuvm_tuple_t tuuvm_filesystem_joinPath(tuuvm_context_t *context, tuuvm_tuple_t left, tuuvm_tuple_t right);

/**
 * Extracts the directory name component of a path.
 */
TUUVM_API tuuvm_tuple_t tuuvm_filesystem_dirname(tuuvm_context_t *context, tuuvm_tuple_t path);

/**
 * Extracts the base name component of a path.
 */
TUUVM_API tuuvm_tuple_t tuuvm_filesystem_basename(tuuvm_context_t *context, tuuvm_tuple_t path);

/**
 * Extracts the extension component of a path.
 */
TUUVM_API tuuvm_tuple_t tuuvm_filesystem_extension(tuuvm_context_t *context, tuuvm_tuple_t path);

#endif //TUUVM_FILESYSTEM_H
