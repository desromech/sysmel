#ifndef TUUVM_SCANNER_H
#define TUUVM_SCANNER_H

#pragma once

#include "tuple.h"

typedef struct tuuvm_context_s tuuvm_context_t;

/**
 * Scans a source code into an array slice of tokens.
 */ 
TUUVM_API tuuvm_tuple_t tuuvm_scanner_scan(tuuvm_context_t *context, tuuvm_tuple_t sourceCode);

/**
 * Scans a source code c string
 */
TUUVM_API tuuvm_tuple_t tuuvm_scanner_scanCString(tuuvm_context_t *context, const char *sourceCodeText, const char *sourceCodeName, const char *languageName);

#endif //TUUVM_SCANNER_H
