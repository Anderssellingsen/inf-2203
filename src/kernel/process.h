#ifndef PROCESS_H
#define PROCESS_H

#include "pagemap.h"

#include <abi.h>

#include <drivers/vfs.h>

#include <core/types.h>

#include <stdint.h>

#define FD_MAX 4

struct process {
    struct file execfile;
    char        name[DEBUGSTR_MAX];

    pid_t     pid;
    uintptr_t start_addr;

    /* Stack addresses */
    uintptr_t ustack;
    uintptr_t kstack;

    /** Process's address space (page mapping hierarchy) */
    struct addrspc addrspc;

    /**
     * File descriptors
     *
     * - 0 = stdin
     * - 1 = stdout
     * - 2 = stderr
     */
    struct file *fds[FD_MAX];
};

extern struct process *current_process;

struct process *process_alloc(void);
int  process_load_path(struct process *p, const char *cwd, const char *path);
int  process_start(struct process *p, int argc, char *argv[]);
void process_close(struct process *p);

void process_kill(struct process *p);
_Noreturn void process_exit(int status);
ssize_t        process_write(int fd, const void *src, size_t count);

#endif /* PROCESS_H */
