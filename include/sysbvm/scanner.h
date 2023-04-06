#ifndef SYSBVM_SCANNER_H
#define SYSBVM_SCANNER_H

#pragma once

#include "tuple.h"

typedef struct sysbvm_context_s sysbvm_context_t;

/**
 * Scans a source code into an array slice of tokens.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_scanner_scan(sysbvm_context_t *context, sysbvm_tuple_t sourceCode);

/**
 * Scans a source code c string
 */
SYSBVM_API sysbvm_tuple_t sysbvm_scanner_scanCString(sysbvm_context_t *context, const char *sourceCodeText, const char *sourceCodeName, const char *languageName);

#endif //SYSBVM_SCANNER_H
