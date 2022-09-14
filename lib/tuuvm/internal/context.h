#ifndef TUUVM_INTERNAL_CONTEXT_H
#define TUUVM_INTERNAL_CONTEXT_H

#pragma once

#include "tuuvm/context.h"
#include "heap.h"

struct tuvvm_context_s
{
    tuuvm_heap_t heap;
};

#endif //TUUVM_INTERNAL_CONTEXT_H
