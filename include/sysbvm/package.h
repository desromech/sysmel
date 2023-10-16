#ifndef SYSBVM_PACKAGE_H
#define SYSBVM_PACKAGE_H

#pragma once

#include "programEntity.h"

typedef struct sysbvm_package_s
{
    sysbvm_programEntity_t super;
    sysbvm_tuple_t children;
} sysbvm_package_t;

#endif //SYSBVM_PACKAGE_H
