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

TUUVM_API void tuuvm_error_argumentCountMismatch(size_t expected, size_t gotten)
{
    (void)expected;
    (void)gotten;
    tuuvm_error("Argument count mismatch");
}