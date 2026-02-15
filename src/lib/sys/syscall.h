#if __linux__
/* Defer to Linux's <syscall.h> to get Linux's syscall numbers. */
#include_next <sys/syscall.h>
#endif

#ifndef SYSCALL_H
#define SYSCALL_H

#if __munix__
enum syscall_nr {
    SYS_NULL = 0,
    SYS_exit,
    SYS_write,
    SYS_MAX
};
#endif /* __munix__ */

long syscall(long number, ...);
long sys_thrd_create(void *thrd_fn, void *fn_arg, void *child_stack, ...);

#endif /* SYSCALL_H */
