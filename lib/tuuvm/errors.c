#include "tuuvm/assert.h"
#include <stdio.h>
#include <stdlib.h>

void tuuvm_error_assertionFailure(const char *message)
{
    fprintf(stderr, "%s\n", message);
    abort();
}

void tuuvm_error_indexOutOfBounds()
{
    fprintf(stderr, "Index out of bounds\n");
    abort();
}
