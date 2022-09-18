#include <stdio.h>
#include <tuuvm/context.h>

static tuuvm_context_t *context;

int main(int argc, const char *argv[])
{
    context = tuuvm_context_create();
    if(!context)
    {
        fprintf(stderr, "Failed to create tuuvm context.\n");
        return 1;
    }

    tuuvm_context_destroy(context);
    
    return 0;
}