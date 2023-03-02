#ifndef TUUVM_FLOAT_H
#define TUUVM_FLOAT_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

/**
 * Parses a float64 from a string.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_float32_parseString(tuuvm_context_t *context, size_t stringSize, const char *string);

/**
 * Parses a float64 from a C string.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_float32_parseCString(tuuvm_context_t *context, const char *cstring);

/**
 * Parses a float64 from a string.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_float64_parseString(tuuvm_context_t *context, size_t stringSize, const char *string);

/**
 * Parses a float64 from a C string.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_float64_parseCString(tuuvm_context_t *context, const char *cstring);

#endif //TUUVM_FLOAT_H
