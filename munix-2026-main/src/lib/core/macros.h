/**
 * @file
 * Useful macros <macros.h>
 *
 * Miscellaneous short macros such as MIN and MAX.
 */
#ifndef MACROS_H
#define MACROS_H

#include <core/compiler.h>

/** @name Incomplete code */
///@{

/** Suppress warnings for unused variables. */
#define UNUSED(x) ((void) x)

///@}

/** @name Min and max */
///@{
#define MIN(x, y) ((x) <= (y) ? (x) : (y)) ///< Pick the lesser of two values.
#define MAX(x, y) ((x) >= (y) ? (x) : (y)) ///< Pick the greater of two values.
///@}

/** @name Bit flag checks */
///@{
#define FLAGS_ALL(val, flags)  (((val) & (flags)) == (flags))
#define FLAGS_NONE(val, flags) (!((val) & (flags)))
///@}

/** @name Power-of-two alignment for integers (pointers and sizes) */
///@{

/**
 * Test if an integer falls on a power-of-two alignment
 *
 * @param   x   integer to check; pointers must be cast to uintptr_t
 * @param   a   alignment; must be a power of two
 *
 * Because the alignment is a power of two,
 * we can optimize with bit twiddling:
 *
 *  1. Subtract 1 from 'a' to get a bit mask, e.g. 0x1000 - 1 == 0x0fff.
 *  2. Bitwise-AND the bit mask with the integer.
 *  3. Logical complement to get a boolean.
 *
 *  For example:
 *
 *  - Not aligned:  0x4321 & 0x0fff == 0x0321; !0x0321 == false
 *  - Aligned:      0x4000 & 0x0fff == 0x0000; !0x0000 == true
 */
#define IS_ALIGNED(x, a) (!((x) & ((a) -1)))

/**
 * Round an integer down to a power-of-two alignment
 *
 * @param   x   integer to round; pointers must be cast to uintptr_t
 * @param   a   alignment; must be a power of two
 *
 * Because the alignment is a power of two,
 * we can optimize with bit twiddling:
 *
 *  1. Subtract 1 from 'a' to get a bit mask, e.g. 0x1000 - 1 == 0x0fff.
 *  2. Bitwise-AND the complement of the bit mask with the integer.
 *      e.g. 0x4321 & ~0x0fff == 0x4321 & 0xf000 == 0x4000
 *
 *  For example:
 *
 *  - Not aligned:  0x4321 & ~0x0fff == 0x4000
 *  - Aligned:      0x4000 & ~0x0fff == 0x4000
 */
#define ALIGN_DOWN(x, a) ((x) & ~((a) -1))

/**
 * Round an integer up to a power-of-two alignment
 *
 * @param   x   integer to round; pointers must be cast to uintptr_t
 * @param   a   alignment; must be a power of two
 *
 * This works by first adding (align-1) to the integer and then rounding down
 * as in @ref ALIGN_DOWN :
 *
 *  - e.g. Not aligned: 0x4321 + 0x0fff == 0x5320; 0x5320 & ~0x0fff == 0x5000
 *  - e.g. Aligned:     0x4000 + 0x0fff == 0x4fff; 0x4fff & ~0x0fff == 0x4000
 */
#define ALIGN_UP(x, a) (((x) + ((a) -1)) & ~((a) -1))

///@}

/** @name Arrays and structs */
///@{

/**
 * Calculate the size of a static array.
 */
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/**
 * Calculate remaining buffer size given a position and end pointer
 *
 * Given a position pointer and an end pointer (start + size),
 * this macro gives the size remaining in the buffer. If
 * the position is beyond the end of the buffer, the result
 * will be zero, rather than a negative value.
 *
 * This is useful for calculating the 'n' value for repeated calls to
 * @ref snprintf. For example:
 *
 * ```c
 *      char buf[BUFSZ], *pos = buf, *end = buf + BUFSZ;
 *      pos += snprintf(pos, BUFREM(pos, end), "some nubers:");
 *      for (int i = 0; i < 5; i++)
 *          pos += snprintf(pos, BUFREM(pos, end), " %d", i);
 *      pos += snprintf(pos, BUFREM(pos, end), "\n");
 * ```
 */
#define BUFREM(pos, end) ((pos) < (end) ? (end) - (pos) : 0)

/**
 * Get a pointer to a containing struct from a pointer to a field.
 *
 * This macro is for when you have a pointer to a field inside a struct,
 * but you need a pointer to the struct itself.
 *
 * For example. we can put structs in a linked list by giving them a @ref
 * list_head field that holds `next` and `prev` pointers. These pointers point
 * to the `list_head` fields inside the other items. We traverse the list via
 * pointers to `list_head`, and then we can use this macro to get a pointer to
 * the item itself.
 *
        container_of(ptr) ----> +-------+   +-------+   +-------+
                                | item  |   | item  |   | item  |
                                +-------+   +-------+   +-------+
                     ptr -----> | p | n |<->| p | n |<->| p | n |
                                +-------+   +-------+   +-------+
 *
 * @note
 *      This is a risky operation. If the pointer is not actually inside
 *      of the expected struct, the result will be @ref undefined_behavior.
 *
 * @param   ptr     Pointer to the field inside of the struct.
 * @param   type    The type of the containing struct.
 * @param   member  The name of the field that _ptr_ is pointing to.
 * @returns         A new pointer, of type _type_, adjusted according to
 *                  the offset of the _member_ field within the _type_.
 *                  If _ptr_ is NULL, then this macro also returns NULL.
 *                  It does not attempt to adjust a NULL pointer.
 *
 * This was adapted from Linux's `container_of` macro
 * ([include/linux/container_of.h](https://elixir.bootlin.com/linux/v6.15.3/source/include/linux/container_of.h#L10)).
 * Unlike the Linux macro, this one handles NULL pointers.
 *
 * This macro relies on some GCC extensions to C's syntax and semantics:
 *
 * - [Statements and Declarations in Expressions](
        https://gcc.gnu.org/onlinedocs/gcc/Statement-Exprs.html)
 * - [Arithmetic on `void`- and Function Pointers](
        https://gcc.gnu.org/onlinedocs/gcc/Pointer-Arith.html)
 */
#define container_of(ptr, type, member) \
    ({ \
        /* Evaluate ptr once and save it. \
         * This avoids accidentally evaluating ptr multiple times, \
         * which would cause problems if, say, ptr was a function \
         * call with side-effects. */ \
        void *_mptr = (ptr); \
        /* Sanity check: Do the pointer and field have the same type. */ \
        _Static_assert( \
                TYPES_COMPATIBLE( \
                        typeof(*(ptr)), typeof(((type *) 0)->member) \
                ), \
                "type of " #ptr " does not match type of " #type "." #member \
        ); \
        /* Adjust pointer by field offset (if not null). */ \
        void *_adjusted = _mptr ? _mptr - offsetof(type, member) : NULL; \
        /* Finally, cast the adjusted pointer to the desired type. */ \
        (type *) _adjusted; \
    })

///@}

#endif /* MACROS_H */
