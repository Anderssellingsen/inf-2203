#include <core/compiler.h>

#include <stdarg.h>
#include <stddef.h>

struct _file {
    int fd;
};

typedef struct _file FILE;

extern FILE *stdin, *stdout, *stderr;

ATTR_PRINTFLIKE(2, 3)
int sprintf(char *s, const char *format, ...);
int vsprintf(char *s, const char *format, va_list args);

ATTR_PRINTFLIKE(3, 4)
int snprintf(char *s, size_t n, const char *format, ...);
int vsnprintf(char *s, size_t n, const char *format, va_list args);

ATTR_PRINTFLIKE(2, 3)
int fprintf(FILE *f, const char *format, ...);
int vfprintf(FILE *f, const char *format, va_list args);

ATTR_PRINTFLIKE(1, 2)
int printf(const char *format, ...);
int vprintf(const char *format, va_list args);

