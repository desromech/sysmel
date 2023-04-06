#ifndef SYSBVM_FLOAT_H
#define SYSBVM_FLOAT_H

#pragma once

#include "tuple.h"

typedef struct sysbvm_context_s sysbvm_context_t;

/**
 * Parses a float64 from a string.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_float32_parseString(sysbvm_context_t *context, size_t stringSize, const char *string);

/**
 * Parses a float64 from a C string.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_float32_parseCString(sysbvm_context_t *context, const char *cstring);

/**
 * Parses a float64 from a string.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_float64_parseString(sysbvm_context_t *context, size_t stringSize, const char *string);

/**
 * Parses a float64 from a C string.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_float64_parseCString(sysbvm_context_t *context, const char *cstring);

#endif //SYSBVM_FLOAT_H
