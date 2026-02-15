#include "path.h"

#include <core/macros.h>
#include <core/sprintf.h>
#include <core/string.h>

int path_join(char *dst, size_t dstsz, const char *a, const char *b)
{
    char *pos = dst, *end = dst + dstsz;

    int path_isabs = b && b[0] == '/';
    if (!path_isabs && a) {
        pos += snprintf(pos, BUFREM(pos, end), "%s", a);
        if (b && pos != dst && *(pos - 1) != '/')
            pos += snprintf(pos, BUFREM(pos, end), "%s", "/");
    }
    if (b) pos += snprintf(pos, BUFREM(pos, end), "%s", b);
    return pos - dst;
}

char *path_strip_prefix(const char *path, const char *prefix)
{
    if (!path) return NULL;
    if (!prefix) return (char *) path;

    int prefixlen = strlen(prefix);
    if (strncmp(path, prefix, prefixlen) != 0) return NULL;

    char *pos = (char *) path + prefixlen;
    if (*pos == '/') pos++;
    return pos;
}

int path_basename(char *dst, size_t dstsz, const char *path)
{
    /* Special cases: null and root */
    if (!path) return 0;
    if (path[0] == '/' && path[1] == '\0') return snprintf(dst, dstsz, "/");

    /* Scan path. Find position of last and second-to-last slashes. */
    char *pos, *prevslash, *lastslash, *pend;
    for (prevslash = lastslash = pos = (char *) path; *pos; pos++)
        if (*pos == '/') prevslash = lastslash, lastslash = pos + 1;
    pend = pos;

    /* Calculate name start and end. */
    char *nstart, *nend;
    if (lastslash == pend) nstart = prevslash, nend = pend - 1; // Trailing
    else nstart = lastslash, nend = pend;                       // Normal

    int nlen = nend - nstart;
    return snprintf(dst, dstsz, "%.*s", nlen, nstart);
}
