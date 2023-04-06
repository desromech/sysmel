#ifndef SYSBVM_TIME_H
#define SYSBVM_TIME_H

#pragma once

#include "tuple.h"

typedef struct sysbvm_context_s sysbvm_context_t;

/**
 * Gets a timestamp in microseconds from a monotonic clock.
 */ 
SYSBVM_API int64_t sysbvm_time_microsecondsTimestamp(void);

/**
 * Gets a timestamp in nanoseconds from a monotonic clock.
 */ 
SYSBVM_API int64_t sysbvm_time_nanosecondsTimestamp(void);

#endif //SYSBVM_TIME_H
