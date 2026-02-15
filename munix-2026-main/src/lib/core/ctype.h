/**
 * @file
 * ctype Character Handling <ctype.h>
 *
 * Character classification and transformation functions.
 *
 * @note
 *  These are very simple implementations
 *  that only consider the 7-bit ASCII character set.
 *
 * @see
 *  - CPP Reference: [ctype.h](https://en.cppreference.com/w/c/header/ctype)
 *  - The CCP Reference page for each of the `is*` functions has a table
 *      showing which characters belong to which classes.
 *      Try [isalpha](https://en.cppreference.com/w/c/string/byte/iscntrl.html)
 */
#ifndef CTYPE_H
#define CTYPE_H

static inline int isdigit(int c) { return '0' <= c && c <= '9'; }
static inline int islower(int c) { return 'a' <= c && c <= 'z'; }
static inline int isupper(int c) { return 'A' <= c && c <= 'Z'; }
static inline int isalpha(int c) { return isupper(c) || islower(c); }
static inline int isalnum(int c) { return isdigit(c) || isalpha(c); }
static inline int isprint(int c) { return 0x20 <= c && c <= 0x7e; }
static inline int isgraph(int c) { return 0x21 <= c && c <= 0x7e; }
static inline int ispunct(int c) { return isgraph(c) && !isalnum(c); }
static inline int iscntrl(int c) { return !isprint(c); }

static inline int isspace(int c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f'
           || c == '\v';
}

static inline int isxdigit(int c)
{
    return ('0' <= c && c <= '9') || ('A' <= c && c <= 'F')
           || ('a' <= c && c <= 'f');
}

static inline int tolower(int c) { return isupper(c) ? c - 'A' + 'a' : c; }
static inline int toupper(int c) { return islower(c) ? c - 'a' + 'A' : c; }

#endif /* CTYPE_H */
