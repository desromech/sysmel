#include "sysmel/pal.h"
#include <unistd.h>

SYSMEL_PAL_EXTERN_C sysmel_pal_filehandle_t sysmel_pal_getStdinFileHandle(void)
{
    return (sysmel_pal_filehandle_t)(uintptr_t)STDIN_FILENO;
}

SYSMEL_PAL_EXTERN_C sysmel_pal_filehandle_t sysmel_pal_getStdoutFileHandle(void)
{
    return (sysmel_pal_filehandle_t)(uintptr_t)STDOUT_FILENO;
}

SYSMEL_PAL_EXTERN_C sysmel_pal_filehandle_t sysmel_pal_getStderrFileHandle(void)
{
    return (sysmel_pal_filehandle_t)(uintptr_t)STDERR_FILENO;
}

SYSMEL_PAL_EXTERN_C intptr_t sysmel_pal_writeToFile(sysmel_pal_filehandle_t handle, size_t size, const void *buffer)
{
    return write((intptr_t)handle, buffer, size);
}

SYSMEL_PAL_EXTERN_C intptr_t sysmel_pal_writeToFileAtOffset(sysmel_pal_filehandle_t handle, uint64_t offset, size_t size, const void *buffer)
{
    return pwrite((intptr_t)handle, buffer, size, offset);
}

SYSMEL_PAL_EXTERN_C intptr_t sysmel_pal_readFromFile(sysmel_pal_filehandle_t handle, size_t size, void *buffer)
{
    return read((intptr_t)handle, buffer, size);
}

SYSMEL_PAL_EXTERN_C intptr_t sysmel_pal_readFromFileAtOffset(sysmel_pal_filehandle_t handle, uint64_t offset, size_t size, void *buffer)
{
    return pread((intptr_t)handle, buffer, size, offset);
}
