#include "process.h"

#include <cpu.h>
#include <sys/syscall.h>

#include <drivers/log.h>

#include <core/errno.h>
#include <core/macros.h>

long syscall_dispatch(
        long   number,
        ureg_t arg1,
        ureg_t arg2,
        ureg_t arg3,
        ureg_t arg4,
        ureg_t arg5
)
{
    pr_debug("syscall %ld from process %s\n", number, current_process->name);

    switch ((enum syscall_nr) number) {
    case SYS_NULL:
    case SYS_MAX: break;

    }

    UNUSED(arg1), UNUSED(arg2), UNUSED(arg3);
    UNUSED(arg4), UNUSED(arg5);

    pr_info("unhandled syscall %ld from process %d (%s)\n", number,
            current_process->pid, current_process->name);
    return -ENOSYS;
}

