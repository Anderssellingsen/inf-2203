#define LOG_LEVEL LOG_DEBUG

#include "process.h"
#include "kernel.h"

#include <abi.h>
#include <cpu.h>

#include <drivers/fileformat/elf.h>
#include <drivers/log.h>
#include <drivers/vfs.h>

#include <core/errno.h>
#include <core/inttypes.h>
#include <core/macros.h>
#include <core/path.h>
#include <core/sprintf.h>
#include <core/string.h>

#define PROCESS_MAX 8
static struct process pcb[PROCESS_MAX];
static pid_t          next_pid = 1;

struct process *process_alloc(void)
{
    for (int i = 0; i < PROCESS_MAX; i++)
        if (!pcb[i].pid) return &pcb[i];
    return NULL;
}

int process_load_path(struct process *p, const char *cwd, const char *path)
{
    int res, file_isopen = 0;

    /* Reset struct. */
    *p = (struct process){.pid = next_pid++};
    path_basename(p->name, DEBUGSTR_MAX, path);

    /* Open file. */
    res = file_open_path(&p->execfile, cwd, path);
    if (res < 0) goto error;
    file_isopen = 1;

    /* Read ELF header. */
    Elf32_Ehdr ehdr;
    res = elf_read_ehdr32(&p->execfile, &ehdr);
    if (res < 0) goto error;
    p->start_addr = ehdr.e_entry;

    /* Load segments. */
    for (int i = 0; i < ehdr.e_phnum; i++) {
        Elf32_Phdr phdr;
        res = elf_read_phdr32(&p->execfile, &ehdr, i, &phdr);
        if (res < 0) goto error;

        /* Skip non-load segments. */
        if (phdr.p_type != PT_LOAD) continue;

        /* Validates segment sizes and returns an error if theres an invalid ELF layout */
        if (phdr.p_filesz > phdr.p_memsz) {
            res = -EINVAL;
            goto error;
        }

        /* cast the address to the ELF segment so the kernel can write bytes into it */
        uint8_t *dst = (uint8_t *)(uintptr_t)phdr.p_vaddr;

        /* reads bytes from p_files and offset from p_offset and stores it in dst */
        size_t size   = phdr.p_filesz;
        loff_t offset = phdr.p_offset;

        ssize_t n = file_pread(&p->execfile, dst, size, offset);

        /* error handling for read failure and short read */
        if (n < 0) {
            res = (int)n;
            goto error;
        }
        if ((size_t)n != (size_t)phdr.p_filesz) {
            res = -EIO;
            goto error;
        }

        /* Zeroes the remaining bytes */
        if (phdr.p_memsz > phdr.p_filesz) {
            memset(dst + phdr.p_filesz, 0,
                   (size_t)(phdr.p_memsz - phdr.p_filesz));
        }
    }

    return 0;

error:
    if (file_isopen) file_close(&p->execfile);
    return res;
}

void process_close(struct process *p)
{
    file_close(&p->execfile);
    *p = (struct process){};
}

enum start_strategy {
    PSTART_CALL,
};

int process_start(struct process *p, int argc, char *argv[])
{
    enum start_strategy start_strat = PSTART_CALL;

    switch (start_strat) {
    case PSTART_CALL: {
        /* Start process via simple function call. */
        typedef int (*entry_fn_t)(int argc, char **argv);
        entry_fn_t entry = (entry_fn_t)(uintptr_t)p->start_addr;

        (void)entry(argc, argv);
        return 0;
    }
    }

    return -ENOTSUP;
}
