#ifndef SYSBVM_COMMON_H
#define SYSBVM_COMMON_H

#pragma once

// Extern C for using the public APIs in C++.
#ifdef __cplusplus
#   define SYSBVM_EXTERN_C extern "C"
#   define SYSBVM_INLINE inline
#else
#   define SYSBVM_EXTERN_C
#   define SYSBVM_INLINE static inline
#endif

// Export and import symbols
#ifdef BUILD_SYSBVM_STATIC
#define SYSBVM_EXPORT
#define SYSBVM_IMPORT
#else
#   ifdef _WIN32
#       define SYSBVM_EXPORT __declspec(dllexport)
#       define SYSBVM_IMPORT __declspec(dllimport)
#   else
#       if __GNUC__ >= 4
#           define SYSBVM_EXPORT __attribute__ ((visibility ("default")))
#           define SYSBVM_IMPORT __attribute__ ((visibility ("default")))
#       else
#           define SYSBVM_EXPORT
#           define SYSBVM_IMPORT
#       endif
#   endif
#endif

#ifdef BUILD_SYSBVM
#   define SYSBVM_API SYSBVM_EXTERN_C SYSBVM_EXPORT
#else
#   define SYSBVM_API SYSBVM_EXTERN_C SYSBVM_IMPORT
#endif

#if defined(_WIN32)
#define SYSBVM_THREAD_LOCAL __declspec(thread)
#elif defined(__GNUC__)
#define SYSBVM_THREAD_LOCAL __thread
#else
#error Add support for thread local storage.
#endif

#endif //SYSBVM_COMMON_H