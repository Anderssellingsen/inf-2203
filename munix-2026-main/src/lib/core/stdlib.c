#include "stdlib.h"

#include "ctype.h"

int atoi(const char *a)
{
    unsigned base = 10;

    /* Skip whitespace. */
    while (*a == ' ') a++;

    /* Check for "0x" prefix */
    if (a[0] == '0' && (a[1] == 'x' || a[1] == 'X')) base = 16, a += 2;

    /* Add digits. */
    int i = 0;
    for (; *a && isxdigit(*a); a++) {
        if ('0' <= *a && *a <= '9') i = i * base + (*a - '0');
        if ('a' <= *a && *a <= 'f') i = i * base + (*a - 'a' + 0xa);
        if ('A' <= *a && *a <= 'F') i = i * base + (*a - 'A' + 0xa);
    }
    return i;
}
