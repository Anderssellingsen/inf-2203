#ifndef PROCESS_H
#define PROCESS_H

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

};

struct process *process_alloc(void);
int  process_load_path(struct process *p, const char *cwd, const char *path);
int  process_start(struct process *p, int argc, char *argv[]);
void process_close(struct process *p);

#endif /* PROCESS_H */
