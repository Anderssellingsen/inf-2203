#include "stdio.h"

#include "unistd.h"

#include <stdarg.h>

static FILE files[] = {
        [0] = {.fd = 0},
        [1] = {.fd = 1},
        [2] = {.fd = 2},
};

FILE *stdin  = &files[0];
FILE *stdout = &files[1];
FILE *stderr = &files[2];

int vfprintf(FILE *f, const char *fmt, va_list va)
{
    int bufsz = 256;
try_format : {
    char buf[bufsz];

    /* Format message. */
    va_list va_tmp;
    va_copy(va_tmp, va);
    int res = vsnprintf(buf, bufsz, fmt, va);
    va_end(va_tmp);
    if (res < 0) return res;

    /* If the formatted output was too big for the buffer,
     * try again with a buffer that is the correct size. */
    if (res >= bufsz) {
        bufsz = res + 1;
        goto try_format;
    }

    /* If formatting was successful, write to the file descriptor. */
    res = write(f->fd, buf, res);
    return res;
}
}

ATTR_PRINTFLIKE(2, 3)
int fprintf(FILE *f, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int res = vfprintf(f, format, args);
    va_end(args);
    return res;
}

int vprintf(const char *format, va_list args)
{
    return vfprintf(stdout, format, args);
}

ATTR_PRINTFLIKE(1, 2)
int printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int res = vfprintf(stdout, format, args);
    va_end(args);
    return res;
}

