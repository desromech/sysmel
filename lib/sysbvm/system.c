#include "sysbvm/system.h"

SYSBVM_API const char *sysbvm_system_getArchitectureName(void)
{
#if defined(__i386__) || defined(_M_IX86)
    return "x86";
#elif defined(__x86_64__) || defined(_M_X64)
    return "x86_64";
#elif defined(__arm__) || defined(_M_ARM)
    return "arm";
#elif defined(__aarch64__)
    return "aarch64";
#else
    #error "Unsupported architecture."
#endif
}

SYSBVM_API const char *sysbvm_system_getVendorName(void)
{
#if defined(__APPLE__)
    return "apple";
#else
    return "";
#endif
}

SYSBVM_API const char *sysbvm_system_getOSName(void)
{
#if defined(_WIN32)
    return "windows";
#elif defined(__APPLE__)
    return "macos";
#elif defined(__linux__)
    return "linux";
#elif defined(unix)
    return "unix";
#else
    #error "Unknown platform";
#endif
}

SYSBVM_API const char *sysbvm_system_getAbiName(void)
{
#if defined(_WIN32)
    return "win32";
#elif defined(__APPLE__)
    return "darwin";
#elif defined(__linux__) || defined(unix)
    return "gnu";
#else
    #error "Unknown platform";
#endif
}

SYSBVM_API const char *sysbvm_system_getObjectFileName(void)
{
#if defined(_WIN32)
    return "coff";
#elif defined(__APPLE__)
    return "macho";
#else
    return "elf";
#endif
}

SYSBVM_API const char *sysbvm_system_getDebugInformationFormatName(void)
{
#if defined(_WIN32)
    return "codeview";
#else
    return "dwarf";
#endif
}

SYSBVM_API const char *sysbvm_system_getExceptionHandlingTableFormatName(void)
{
#if defined(_WIN32)
    return sizeof(void*) == 4 ? "" : "win64";
#else
    return "dwarf";
#endif
}
