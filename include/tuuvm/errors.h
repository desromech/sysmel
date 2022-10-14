#ifndef TUUVM_ERRORS_H
#define TUUVM_ERRORS_H

#include "common.h"

TUUVM_API void tuuvm_error(const char *message);
TUUVM_API void tuuvm_error_assertionFailure(const char *message);
TUUVM_API void tuuvm_error_indexOutOfBounds();

#endif //TUUVM_ERRORS_H