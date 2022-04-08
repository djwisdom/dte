#ifndef UTIL_CHECKED_ARITH_H
#define UTIL_CHECKED_ARITH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "debug.h"
#include "macros.h"

// This is equivalent to `(x + 1) % modulus`, given the constraints
// imposed by BUG_ON(), but avoids expensive divisions by a non-constant
// and/or non-power-of-2
static inline size_t size_increment_wrapped(size_t x, size_t modulus)
{
    BUG_ON(modulus == 0);
    BUG_ON(x >= modulus);
    return (x + 1 < modulus) ? x + 1 : 0;
}

// As above, but equivalent to `x ? (x - 1) % modulus : modulus - 1`
static inline size_t size_decrement_wrapped(size_t x, size_t modulus)
{
    BUG_ON(modulus == 0);
    BUG_ON(x >= modulus);
    return (x ? x : modulus) - 1;
}

static inline bool size_add_overflows(size_t a, size_t b, size_t *result)
{
#if HAS_BUILTIN(__builtin_add_overflow) || GNUC_AT_LEAST(5, 0)
    return __builtin_add_overflow(a, b, result);
#else
    if (unlikely(b > SIZE_MAX - a)) {
        return true;
    }
    *result = a + b;
    return false;
#endif
}

static inline bool size_multiply_overflows(size_t a, size_t b, size_t *result)
{
#if HAS_BUILTIN(__builtin_mul_overflow) || GNUC_AT_LEAST(5, 0)
    return __builtin_mul_overflow(a, b, result);
#else
    if (unlikely(a > 0 && b > SIZE_MAX / a)) {
        return true;
    }
    *result = a * b;
    return false;
#endif
}

static inline bool umax_add_overflows(uintmax_t a, uintmax_t b, uintmax_t *result)
{
#if HAS_BUILTIN(__builtin_add_overflow) || GNUC_AT_LEAST(5, 0)
    return __builtin_add_overflow(a, b, result);
#else
    if (unlikely(b > UINTMAX_MAX - a)) {
        return true;
    }
    *result = a + b;
    return false;
#endif
}

static inline bool umax_multiply_overflows(uintmax_t a, uintmax_t b, uintmax_t *result)
{
#if HAS_BUILTIN(__builtin_mul_overflow) || GNUC_AT_LEAST(5, 0)
    return __builtin_mul_overflow(a, b, result);
#else
    if (unlikely(a > 0 && b > UINTMAX_MAX / a)) {
        return true;
    }
    *result = a * b;
    return false;
#endif
}

#endif