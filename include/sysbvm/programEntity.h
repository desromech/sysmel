#ifndef SYSBVM_PROGRAM_ENTITY_H
#define SYSBVM_PROGRAM_ENTITY_H

#pragma once

#include "tuple.h"

typedef struct sysbvm_programEntity_s
{
    sysbvm_tuple_header_t header;
    sysbvm_tuple_t name;
    sysbvm_tuple_t owner;
    sysbvm_tuple_t serialToken;
} sysbvm_programEntity_t;

SYSBVM_API void sysbvm_programEntity_recordBindingWithOwnerAndName(sysbvm_context_t *context, sysbvm_tuple_t programEntity, sysbvm_tuple_t owner, sysbvm_tuple_t name);

#endif //SYSBVM_PROGRAM_ENTITY_H
