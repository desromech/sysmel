#ifndef TUUVM_COMMON_H
#define TUUVM_COMMON_H

#pragma once

// Extern C for using the public APIs in C++.
#ifdef __cplusplus
#   define TUUVM_EXTERN_C extern "C"
#   define TUUVM_INLINE inline
#else
#   define TUUVM_EXTERN_C
#   define TUUVM_INLINE static inline
#endif

// Export and import symbols
#ifdef _WIN32
#   define TUUVM_EXPORT __declspec(dllexport)
#   define TUUVM_IMPORT __declspec(dllimport)
#else
#   if __GNUC__ >= 4
#       define TUUVM_EXPORT __attribute__ ((visibility ("default")))
#       define TUUVM_IMPORT __attribute__ ((visibility ("default")))
#   else
#       define TUUVM_EXPORT
#       define TUUVM_IMPORT
#   endif
#endif

#ifdef BUILD_TUUVM
#   define TUUVM_API TUUVM_EXTERN_C TUUVM_EXPORT
#else
#   define TUUVM_API TUUVM_EXTERN_C TUUVM_IMPORT
#endif

#endif //TUUVM_COMMON_H