#ifndef SYSBVM_SYSMEL_SYSTEM_H
#define SYSBVM_SYSMEL_SYSTEM_H

#pragma once

#include "common.h"

typedef struct sysbvm_context_s sysbvm_context_t;

/**
 * Gets the architecture name.
 */ 
SYSBVM_API const char *sysbvm_system_getArchitectureName(void);

/**
 * Gets the vendor name.
 */ 
SYSBVM_API const char *sysbvm_system_getVendorName(void);

/**
 * Gets the platform name.
 */ 
SYSBVM_API const char *sysbvm_system_getPlatformName(void);

/**
 * Gets the abi name.
 */ 
SYSBVM_API const char *sysbvm_system_getAbiName(void);

/**
 * Gets the object file name.
 */ 
SYSBVM_API const char *sysbvm_system_getObjectFileName(void);

/**
 * Gets the debug information format name.
 */ 
SYSBVM_API const char *sysbvm_system_getDebugInformationFormatName(void);

/**
 * Gets the exception handling table format name.
 */ 
SYSBVM_API const char *sysbvm_system_getExceptionHandlingTableFormatName(void);

#endif //SYSBVM_SYSMEL_SYSTEM_H
