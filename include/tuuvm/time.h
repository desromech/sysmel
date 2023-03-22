#ifndef TUUVM_TIME_H
#define TUUVM_TIME_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

/**
 * Gets a timestamp in microseconds from a monotonic clock.
 */ 
TUUVM_API int64_t tuuvm_time_microsecondsTimestamp(void);

/**
 * Gets a timestamp in nanoseconds from a monotonic clock.
 */ 
TUUVM_API int64_t tuuvm_time_nanosecondsTimestamp(void);

#endif //TUUVM_TIME_H
