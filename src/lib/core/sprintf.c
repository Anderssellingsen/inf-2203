/**
 * @file
 * sprintf implementation, including all printf formatting logic
 */
#include "sprintf.h"

#include <core/compiler.h>
#include <core/ctype.h>
#include <core/errno.h>
#include <core/math.h>
#include <core/string.h>

#include <float.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** @name Settings for pointer format ("%p") */
///@{
#define PTR_PREFIX    "0x"                     ///< Prefix to use for pointers
#define PTR_WIDTH     (sizeof(void *) * 2 + 2) ///< Width to use for pointers
#define PTR_PRECISION (1) ///< Precision to use for pointers
///@}

/** @name Internal buffer sizes */
///@{
#define PREFIXBUFSZ 8   ///< Buffer size for number prefixes (sign, "0x", etc.)
#define FMTCPYSZ    32  ///< Buffer size for debug copy of format pattern
#define DIGITBUFSZ  128 ///< Buffer size for temporary digit buffers
///@}

/** @name Internal implemention flags */
///@{
#define CF_NONE        0x00 ///< No flags
#define CF_UP          0x01 ///< Use uppercase ("%#X" -> "0X0AF")
#define CF_OCTALPREFIX 0x02 ///< Add zero for octal prefix ("%#o" -> "0644")

///@}

#define WP_NONE (-1) ///< Value for width or precision that is unspecifieid

/** Parsed format pattern options: flags, width, precision, length modifier */
struct fmtspec {
    char fmtbuf[FMTCPYSZ]; ///< Copy of format string, for debugging.

    bool left;  ///< `-` flag: Left justify.
    bool plus;  ///< `+` flag: Show a plus sign for positive values.
    bool space; ///< ` ` flag: Add a placeholder space for positive values.
    bool alt;   ///< `#` flag: "Alternate", e.g. "0x" for hex.
    bool zero;  ///< `0` flag: Pad with zeros.

    int w; ///< Width
    int p; ///< Precision

    /** Length modifier, expressed as `sizeof` desired type. */
    size_t lenmodsz;
};

/**
 * @name Character and string output
 *
 * The 'snprintf' family writes to a bounded buffer, but they also calculate
 * the number of characters that _would be written_ if the buffer was large
 * enough.
 *
 * To calculate this, every character that would be written is sent through
 * these output functions. The character is only written if the position
 * is in bounds. But the position pointer is advanced either way.
 *
 * In the end, the formatted output is written as far as it will go,
 * and the character count can be found by comparing the ending position
 * to the start of the buffer.
 *
 * @param   s       Pointer to position in buffer
 * @param   t       Pointer to position one past end of buffer
 * @return  Updated s pointer
 */
///@{

/** Basic output: Output a single character */
static inline char *outchar(char *s, char *t, int c)
{
    if (s < t) *s = c; // Write to buffer, only if in bounds.
    return ++s;        // Advance buffer pointer either way.
}

/** Basic output: Output padding chars */
static char *outpad(char *s, char *t, int c, size_t len)
{
    for (; len; len--) s = outchar(s, t, c);
    return s;
}

/** Basic output: Output a string */
static char *outstr(char *s, char *t, const char *str)
{
    for (; *str; str++) s = outchar(s, t, *str);
    return s;
}

/**
 * Pad-to-width step: output a value with padding
 *
 * This is the final step in the format conversion: padding the output to
 * fit the field width. Earlier steps should convert numerical values to
 * strings, and then call this function to pad and output the value strings.
 *
 * @param   s       Pointer to position in buffer
 * @param   t       Pointer to position one past end of buffer
 * @param   spec    The parsed format specifier (need width and flags)
 * @param   prefix  String such as sign or "0x" prefix that should go
 *                  before any zero-padding, e.g. the "+" in "+00123"
 * @param   valstr  String value, e.g. the "123" in "+00123"
 */
static char *outpadded(
        char           *s,
        char           *t,
        struct fmtspec *spec,
        const char     *prefix,
        const char     *valstr
)
{
    /* Calculate lengths. */
    size_t prefixlen = strlen(prefix);
    size_t valstrlen = strlen(valstr);

    /* Calcualte padding. */
    size_t needwidth = prefixlen + valstrlen;
    size_t w         = spec->w != WP_NONE ? spec->w : 0;
    size_t padlen    = w > needwidth ? w - needwidth : 0;

    /* Output with padding.*/
    if (!spec->left && !spec->zero) s = outpad(s, t, ' ', padlen);
    s = outstr(s, t, prefix);
    if (!spec->left && spec->zero) s = outpad(s, t, '0', padlen);
    s = outstr(s, t, valstr);
    if (spec->left) s = outpad(s, t, ' ', padlen);

    return s;
}

///@}

/** @name Miscellaneous helper functions. */
///@{

/** Choose sign string ('-', '+', or ' ') based on value and spec */
static const char *sign_prefix(struct fmtspec *spec, bool isneg)
{
    if (isneg) return "-";
    if (spec->plus) return "+";
    if (spec->space) return " ";
    return "";
}

/** Convert a string to uppercase. */
static void strtoupper(char *s)
{
    for (; *s; s++) *s = toupper(*s);
}

///@}

/** @name Basic number conversions */
///@{

/** Get a single hex digit for a value from 0 to 15 */
static inline unsigned hexdigit(unsigned d)
{
    return d < 10 ? '0' + d : 'a' + (d - 0xa);
}

/** Integer to ASCII: decode an unsigned int into a digit string */
static char *
itoa(char *s, uintmax_t uval, unsigned base, unsigned precision, unsigned flags
)
{
    /* Decode digits into buffer
     * (in reverse: least significant digits first). */
    char  digitbuf[DIGITBUFSZ];
    char *d = digitbuf;
    for (; uval; uval /= base) {
        int digit = uval % base;
        *d++      = hexdigit(digit);
    }

    /* Precision: add leading zeros to meet precision. */
    char *p = digitbuf + precision;
    while (d < p) *d++ = '0';

    /* Octal prefix: add one leading zero if it does not already have one. */
    if (flags & CF_OCTALPREFIX && (d == digitbuf || *(d - 1) != '0'))
        *d++ = '0';

    /* Write reversed digits into buffer in correct order. */
    while (d > digitbuf) *s++ = *--d;

    /* Write a terminator. */
    *s = '\0';
    return s;
}

///@}

/** @name Paramter extraction */
///@{

/** Get a signed integer of descired size from argument list */
static uintmax_t va_arg_signed(size_t sz, va_list *args)
{
    if (!sz) return va_arg(*args, int);
    if (sz <= sizeof(char)) return (char) va_arg(*args, int);
    if (sz <= sizeof(short)) return (short) va_arg(*args, int);
    if (sz <= sizeof(long)) return va_arg(*args, long);
    if (sz <= sizeof(long long)) return va_arg(*args, long long);
    return va_arg(*args, intmax_t);
}

/** Get an unsigned integer of descired size from argument list */
static uintmax_t va_arg_unsigned(size_t sz, va_list *args)
{
    typedef unsigned char      uc;
    typedef unsigned short     us;
    typedef unsigned long      ul;
    typedef unsigned long long ull;

    if (!sz) return va_arg(*args, unsigned int);
    if (sz <= sizeof(uc)) return (uc) va_arg(*args, unsigned int);
    if (sz <= sizeof(us)) return (us) va_arg(*args, unsigned int);
    if (sz <= sizeof(ul)) return va_arg(*args, unsigned long);
    if (sz <= sizeof(ull)) return va_arg(*args, unsigned long long);
    return va_arg(*args, uintmax_t);
}

///@}

/** @name Formatting logic for each major conversion type */
///@{

/** Format a string value ("%s") */
static char *format_str(char *s, char *t, struct fmtspec *spec, va_list *args)
{
    /* Get string pointer from arguments. */
    char *str = va_arg(*args, char *);

    /* "%s" uses precision to truncate input. Create a truncated string. */
    if (spec->p != WP_NONE) {
        char pstr[spec->p + 1];
        strncpy(pstr, str, spec->p);
        pstr[spec->p] = '\0'; // Ensure there is a terminator.
        return outpadded(s, t, spec, "", pstr);
    }

    return outpadded(s, t, spec, "", str);
}

/** Format a character value ("%c") */
static char *format_char(char *s, char *t, struct fmtspec *spec, va_list *args)
{
    /* Get character from arguments and embed it in a string. */
    char c         = (unsigned char) va_arg(*args, int);
    char convbuf[] = {c, '\0'};
    return outpadded(s, t, spec, "", convbuf);
}

/** Format a signed integer ("%d" or "%i") */
static char *format_signed(
        char *s, char *t, struct fmtspec *spec, unsigned base, va_list *args
)
{
    /* Get the appropriate type from the arguments. */
    intmax_t val = va_arg_signed(spec->lenmodsz, args);

    /* Choose sign character. */
    const char *sign = sign_prefix(spec, val < 0);
    val              = val < 0 ? -val : val;

    /* Chcek precision. */
    if (spec->p == WP_NONE) spec->p = 1; // Default precision.
    else spec->zero = false; // If precision given, ignore zero flag.

    /* Format digits. */
    char digitbuf[DIGITBUFSZ];
    itoa(digitbuf, val, base, spec->p, CF_NONE);
    return outpadded(s, t, spec, sign, digitbuf);
}

/** Format an unsigned integer (u, o, x, X) */
static char *format_unsigned(
        char           *s,
        char           *n,
        struct fmtspec *spec,
        unsigned        base,
        unsigned        flags,
        va_list        *args
)
{
    /* Get the appropriate type from the arguments. */
    uintmax_t val = va_arg_unsigned(spec->lenmodsz, args);

    /* Choose prefix. */
    char prefixbuf[PREFIXBUFSZ] = {0};
    if (spec->alt && base == 8) flags |= CF_OCTALPREFIX;
    else if (spec->alt && base == 2 && val) strcpy(prefixbuf, "0b");
    else if (spec->alt && base == 16 && val) strcpy(prefixbuf, "0x");

    /* Check precision */
    if (spec->p == WP_NONE) spec->p = 1; // Default precision.
    else spec->zero = false; // If precision given, ignore zero flag.

    /* Format digits. */
    char digitbuf[DIGITBUFSZ];
    itoa(digitbuf, val, base, spec->p, flags);
    if (flags & CF_UP) strtoupper(prefixbuf), strtoupper(digitbuf);
    return outpadded(s, n, spec, prefixbuf, digitbuf);
}

/** Format a pointer ("%p") */
static char *
format_pointer(char *s, char *t, struct fmtspec *spec, va_list *args)
{
    /* Format like a prefixed unsigned hex number. */
    struct fmtspec uspec = {
            .alt      = 1,
            .p        = spec->p,
            .w        = spec->w,
            .lenmodsz = sizeof(void *),
    };
    return format_unsigned(s, t, &uspec, 16, CF_NONE, args);
}

///@}

/** @name Main formatting logic */
///@{

/** Parse and process a single conversion specifier ("%s", etc.) */
static char *
process_conversion(char *s, char *t, const char **fmt, va_list *args)
{
    struct fmtspec spec = {.w = WP_NONE, .p = WP_NONE};
    char          *f    = spec.fmtbuf;  // Copy for debugging
    int           *wp   = &spec.w;      // Width or precision, start on width
    bool           zero_is_flag = true; // First zero could be a flag

    *f++ = *(*fmt)++; // Consume initial '%' character.

    for (;;) {
        char ch = *f++ = *(*fmt)++; // Consume character.
        char nextchar  = **fmt;     // Peek at next character.

        /* Process consumed character. */
        switch (ch) {
        case '-': spec.left = true; break;
        case '+': spec.plus = true; break;
        case ' ': spec.space = true; break;
        case '#': spec.alt = true; break;

        case '0': /* Zero: May be a flag or a digit. */
            if (zero_is_flag) {
                spec.zero    = true;
                zero_is_flag = false;
                break;
            }
            FALLTHROUGH; // Fall through to digit logic.

        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': /* Digit: Add to width/precision. */
            if (*wp == WP_NONE) *wp = 0;
            *wp          = *wp * 10 + (ch - '0');
            zero_is_flag = false;
            break;

        case '*': /* Star: Get width/precision from args. */
            *wp = va_arg(*args, int);

            /* If width arg is negative, left justify and use abs. val. */
            if (wp == &spec.w && *wp < 0) spec.left = true, spec.w = -*wp;

            /* If precision arg is negative, Treat it as unspecified. */
            if (wp == &spec.p && *wp < 0) spec.p = WP_NONE;
            break;

        case '.': /* Dot: Switch from parsing width to parsing precision. */
            wp           = &spec.p;
            spec.p       = 0; // Dot with no digits counts as 0.
            zero_is_flag = false;
            break;

        case 'h': /* Length modifiler: "h" = short, "hh" = char */
            if (nextchar == 'h') spec.lenmodsz = sizeof(char), ++*fmt;
            else spec.lenmodsz = sizeof(short);
            break;

        case 'l': /* Length modifiler: "l" = long, "ll" = long long */
            if (nextchar == 'l') spec.lenmodsz = sizeof(long long), ++*fmt;
            else spec.lenmodsz = sizeof(long);
            break;

        case 'j': spec.lenmodsz = sizeof(intmax_t); break;
        case 'z': spec.lenmodsz = sizeof(size_t); break;
        case 't': spec.lenmodsz = sizeof(ptrdiff_t); break;
        case 'L': spec.lenmodsz = sizeof(long double); break;

        case '%': return outchar(s, t, '%'); // Literal '%'.
        case 's': return format_str(s, t, &spec, args);
        case 'c': return format_char(s, t, &spec, args);

        case 'd':
        case 'i': return format_signed(s, t, &spec, 10, args);
        case 'u': return format_unsigned(s, t, &spec, 10, CF_NONE, args);
        case 'o': return format_unsigned(s, t, &spec, 8, CF_NONE, args);
        case 'x': return format_unsigned(s, t, &spec, 16, CF_NONE, args);
        case 'X': return format_unsigned(s, t, &spec, 16, CF_UP, args);
        case 'b': return format_unsigned(s, t, &spec, 2, CF_NONE, args);
        case 'B': return format_unsigned(s, t, &spec, 2, CF_UP, args);
        case 'p': return format_pointer(s, t, &spec, args);

        default: return NULL; // Unknown or unsupported conversion char
        }
    }
}

/** Inner sprintf implementation */
static int snprintf_impl(char *s, size_t n, const char *fmt, va_list *args)
{
    char *bufstart = s;                  // Save start of buffer for later.
    char *t        = s + n;              // Calculate end of buffer.
    if (t < s) t = (char *) UINTPTR_MAX; // Avoid wraparound.

    /* Main loop: process characters in format string. */
    for (;;) {
        /* Copy non-pattern characters directly. */
        for (; *fmt && *fmt != '%'; fmt++) s = outchar(s, t, *fmt);
        if (!*fmt) break;

        /* Parse and interpret encountered pattern. */
        s = process_conversion(s, t, &fmt, args);
        if (!s) return -ENOTSUP; // Report error.
    }

    /* Write a terminator. */
    if (n) {
        if (s < t) *s = '\0'; // Either end of string,
        else *(t - 1) = '\0'; // or at end of buffer.
    }

    return s - bufstart;
}

///@}

/** @name Public sprintf interfaces */
///@{

/**
 * Print to bounded string buffer, with args from va_list.
 *
 * @callergraph
 */
int vsnprintf(char *s, size_t n, const char *format, va_list args)
{
    va_list argscopy;
    va_copy(argscopy, args); // Copy so we can get a pointer.
    int res = snprintf_impl(s, n, format, &argscopy);
    va_end(argscopy);
    return res;
}

/**
 * Print to bounded string buffer.
 *
 * @callgraph
 */
ATTR_PRINTFLIKE(3, 4)
int snprintf(char *s, size_t n, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int res = snprintf_impl(s, n, format, &args);
    va_end(args);
    return res;
}

/**
 * Print to string buffer, with args from va_list
 *
 * @callergraph
 * @callgraph
 */
int vsprintf(char *s, const char *format, va_list args)
{
    return vsnprintf(s, SIZE_MAX, format, args);
}

/**
 * Print to string buffer.
 *
 * @callgraph
 */
ATTR_PRINTFLIKE(2, 3)
int sprintf(char *s, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int res = vsprintf(s, format, args);
    va_end(args);
    return res;
}

///@}
