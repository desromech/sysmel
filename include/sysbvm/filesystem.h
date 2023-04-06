#ifndef SYSBVM_FILESYSTEM_H
#define SYSBVM_FILESYSTEM_H

#pragma once

#include "tuple.h"

typedef struct sysbvm_context_s sysbvm_context_t;

/**
 * Gets the current working directory.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_filesystem_getWorkingDirectory(sysbvm_context_t *context);

/**
 * Sets the current working directory.
 */
SYSBVM_API void sysbvm_filesystem_setWorkingDirectory(sysbvm_tuple_t path);

/**
 * Makes an absolute path.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_filesystem_absolute(sysbvm_context_t *context, sysbvm_tuple_t path);

/**
 * Is the path an absolute?
 */
SYSBVM_API bool sysbvm_filesystem_isAbsolute(sysbvm_tuple_t path);

/**
 * Joins a path by adding a separator if needed.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_filesystem_joinPath(sysbvm_context_t *context, sysbvm_tuple_t left, sysbvm_tuple_t right);

/**
 * Extracts the directory name component of a path.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_filesystem_dirname(sysbvm_context_t *context, sysbvm_tuple_t path);

/**
 * Extracts the base name component of a path.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_filesystem_basename(sysbvm_context_t *context, sysbvm_tuple_t path);

/**
 * Extracts the extension component of a path.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_filesystem_extension(sysbvm_context_t *context, sysbvm_tuple_t path);

#endif //SYSBVM_FILESYSTEM_H
