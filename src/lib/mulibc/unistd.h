#ifndef UNISTD_H
#define UNISTD_H

#include <core/types.h>

_Noreturn void _exit(int status);
ssize_t        write(int fd, const void *src, size_t count);

#endif /* UNISTD_H */
