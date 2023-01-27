#ifndef TUUVM_GC_H
#define TUUVM_GC_H

#pragma once

#include "stackFrame.h"

/**
 * Schedules and attempts a garbage collection in this place and moment.
 */
TUUVM_API void tuuvm_gc_collect(tuuvm_context_t *context);

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

/**
 * Iterates through all of the roots in the system. 
 */
TUUVM_API void tuuvm_gc_iterateRoots(tuuvm_context_t *context, void *userdata, tuuvm_GCRootIterationFunction_t iterationFunction);

#endif //TUUVM_GC_H
