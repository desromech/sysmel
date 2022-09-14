#include "internal/context.h"
#include <stdlib.h>

TUUVM_API tuvvm_context_t *tuuvm_context_create(void)
{
    tuvvm_context_t *context = (tuvvm_context_t*)calloc(1, sizeof(tuvvm_context_t));
    return context;
}

TUUVM_API void tuuvm_context_destroy(tuvvm_context_t *context)
{
    free(context);
}