#include "kernel.h"
#include "process.h"

#include <cpu_interrupt.h>

#include <drivers/log.h>

static void handle_exception(ivec_t ivec, struct intrdata *idata)
{
    const int dbgsz = 256;
    char      dbgbuf[dbgsz];

    pr_error(
            "process %d (%s): unhandled exception " FMT_IVEC " (%s) %s\n",
            current_process ? current_process->pid : 0,
            current_process ? current_process->name : "", ivec,
            ivec_name(ivec),
            (intrdata_tostr(dbgbuf, dbgsz, ivec, idata), dbgbuf)
    );

    /* Kill current process. */
    process_kill(current_process);
    kernel_noreturn();
}

void interrupt_dispatch(ivec_t ivec, struct intrdata *idata)
{
    pr_debug("interrupt %d (%s)\n", ivec, ivec_name(ivec));

    if (ivec_isexception(ivec)) return handle_exception(ivec, idata);
}
