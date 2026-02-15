/**
 * @file
 * Formatted output <sprintf.h>
 *
 * Printf formatting for kernel.
 *
 * Includes only the string formatting functions.
 */
#ifndef KPRINTF_H
#define KPRINTF_H

#include <core/compiler.h>

#include <stdarg.h>
#include <stddef.h>

ATTR_PRINTFLIKE(2, 3)
int sprintf(char *s, const char *format, ...);
int vsprintf(char *s, const char *format, va_list args);

ATTR_PRINTFLIKE(3, 4)
int snprintf(char *s, size_t n, const char *format, ...);
int vsnprintf(char *s, size_t n, const char *format, va_list args);

#endif /* KPRINTF_H */
