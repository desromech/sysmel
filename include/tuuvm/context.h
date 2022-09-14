#ifndef TUUVM_CONTEXT_H
#define TUUVM_CONTEXT_H

#pragma once

#include "common.h"

typedef struct tuvvm_context_s tuvvm_context_t;

/**
 * Creates a new context.
 */
TUUVM_API tuvvm_context_t *tuuvm_context_create(void);

/**
 * Creates a new context.
 */
TUUVM_API void tuuvm_context_destroy(tuvvm_context_t *context);

#endif //TUUVM_CONTEXT_H
