#ifndef SYSBVM_GC_H
#define SYSBVM_GC_H

#pragma once

#include "stackFrame.h"

/**
 * Schedules and attempts a garbage collection in this place and moment.
 */
SYSBVM_API void sysbvm_gc_collect(sysbvm_context_t *context);

/**
 * A safepoint for the GC. This is a location where the Garbage Collection is allowed.
 */
SYSBVM_API void sysbvm_gc_safepoint(sysbvm_context_t *context);

/**
 * Locks the GC so that it cannot be triggered. This increments a per-thread counter so that it can be called recursively.
 */
SYSBVM_API void sysbvm_gc_lock(sysbvm_context_t *context);

/**
 * Unlocks the GC so that it can be triggered.
 * This decrements a per-thread counter so that it can be called recursively. At the end, this acts like a safepoint.
 */
SYSBVM_API void sysbvm_gc_unlock(sysbvm_context_t *context);

/**
 * Iterates through all of the roots in the system. 
 */
SYSBVM_API void sysbvm_gc_iterateRoots(sysbvm_context_t *context, void *userdata, sysbvm_GCRootIterationFunction_t iterationFunction);

#endif //SYSBVM_GC_H
