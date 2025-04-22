/*-Copyright (c) Robert Patrick Rankin, 2017. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef CSTD_H
#define CSTD_H

/*
 * The list of standard (C99 unless noted otherwise) header files:
 *
 * <assert.h>	         Conditionally compiled macro that calls abort if its
 *                       argument evaluates to zero
 * <complex.h> (C99)     Complex number arithmetic
 * <ctype.h>	         Functions to categorize single characters
 * <errno.h>	         Macros reporting error conditions
 * <fenv.h> (C99)        Floating-point environment
 * <float.h>             Limits of floating-point types
 * <inttypes.h> (C99)    Format conversion of integer types
 * <iso646.h> (C95)      Alternative operator spellings
 * <limits.h>            Ranges of integer types
 * <locale.h>            Localization utilities
 * <math.h>              Common mathematics functions
 * <setjmp.h>            Nonlocal jumps
 * <signal.h>            Signal handling
 * <stdarg.h>            Variable arguments
 * <stdbool.h> (C99)     Macros for boolean type
 * <stddef.h>            Common macro definitions
 * <stdint.h> (C99)      Fixed-width integer types
 * <stdio.h>             Input/output program utilities
 * <stdlib.h>            General utilities: memory management,
 *                       program utilities, string conversions,
 *                       random numbers, algorithms
 * <string.h>            String handling
 * <tgmath.h> (C99)      Type-generic math (macros wrapping math.h and
 *                       complex.h)
 * <time.h>              Time/date utilities
 * <wchar.h> (C95)       Extended multibyte and wide character utilities
 * <wctype.h> (C95)      Functions to categorize single wide character

 * We watch these and try not to conflict with them, or make it tough to adopt
 * these in future:
 *
 * <stdalign.h> (C11)    alignas and alignof convenience macros
 * <stdatomic.h> (C11)   Atomic operations
 * <stdbit.h> (C23)      Macros to work with the byte and bit representations
 *                       of types
 * <stdckdint.h> (C23)   Macros for performing checked integer arithmetic
 * <stdnoreturn.h> (C11) noreturn convenience macro
 * <threads.h> (C11)     Thread library
 * <uchar.h> (C11)       UTF-16 and UTF-32 character utilities
 *
 */
#if !defined(__cplusplus)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>

#else /* !__cplusplus */
/* for FILE */
#include <stdio.h>
#endif /* !__cplusplus */
#endif /* CSTD_H */
