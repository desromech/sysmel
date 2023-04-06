#include "sysbvm/time.h"
#include "sysbvm/context.h"
#include "sysbvm/errors.h"
#include "sysbvm/function.h"
#include <stdio.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 
#define NOMINMAX
#include <windows.h>
#else
#   ifndef _XOPEN_SOURCE
#       define _XOPEN_SOURCE 600
#   endif
#include <time.h>
#endif

SYSBVM_API int64_t sysbvm_time_microsecondsTimestamp(void)
{
#ifdef _WIN32
    LARGE_INTEGER timestamp = {0};
    LARGE_INTEGER timestampFrequency = {0};
    if(!QueryPerformanceCounter(&timestamp) || !QueryPerformanceFrequency(&timestampFrequency))
        return 0;

    int64_t frequencyDivisor = timestamp.QuadPart / (int64_t)1000000;
    return timestamp.QuadPart / frequencyDivisor;
#else
    struct timespec ts = {};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * (int64_t)1000000 + (int64_t)ts.tv_nsec / (int64_t)1000; 
#endif
}

SYSBVM_API int64_t sysbvm_time_nanosecondsTimestamp(void)
{
#ifdef _WIN32
    LARGE_INTEGER timestamp = {0};
    LARGE_INTEGER timestampFrequency = {0};
    if(!QueryPerformanceCounter(&timestamp) || !QueryPerformanceFrequency(&timestampFrequency))
        return 0;

    int64_t frequencyDivisor = timestamp.QuadPart / (int64_t)1000000000;
    return timestamp.QuadPart / frequencyDivisor;
#else
    struct timespec ts = {};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * (int64_t)1000000000 + (int64_t)ts.tv_nsec; 
#endif
}

static sysbvm_tuple_t sysbvm_time_timestamp_primitive_microsecondsNow(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 0) sysbvm_error_argumentCountMismatch(0, argumentCount);

    return sysbvm_tuple_int64_encode(context, sysbvm_time_microsecondsTimestamp());
}

static sysbvm_tuple_t sysbvm_time_timestamp_primitive_nanosecondsNow(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    (void)arguments;
    if(argumentCount != 0) sysbvm_error_argumentCountMismatch(0, argumentCount);

    return sysbvm_tuple_int64_encode(context, sysbvm_time_nanosecondsTimestamp());
}

void sysbvm_time_registerPrimitives(void)
{
    sysbvm_primitiveTable_registerFunction(sysbvm_time_timestamp_primitive_microsecondsNow, "Time::Timestamp::microsecondsNow");
    sysbvm_primitiveTable_registerFunction(sysbvm_time_timestamp_primitive_nanosecondsNow, "Time::Timestamp::nanosecondsNow");
}

void sysbvm_time_setupPrimitives(sysbvm_context_t *context)
{
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "Time::Timestamp::microsecondsNow", 0, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_time_timestamp_primitive_microsecondsNow);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "Time::Timestamp::nanosecondsNow", 0, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_time_timestamp_primitive_nanosecondsNow);
}
