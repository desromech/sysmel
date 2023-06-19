#ifndef SYSMEL_PAL_H
#define SYSMEL_PAL_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
#define SYSMEL_PAL_EXTERN_C extern "C"
#else
#define SYSMEL_PAL_EXTERN_C
#endif

#define SYSMEl_PAL_FILE_OPEN_FLAGS_READ_ONLY (1<<0)
#define SYSMEl_PAL_FILE_OPEN_FLAGS_WRITE_ONLY (1<<1)
#define SYSMEl_PAL_FILE_OPEN_FLAGS_READ_WRITE (1<<2)

#define SYSMEl_PAL_FILE_OPEN_FLAGS_CREATE (1<<3)
#define SYSMEl_PAL_FILE_OPEN_FLAGS_TRUNCATE (1<<4)
#define SYSMEl_PAL_FILE_OPEN_FLAGS_APPEND (1<<5)
#define SYSMEl_PAL_FILE_OPEN_FLAGS_CLOSE_ON_EXEC (1<<6)

#define SYSMEl_PAL_SEEK_MODE_SET 0
#define SYSMEl_PAL_SEEK_MODE_CURRENT 1
#define SYSMEl_PAL_SEEK_MODE_END 2

typedef struct sysmel_pal_file_s *sysmel_pal_filehandle_t;

SYSMEL_PAL_EXTERN_C void sysmel_pal_abort(void);

SYSMEL_PAL_EXTERN_C void* sysmel_pal_malloc(size_t size);
SYSMEL_PAL_EXTERN_C void sysmel_pal_free(void *pointer);

SYSMEL_PAL_EXTERN_C sysmel_pal_filehandle_t sysmel_pal_getStdinFileHandle(void);
SYSMEL_PAL_EXTERN_C sysmel_pal_filehandle_t sysmel_pal_getStdoutFileHandle(void);
SYSMEL_PAL_EXTERN_C sysmel_pal_filehandle_t sysmel_pal_getStderrFileHandle(void);

SYSMEL_PAL_EXTERN_C sysmel_pal_filehandle_t sysmel_pal_openFile( size_t nameSize, const char *name, uint32_t openFlags, uint32_t creationPermissions);
SYSMEL_PAL_EXTERN_C void sysmel_pal_closeFile(sysmel_pal_filehandle_t handle);
SYSMEL_PAL_EXTERN_C bool sysmel_pal_isFileHandleValid(sysmel_pal_filehandle_t handle);

SYSMEL_PAL_EXTERN_C int64_t sysmel_pal_seek(sysmel_pal_filehandle_t handle, int64_t offset, int32_t mode);

SYSMEL_PAL_EXTERN_C intptr_t sysmel_pal_writeToFile(sysmel_pal_filehandle_t handle, size_t size, const void *buffer);
SYSMEL_PAL_EXTERN_C intptr_t sysmel_pal_writeToFileAtOffset(sysmel_pal_filehandle_t handle, uint64_t offset, size_t size, const void *buffer);

SYSMEL_PAL_EXTERN_C intptr_t sysmel_pal_readFromFile(sysmel_pal_filehandle_t handle, size_t size, void *buffer);
SYSMEL_PAL_EXTERN_C intptr_t sysmel_pal_readFromFileAtOffset(sysmel_pal_filehandle_t handle, uint64_t offset, size_t size, void *buffer);

SYSMEL_PAL_EXTERN_C double sysmel_pal_parseFloat64(size_t size, const char *string);
SYSMEL_PAL_EXTERN_C char *sysmel_pal_float64ToString(double value);

SYSMEL_PAL_EXTERN_C int64_t sysmel_pal_microsecondsNow(void);
SYSMEL_PAL_EXTERN_C int64_t sysmel_pal_nanosecondsNow(void);

#endif //SYSMEL_PAL_H
