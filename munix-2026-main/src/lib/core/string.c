#include "string.h"

#include <stdint.h>

/**
 * Copy memory area (not overlap safe)
 */
void *memcpy(void *restrict dest, const void *restrict src, size_t count)
{
    const unsigned char *restrict bsrc = src;
    for (unsigned char *cur = dest; count > 0; count--, cur++, bsrc++)
        *cur = *(unsigned char *) bsrc;

    return dest;
}

/**
 * Copy memory area (overlap safe)
 */
void *memmove(void *dest, const void *src, size_t n)
{
    if (n == 0 || src == dest) return dest;

    if (src > dest) {
        /* If source is higher than dest, copy from low to high. */
        const unsigned char *s = src;
        unsigned char       *d = dest;
        for (; n; n--) *(d++) = *(s++);
    } else {
        /* If source is lower dest, copy from high to low. */
        const unsigned char *s = src + n;
        unsigned char       *d = dest + n;
        for (; n; n--) *--d = *--s;
    }
    return dest;
}

/**
 * Fill memory area
 */
void *memset(void *s, int c, size_t n)
{
    for (unsigned char *cur = s; n > 0; n--, cur++) *cur = (unsigned char) c;
    return s;
}

/**
 * Compare memory area
 */
int memcmp(const void *s1, const void *s2, size_t n)
{
    const unsigned char *b1 = s1;
    const unsigned char *b2 = s2;

    for (; n > 0; n--, b1++, b2++) {
        if (*b1 < *b2) return -1;
        if (*b1 > *b2) return 1;
    }
    return 0;
}

char *strcpy(char *restrict dest, const char *restrict src)
{
    char *destorig = dest;
    for (;; dest++, src++) {
        char copiedchar = *dest = *src;
        if (!copiedchar) break;
    }
    return destorig;
}

char *strncpy(char *restrict dest, const char *restrict src, size_t n)
{
    char *destorig = dest;
    for (; *src && n; dest++, src++, n--) *dest = *src; // Copy.
    for (; n; dest++, n--) *dest = 0;                   // Fill with zeroes.

    /* Note that we do not add a terminator if src is too long.
     *
     * C Standard: "If there is no null character in the first n characters of
     * the array pointed to by [src], the result will not be null-terminated."
     */

    /* Write nulls until the count is finished.
     *
     * C Standard: "If the array pointed to by [src] is a string that is
     * shorter than n characters, null characters are appended to the copy in
     * the array pointed to by [dest], until n characters in all have been
     * written."
     */
    for (; n; n--, dest++) {
        *dest = '\0';
    }
    return destorig;
}

char *strcat(char *dest, const char *src)
{
    char *destorig = dest;
    while (*dest) dest++;                     // Find end.
    for (; *src; dest++, src++) *dest = *src; // Copy src.
    *dest = 0;                                // Terminate.
    return destorig;
}

char *strncat(char *dest, const char *src, size_t n)
{
    char *destorig = dest;
    while (*dest) dest++;                               // Find end.
    for (; *src && n; dest++, src++, n--) *dest = *src; // Copy.
    *dest = 0;                                          // Terminate.
    return destorig;
}

int strcmp(const char *s, const char *t)
{
    for (;; s++, t++) {
        if (*s && *t && *s == *t) continue;
        if (*s < *t) return -1;
        if (*s == *t) return 0;
        if (*s > *t) return 1;
    }
    return 0;
}

/*
 * strncmp:
 * Compares at most n bytes from the strings s and t.
 * Returns >0 if s > t, <0 if s < t and 0 if s == t.
 */
int strncmp(const char *s, const char *t, size_t n)
{
    for (; n; s++, t++, n--) {
        if (*s && *t && *s == *t) continue;
        if (*s < *t) return -1;
        if (*s == *t) return 0;
        if (*s > *t) return 1;
    }
    return 0;
}

/** Get the length of a null-terminated string */
size_t strlen(const char *s)
{
    /* Note that there is no null-pointer check here.
     * This is dictated by the C Standard: "The behavior is undefined if str is
     * not a pointer to a null-terminated byte string." */
    size_t n;
    for (n = 0; *s != '\0'; s++) n++;
    return n;
}

char *strchr(const char *str, int ch)
{
    for (;; str++)
        if (*str == (char) ch) return (char *) str;
        else if (!*str) return NULL;
}

char *strstr(const char *str, const char *substr)
{
    int substrlen = strlen(substr);
    for (; *str; str++)
        if (strncmp(str, substr, substrlen) == 0) return (char *) str;
    return NULL;
}
