#ifndef TUUVM_MACRO_CONTEXT_H
#define TUUVM_MACRO_CONTEXT_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

typedef struct tuuvm_macroContext_s
{
    tuuvm_tuple_header_t header;
    tuuvm_tuple_t sourceNode;
    tuuvm_tuple_t sourcePosition;
} tuuvm_macroContext_t;

TUUVM_API tuuvm_tuple_t tuuvm_macroContext_create(tuuvm_context_t *context, tuuvm_tuple_t sourceNode, tuuvm_tuple_t sourcePosition);

#endif //TUUVM_MACRO_CONTEXT_H
