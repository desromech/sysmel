#ifndef TUUVM_GC_H
#define TUUVM_GC_H

#pragma once

#include "stackFrame.h"

/**
 * A safepoint for the GC. This is a location where the Garbage Collection is allowed.
 */
TUUVM_API void tuuvm_gc_safepoint(tuuvm_context_t *context);

/**
 * Locks the GC so that it cannot be triggered. This increments a per-thread counter so that it can be called recursively.
 */
TUUVM_API void tuuvm_gc_lock(tuuvm_context_t *context);

/**
 * Unlocks the GC so that it can be triggered.
 * This decrements a per-thread counter so that it can be called recursively. At the end, this acts like a safepoint.
 */
TUUVM_API void tuuvm_gc_unlock(tuuvm_context_t *context);

#endif //TUUVM_GC_H
