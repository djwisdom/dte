#ifndef UTIL_ALIGN_H
#define UTIL_ALIGN_H

#include <stddef.h> // size_t, max_align_t
#include <stdint.h> // intptr_t, intmax_t
#include "macros.h" // GNUC_AT_LEAST(), HAS_ATTRIBUTE(), MIN()

union MaxAlign {
    void *a;
    size_t b;
    long long c;
    intptr_t d;
    intmax_t e;
};

#if __STDC_VERSION__ >= 202311L
    #define ALIGNOF(t) alignof(t)
#elif __STDC_VERSION__ >= 201112L
    #define ALIGNOF(t) _Alignof(t)
#elif GNUC_AT_LEAST(3, 0)
    #define ALIGNOF(t) __alignof__(t)
#else
    #define ALIGNOF(t) MIN(sizeof(t), offsetof(struct{char c; t x;}, x))
#endif

// https://gcc.gnu.org/onlinedocs/gcc/Common-Attributes.html#index-aligned
// https://gcc.gnu.org/onlinedocs/gcc-3.0.4/gcc_5.html#SEC101
#if GNUC_AT_LEAST(3, 0) || HAS_ATTRIBUTE(aligned) || defined(__TINYC__)
    #define ALIGNED(alignment) __attribute__((__aligned__(alignment)))
    #define HAS_ATTR_ALIGNED 1
#else
    // Since this falls back to a no-op, it should only be used for
    // optimization purposes (not relied upon as a hard requirement)
    #define ALIGNED(alignment)
#endif

#if __STDC_VERSION__ >= 202311L
    #define ALIGNAS(type) alignas(type)
#elif __STDC_VERSION__ >= 201112L
    #define ALIGNAS(type) _Alignas(type)
#else // See the comment above ALIGNED()
    #define ALIGNAS(type) ALIGNED(ALIGNOF(type))
#endif

#if __STDC_VERSION__ >= 201112L
    // ISO C11 §6.7.5, §7.19
    #define MAXALIGN ALIGNAS(max_align_t)
#elif defined(HAS_ATTR_ALIGNED) && defined(__BIGGEST_ALIGNMENT__)
    // https://gcc.gnu.org/onlinedocs/gcc-4.4.7/gcc/Variable-Attributes.html#:~:text=__BIGGEST_ALIGNMENT__
    #define MAXALIGN ALIGNED(__BIGGEST_ALIGNMENT__)
#else
    #define MAXALIGN ALIGNAS(union MaxAlign)
#endif

#endif
