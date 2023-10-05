#ifndef SYSBVM_BACKTRACE_H
#define SYSBVM_BACKTRACE_H

#include "common.h"
#include <stddef.h>

SYSBVM_API int sysbvm_backtrace_obtain(void **returnAddresses, int maxNumberOfReturnAddress);
SYSBVM_API void sysbvm_backtrace_print(void);

#endif //SYSBVM_BACKTRACE_H
