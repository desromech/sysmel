#ifndef TUUVM_ERRORS_H
#define TUUVM_ERRORS_H

#include "common.h"
#include <stddef.h>

TUUVM_API void tuuvm_error(const char *message);
TUUVM_API void tuuvm_error_assertionFailure(const char *message);
TUUVM_API void tuuvm_error_indexOutOfBounds();
TUUVM_API void tuuvm_error_argumentCountMismatch(size_t expected, size_t gotten);

#endif //TUUVM_ERRORS_H