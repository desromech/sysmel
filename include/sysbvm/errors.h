#ifndef SYSBVM_ERRORS_H
#define SYSBVM_ERRORS_H

#pragma once

#include "common.h"
#include <stddef.h>
#include <stdint.h>

typedef uintptr_t sysbvm_tuple_t;

SYSBVM_API void sysbvm_errorWithMessageTuple(sysbvm_tuple_t message);

SYSBVM_API void sysbvm_error(const char *message);
SYSBVM_API void sysbvm_error_accessDummyValue();
SYSBVM_API void sysbvm_error_assertionFailure(const char *message);
SYSBVM_API void sysbvm_error_fatalAssertionFailure(const char *message);
SYSBVM_API void sysbvm_error_indexOutOfBounds();
SYSBVM_API void sysbvm_error_modifyImmediateValue();
SYSBVM_API void sysbvm_error_modifyImmutableTuple();
SYSBVM_API void sysbvm_error_argumentCountMismatch(size_t expected, size_t gotten);
SYSBVM_API void sysbvm_error_outOfBoundsSlotAccess();
SYSBVM_API void sysbvm_error_trap();
SYSBVM_API void sysbvm_error_nullArgument();
SYSBVM_API void sysbvm_error_unexpectedType(sysbvm_tuple_t expectedType, sysbvm_tuple_t value);

#endif //SYSBVM_ERRORS_H