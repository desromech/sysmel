#include "tuuvm/assert.h"
#include "tuuvm/stackFrame.h"
#include "tuuvm/string.h"
#include <stdio.h>
#include <stdlib.h>

TUUVM_API void tuuvm_error(const char *message)
{
    tuuvm_context_t *activeContext = tuuvm_stackFrame_getActiveContext();
    if(!activeContext)
    {
        fprintf(stderr, "%s\n", message);
        abort();
    }

    tuuvm_tuple_t errorString = tuuvm_string_createWithCString(activeContext, message);
    tuuvm_stackFrame_raiseException(errorString);
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