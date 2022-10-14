#include "tuuvm/assert.h"
#include <stdio.h>
#include <stdlib.h>

TUUVM_API void tuuvm_error(const char *message)
{
    fprintf(stderr, "%s\n", message);
    abort();
}

TUUVM_API void tuuvm_error_assertionFailure(const char *message)
{
    tuuvm_error(message);
}

TUUVM_API void tuuvm_error_indexOutOfBounds()
{
    tuuvm_error("Index out of bounds");
}
