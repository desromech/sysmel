#include "sysbvm/backtrace.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#else
#include <execinfo.h>
#endif

SYSBVM_API int sysbvm_backtrace_obtain(void **returnAddresses, int maxNumberOfReturnAddress)
{
#ifdef _WIN32
    return 0;
#else
    return backtrace(returnAddresses, maxNumberOfReturnAddress);
#endif
}

SYSBVM_API char **sysbvm_backtrace_symbols(void **returnAddresses, int addressCount)
{
#ifdef _WIN32
    return calloc(sizeof(char*), addressCount);
#else
    return backtrace_symbols(returnAddresses, addressCount);
#endif
}

SYSBVM_API void sysbvm_backtrace_print(void)
{
    const int MaxBacktraceSize = 100;
    void *backtraceAddresses[MaxBacktraceSize];

    int backtraceSize = sysbvm_backtrace_obtain(backtraceAddresses, MaxBacktraceSize);
    if(backtraceSize < 0)
    {
        fprintf(stderr, "Failed to obtain a backtrace.");
        return;
    }

    char **symbols = sysbvm_backtrace_symbols(backtraceAddresses, backtraceSize);
    for(int i = 0; i < backtraceSize; ++i)
        printf("%s\n", symbols[i]);
    free(symbols);
}
