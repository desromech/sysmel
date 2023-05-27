#ifndef SYSMEL_SOFT_PRIMITIVES_H
#define SYSMEL_SOFT_PRIMITIVES_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
#define SYSMEL_SOFT_PRIMITIVE_EXTERN_C extern "C"
#else
#define SYSMEL_SOFT_PRIMITIVE_EXTERN_C
#endif

SYSMEL_SOFT_PRIMITIVE_EXTERN_C int64_t sysmel_softPrimitive_int64Negate(int64_t operand);
SYSMEL_SOFT_PRIMITIVE_EXTERN_C uint64_t sysmel_softPrimitive_int64Add(uint64_t left, uint64_t right);
SYSMEL_SOFT_PRIMITIVE_EXTERN_C uint64_t sysmel_softPrimitive_int64Sub(uint64_t left, uint64_t right);
SYSMEL_SOFT_PRIMITIVE_EXTERN_C uint64_t sysmel_softPrimitive_int64Mul(uint64_t left, uint64_t right);
SYSMEL_SOFT_PRIMITIVE_EXTERN_C uint64_t sysmel_softPrimitive_int64UDiv(uint64_t left, uint64_t right);
SYSMEL_SOFT_PRIMITIVE_EXTERN_C uint64_t sysmel_softPrimitive_int64URem(uint64_t left, uint64_t right);
SYSMEL_SOFT_PRIMITIVE_EXTERN_C int64_t sysmel_softPrimitive_int64SDiv(int64_t left, int64_t right);
SYSMEL_SOFT_PRIMITIVE_EXTERN_C int64_t sysmel_softPrimitive_int64SRem(int64_t left, int64_t right);

SYSMEL_SOFT_PRIMITIVE_EXTERN_C uint64_t sysmel_softPrimitive_int64BitInvert(uint64_t operand);
SYSMEL_SOFT_PRIMITIVE_EXTERN_C uint64_t sysmel_softPrimitive_int64BitAnd(uint64_t left, uint64_t right);
SYSMEL_SOFT_PRIMITIVE_EXTERN_C uint64_t sysmel_softPrimitive_int64BitOr(uint64_t left, uint64_t right);
SYSMEL_SOFT_PRIMITIVE_EXTERN_C uint64_t sysmel_softPrimitive_int64BitXor(uint64_t left, uint64_t right);
SYSMEL_SOFT_PRIMITIVE_EXTERN_C uint64_t sysmel_softPrimitive_int64LogicalShiftLeft(uint64_t left, uint64_t right);
SYSMEL_SOFT_PRIMITIVE_EXTERN_C uint64_t sysmel_softPrimitive_int64LogicalShiftRight(uint64_t left, uint64_t right);
SYSMEL_SOFT_PRIMITIVE_EXTERN_C int64_t sysmel_softPrimitive_int64ArithmeticShiftLeft(int64_t left, int64_t right);

SYSMEL_SOFT_PRIMITIVE_EXTERN_C bool sysmel_softPrimitive_int64Equals(uint64_t left, uint64_t right);
SYSMEL_SOFT_PRIMITIVE_EXTERN_C bool sysmel_softPrimitive_int64NotEquals(uint64_t left, uint64_t right);
SYSMEL_SOFT_PRIMITIVE_EXTERN_C bool sysmel_softPrimitive_int64LessThan(int64_t left, int64_t right);
SYSMEL_SOFT_PRIMITIVE_EXTERN_C bool sysmel_softPrimitive_int64LessOrEquals(int64_t left, int64_t right);
SYSMEL_SOFT_PRIMITIVE_EXTERN_C bool sysmel_softPrimitive_int64GreaterThan(int64_t left, int64_t right);
SYSMEL_SOFT_PRIMITIVE_EXTERN_C bool sysmel_softPrimitive_int64GreaterOrEquals(int64_t left, int64_t right);
SYSMEL_SOFT_PRIMITIVE_EXTERN_C bool sysmel_softPrimitive_uint64LessThan(uint64_t left, uint64_t right);
SYSMEL_SOFT_PRIMITIVE_EXTERN_C bool sysmel_softPrimitive_uint64LessOrEquals(uint64_t left, uint64_t right);
SYSMEL_SOFT_PRIMITIVE_EXTERN_C bool sysmel_softPrimitive_uint64GreaterThan(uint64_t left, uint64_t right);
SYSMEL_SOFT_PRIMITIVE_EXTERN_C bool sysmel_softPrimitive_uint64GreaterOrEquals(uint64_t left, uint64_t right);

#endif //SYSMEL_SOFT_PRIMITIVES_H
