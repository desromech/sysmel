#include "tuuvm/gc.h"
#include <threads.h>

thread_local uint32_t tuuvm_gc_perThreadLockCount;

TUUVM_API void tuuvm_gc_safepoint(tuuvm_context_t *context)
{
    (void)context;
}

TUUVM_API void tuuvm_gc_lock(tuuvm_context_t *context)
{
    ++tuuvm_gc_perThreadLockCount;
    if(tuuvm_gc_perThreadLockCount != 1)
        return;
}

TUUVM_API void tuuvm_gc_unlock(tuuvm_context_t *context)
{
    --tuuvm_gc_perThreadLockCount;
    if(tuuvm_gc_perThreadLockCount != 0)
        return;
}