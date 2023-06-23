#ifndef SYSBVM_EXCEPTIONS_H
#define SYSBVM_EXCEPTIONS_H

#pragma once

#include "tuple.h"

typedef struct sysbvm_context_s sysbvm_context_t;

typedef struct sysbvm_exception_s
{
    sysbvm_tuple_header_t header;
    sysbvm_tuple_t messageText;
} sysbvm_exception_t;

typedef sysbvm_exception_t sysbvm_error_t;

/**
 * Signals the specified exception.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_exception_signal(sysbvm_context_t *context, sysbvm_tuple_t exception);

/**
 * Creates an error with the specified message text.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_error_createWithMessageText(sysbvm_context_t *context, sysbvm_tuple_t messageText);


#endif //SYSBVM_ARRAY_H