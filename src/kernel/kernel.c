#include "kernel.h"

#include "kshell.h"

#include <boot.h>
#include <cpu.h>

#include <drivers/devices.h>
#include <drivers/fileformat/ascii.h>
#include <drivers/log.h>
#include <drivers/vfs.h>

#include <core/errno.h>
#include <core/string.h>
#include <core/types.h>

#include <stdalign.h>

static struct file      serial1;
static struct boot_info boot_info;

static int init_log(void)
{
    int res;

    res = file_open_dev(&serial1, MAKEDEV(MAJ_SERIAL, 1));
    if (res < 0) return res;
    log_set_file(&serial1);

    res = file_ioctl(&serial1, SRL_SETFLAGS, SRL_ICRNL | SRL_OCRNL);
    log_result(res, "turn on serial newline fixes\n");

    return 0;
}

static int mount_initrd(void)
{
    int res;

    res = boot_info.initrd_addr ? 0 : -ENODEV;
    log_result(res, "get initrd info provided by bootloader\n");
    if (res < 0) return res;

    unsigned rd_minor = res = ramdisk_create(
            boot_info.initrd_addr, boot_info.initrd_size, "initrd"
    );
    if (res < 0) return res;

    res = fs_mountdev(MAKEDEV(MAJ_RAMDISK, rd_minor), FS_CPIO, "/");
    if (res < 0) return res;

    return 0;
}

int kernel_main(void)
{
    int res;

    /* Set up essential I/O and logging. */
    res = init_driver_serial();
    if (res < 0) return res;
    init_log();
    read_boot_info(&boot_info);

    /* Init more essential drivers. */
    init_driver_ramdisk();
    init_driver_tty();
    init_driver_cpiofs();

    /* Mount init ramdisk. */
    mount_initrd();

    /* Start shell. */
    kshell_init_run();

    pr_info("nothing more to do; returning to bootloader to restart...\n");
    return 0;
}

