#include "tuuvm/time.h"
#include "tuuvm/context.h"
#include "tuuvm/errors.h"
#include "tuuvm/function.h"
#include <stdio.h>

#ifdef _WIN32
#else
#   ifndef _XOPEN_SOURCE
#       define _XOPEN_SOURCE 600
#   endif
#include <time.h>
#endif

TUUVM_API int64_t tuuvm_time_microsecondsTimestamp(void)
{
#ifdef _WIN32
#error Implement this
#else
    struct timespec ts = {};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * (int64_t)1000000 + (int64_t)ts.tv_nsec / (int64_t)1000; 
#endif
}

TUUVM_API int64_t tuuvm_time_nanosecondsTimestamp(void)
{
#ifdef _WIN32
#error Implement this
#else
    struct timespec ts = {};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * (int64_t)1000000000 + (int64_t)ts.tv_nsec; 
#endif
}

static tuuvm_tuple_t tuuvm_time_timestamp_primitive_microsecondsNow(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 0) tuuvm_error_argumentCountMismatch(0, argumentCount);

    return tuuvm_tuple_int64_encode(context, tuuvm_time_microsecondsTimestamp());
}

static tuuvm_tuple_t tuuvm_time_timestamp_primitive_nanosecondsNow(tuuvm_context_t *context, tuuvm_tuple_t *closure, size_t argumentCount, tuuvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 0) tuuvm_error_argumentCountMismatch(0, argumentCount);

    return tuuvm_tuple_int64_encode(context, tuuvm_time_nanosecondsTimestamp());
}

void tuuvm_time_registerPrimitives(void)
{
    tuuvm_primitiveTable_registerFunction(tuuvm_time_timestamp_primitive_microsecondsNow, "Time::Timestamp::microsecondsNow");
    tuuvm_primitiveTable_registerFunction(tuuvm_time_timestamp_primitive_nanosecondsNow, "Time::Timestamp::nanosecondsNow");
}

void tuuvm_time_setupPrimitives(tuuvm_context_t *context)
{
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "Time::Timestamp::microsecondsNow", 0, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_time_timestamp_primitive_microsecondsNow);
    tuuvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "Time::Timestamp::nanosecondsNow", 0, TUUVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, tuuvm_time_timestamp_primitive_nanosecondsNow);
}
