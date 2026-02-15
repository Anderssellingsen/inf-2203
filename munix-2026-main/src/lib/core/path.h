#ifndef PATH_H
#define PATH_H

#include <stddef.h>

int   path_join(char *dst, size_t dstsz, const char *a, const char *b);
char *path_strip_prefix(const char *path, const char *prefix);
int   path_basename(char *dst, size_t dstsz, const char *path);

#endif /* PATH_H */
