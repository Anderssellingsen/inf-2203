#ifndef KSHELL_H
#define KSHELL_H

#include <drivers/vfs.h>

struct kshell {
    struct file *in, *out, *err;
    char         cwd[PATH_MAX];
    int          waiting_for_input;
};

int kshell_init_tty(struct kshell *sh, struct file *tty);
int kshell_read_exec(struct kshell *sh);
int kshell_run(struct kshell *sh);
int kshell_init_run(void);

#endif /* KSHELL_H */
