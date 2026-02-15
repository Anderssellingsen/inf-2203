#define LOG_LEVEL LOG_DEBUG
#include "vfs.h"

#include <drivers/devices.h>
#include <drivers/log.h>

#include <core/errno.h>
#include <core/sprintf.h>

#include <stdarg.h>

static const struct file_operations *chrdev_drivers[MAJORS_MAX];

static int
chrdev_register_inner(unsigned maj, const struct file_operations *fops)
{
    if (maj <= 0 || MAJORS_MAX <= maj) return -EINVAL;
    if (chrdev_drivers[maj]) return chrdev_drivers[maj] == fops ? 0 : -EBUSY;
    chrdev_drivers[maj] = fops;
    return 0;
}

int chrdev_register(unsigned maj, const struct file_operations *fops)
{
    int res = chrdev_register_inner(maj, fops);
    log_result(
            res, "registered device driver: major #%u = %s\n", maj, fops->name
    );
    return maj;
}

static int file_open_dev_inner(struct file *file, dev_t rdev)
{
    /* Find device. */
    unsigned maj = MAJOR(rdev), min = MINOR(rdev);
    if (maj <= 0 || MAJORS_MAX <= maj) return -ENODEV;
    const struct file_operations *ops = chrdev_drivers[maj];
    if (!ops) return -ENODEV;

    /* Set file operations. */
    file->f_stat.f_type = DT_CHR;
    file->f_op          = ops;
    file->f_stat.f_rdev = rdev;

    /* Call device's open method. */
    if (!file->f_op->open_dev) return 0;
    return file->f_op->open_dev(file, min);
}

int file_open_dev(struct file *file, dev_t rdev)
{
    /* Reset file struct. */
    *file = (struct file){};
    return file_open_dev_inner(file, rdev);
}

int file_close(struct file *file)
{
    if (file && file->f_op && file->f_op->release)
        return file->f_op->release(file);
    else return 0;
}

int file_debugstr(char *descbuf, size_t n, struct file *f)
{
    if (!f || !f->f_op) return snprintf(descbuf, n, "file{NULL}");
    if (f->f_op->debugstr) return f->f_op->debugstr(descbuf, n, f);

    if (f->f_stat.f_rdev)
        return snprintf(
                descbuf, n, "%s%u", f->f_op->name, MINOR(f->f_stat.f_rdev)
        );

    return snprintf(descbuf, n, "file{%p}", f);
    return 0;
}

ssize_t file_write(struct file *f, const void *src, size_t count)
{
    if (!f || !f->f_op || !f->f_op->write) return -EINVAL;
    if (!src || !count) return 0;
    return f->f_op->write(f, src, count, &f->f_pos);
}

ssize_t file_pwrite(struct file *f, const void *src, size_t count, loff_t off)
{
    if (!f || !f->f_op || !f->f_op->write) return -EINVAL;
    if (!src || !count) return 0;
    return f->f_op->write(f, src, count, &off);
}

ssize_t file_read(struct file *f, void *dst, size_t count)
{
    if (!f || !f->f_op || !f->f_op->read) return -EINVAL;
    if (!dst || !count) return 0;
    return f->f_op->read(f, dst, count, &f->f_pos);
}

ssize_t file_pread(struct file *f, void *dst, size_t count, loff_t off)
{
    if (!f || !f->f_op || !f->f_op->read) return -EINVAL;
    if (!dst || !count) return 0;
    return f->f_op->read(f, dst, count, &off);
}

loff_t file_lseek(struct file *f, loff_t off, int whence)
{
    int res = 0;
    if (!f || !f->f_op) return -EINVAL;
    if (f->f_op->lseek) {
        res = f->f_op->lseek(f, off, whence);
        if (res < 0) return res;
    }

    switch (whence) {
    case SEEK_SET: f->f_pos = off; return res;
    case SEEK_CUR: f->f_pos += off; return res;
    case SEEK_END: f->f_pos = f->f_stat.f_size + off; return res;
    default: return -EINVAL;
    };
}

int file_ioctl(struct file *f, unsigned cmd, uintptr_t arg)
{
    if (!f || !f->f_op || !f->f_op->ioctl) return -EINVAL;
    return f->f_op->ioctl(f, cmd, arg);
}

int file_readstr(struct file *f, char *dst, size_t n)
{
    if (!n) return 0;
    int res = file_read(f, dst, n - 1);
    if (res < 0) return res;
    dst[res] = '\0'; // Write null terminator.
    return res;
}

#define DEFAULT_BUFSZ 256

int file_vprintf(struct file *f, const char *fmt, va_list va)
{
    /* To format into a file, we first format into a buffer, and then write
     * from the buffer into the file. We don't know how big a buffer we need
     * though, so we might have to do this twice: first with the default size,
     * then with the correct size. */
    va_list va_tmp;
    int     bufsz = DEFAULT_BUFSZ;
    for (;;) {
        /* Allocate a temporary buffer and format into it. */
        char buf[bufsz];     // Buffer for this iteration.
        va_copy(va_tmp, va); // Copy of arg state for this iteration.
        int len = vsnprintf(buf, bufsz, fmt, va_tmp);
        va_end(va_tmp);

        /* Check for errors. */
        if (len < 0) return len;

        /* If the formatting was complete, write the output to the file. */
        if (len < bufsz) return file_write(f, buf, len);

        /* If the buffer was too small, we now know exactly how long the
         * formatted output is. Run through the loop again, this time
         * with the correct buffer size. */
        bufsz = len + 1;
    }
}

ATTR_PRINTFLIKE(2, 3)
int file_printf(struct file *f, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    int res = file_vprintf(f, fmt, va);
    va_end(va);
    return res;
}
