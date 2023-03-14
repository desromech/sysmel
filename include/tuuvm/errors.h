#ifndef TUUVM_ERRORS_H
#define TUUVM_ERRORS_H

#pragma once

#include "common.h"
#include <stddef.h>
#include <stdint.h>

typedef uintptr_t tuuvm_tuple_t;

TUUVM_API void tuuvm_errorWithMessageTuple(tuuvm_tuple_t message);

TUUVM_API void tuuvm_error(const char *message);
TUUVM_API void tuuvm_error_accessDummyValue();
TUUVM_API void tuuvm_error_assertionFailure(const char *message);
TUUVM_API void tuuvm_error_indexOutOfBounds();
TUUVM_API void tuuvm_error_modifyImmediateValue();
TUUVM_API void tuuvm_error_modifyImmutableTuple();
TUUVM_API void tuuvm_error_argumentCountMismatch(size_t expected, size_t gotten);

#endif //TUUVM_ERRORS_H