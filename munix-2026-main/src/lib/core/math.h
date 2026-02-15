/**
 * @file
 * Floating-point math utils <math.h>
 *
 * Floating-point math utils.
 */
#ifndef MATH_H
#define MATH_H

/* Many math functions are provided as builtins by GCC.
 * We can simply defer to them. */

#define INFINITY (__builtin_inff())
#define NAN      (__builtin_nanf(""))
#define HUGE_VAL (__builtin_huge_val())
#define isinf    __builtin_isinf
#define isnan    __builtin_isnan
#define signbit  __builtin_signbit
#define fabsl    __builtin_fabsl

#endif /* MATH_H */
