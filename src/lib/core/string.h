/**
 * @file
 * String functions <string.h>
 *
 * Operations on bytes and strings in memory.
 */
#ifndef STRING_H
#define STRING_H

#include <stddef.h>

/** @name Character array manipulation */
/// @{
void *memcpy(void *dest, const void *src, size_t count);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
int   memcmp(const void *s1, const void *s2, size_t n);
/// @}

/** @name String manipulation */
/// @{
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t count);

char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t count);

int strcmp(const char *s, const char *t);
int strncmp(const char *s, const char *t, size_t n);

char *strerror(int errnum);
int   strerror_s(char *dst, size_t n, int errnum);
/// @}

/** @name String examination */
/// @{
size_t strlen(const char *s);
char  *strchr(const char *str, int ch);
char  *strstr(const char *str, const char *substr);
/// @}

#endif /* STRING_H */
