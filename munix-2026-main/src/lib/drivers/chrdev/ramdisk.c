#include <drivers/devices.h>
#include <drivers/log.h>
#include <drivers/vfs.h>

#include <core/errno.h>
#include <core/macros.h>
#include <core/sprintf.h>

#define RAMDISKS_MAX 4

struct ramdisk {
    char       *addr;
    size_t      rd_size;
    const char *name;
};

static struct ramdisk ramdisks[RAMDISKS_MAX];

static int ramdisk_create_inner(void *addr, size_t size, const char *name)
{
    if (!addr || !size) return -EINVAL;
    for (int i = 0; i < RAMDISKS_MAX; i++) {
        if (!ramdisks[i].addr) {
            ramdisks[i] = (struct ramdisk
            ){.addr = addr, .rd_size = size, .name = name};
            return i;
        }
    }
    return -ENOMEM;
}

int ramdisk_create(void *addr, size_t size, const char *name)
{
    int res = ramdisk_create_inner(addr, size, name);
    log_result(
            res, "create ramdisk device for %s at %p, size %#zx\n", name, addr,
            size
    );
    return res;
}

static int ramdisk_open_dev(struct file *file, unsigned min)
{
    /* Use minor number as ramdisk index. */
    unsigned disk_no = min;
    if (disk_no >= RAMDISKS_MAX || !ramdisks[disk_no].addr) return -ENODEV;
    struct ramdisk *rd = &ramdisks[disk_no];

    file->f_driver_data = rd;
    file->f_stat.f_size = rd->rd_size;
    return 0;
}

static int ramdisk_debugstr(char *descbuf, size_t n, struct file *f)
{
    struct ramdisk *rd = f->f_driver_data;
    return snprintf(descbuf, n, "ramdisk{%s %p}", rd->name, rd->addr);
}

static ssize_t
ramdisk_read(struct file *f, void *dst, size_t count, loff_t *off)
{
    UNUSED(off);
    struct ramdisk *rd = f->f_driver_data;

    unsigned char *bdst = dst;
    if (*off < 0) *off = 0;                 // Do not go below zero.
    if (*off >= f->f_stat.f_size) return 0; // If past end of file, EOF.

    size_t i = 0;
    for (; i < count && *off < f->f_stat.f_size; i++, (*off)++, bdst++)
        *bdst = *(rd->addr + *off);
    return i;
}

static struct file_operations ramdisk_ops = {
        .name     = "ramdisk",
        .open_dev = ramdisk_open_dev,
        .debugstr = ramdisk_debugstr,
        .read     = ramdisk_read,
};

int init_driver_ramdisk(void)
{
    return chrdev_register(MAJ_RAMDISK, &ramdisk_ops);
}
