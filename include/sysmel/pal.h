#ifndef SYSMEL_PAL_H
#define SYSMEL_PAL_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
#define SYSMEL_PAL_EXTERN_C extern "C"
#else
#define SYSMEL_PAL_EXTERN_C
#endif

typedef struct sysmel_pal_file_s *sysmel_pal_filehandle_t;

SYSMEL_PAL_EXTERN_C void sysmel_pal_abort(void);

SYSMEL_PAL_EXTERN_C void* sysmel_pal_malloc(size_t size);
SYSMEL_PAL_EXTERN_C void sysmel_pal_free(void *pointer);

SYSMEL_PAL_EXTERN_C sysmel_pal_filehandle_t sysmel_pal_getStdinFileHandle(void);
SYSMEL_PAL_EXTERN_C sysmel_pal_filehandle_t sysmel_pal_getStdoutFileHandle(void);
SYSMEL_PAL_EXTERN_C sysmel_pal_filehandle_t sysmel_pal_getStderrFileHandle(void);

SYSMEL_PAL_EXTERN_C intptr_t sysmel_pal_writeToFile(sysmel_pal_filehandle_t handle, size_t size, const void *buffer);
SYSMEL_PAL_EXTERN_C intptr_t sysmel_pal_writeToFileAtOffset(sysmel_pal_filehandle_t handle, uint64_t offset, size_t size, const void *buffer);

SYSMEL_PAL_EXTERN_C intptr_t sysmel_pal_readFromFile(sysmel_pal_filehandle_t handle, size_t size, void *buffer);
SYSMEL_PAL_EXTERN_C intptr_t sysmel_pal_readFromFileAtOffset(sysmel_pal_filehandle_t handle, uint64_t offset, size_t size, void *buffer);

SYSMEL_PAL_EXTERN_C double sysmel_pal_parseFloat64(size_t size, const char *string);
SYSMEL_PAL_EXTERN_C char *sysmel_pal_float64ToString(double value);

SYSMEL_PAL_EXTERN_C int64_t sysmel_pal_microsecondsNow(void);
SYSMEL_PAL_EXTERN_C int64_t sysmel_pal_nanosecondsNow(void);

#endif //SYSMEL_PAL_H
