#ifndef VFS_H
#define VFS_H

#include <core/compiler.h>
#include <core/list.h>
#include <core/types.h>

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#define SEEK_SET 1
#define SEEK_CUR 2
#define SEEK_END 3

#define DEBUGSTR_MAX 64
#define PATH_MAX     128

enum dirtype {
    DT_UNKNOWN = 0, ///< Unknown file type
    DT_CHR,         ///< Character device
    DT_DIR,         ///< Directory
    DT_REG,         ///< Regular file
};

/** Entry in a directory listing */
struct dirent {
    ino_t         d_ino;
    unsigned char d_type;
    char          d_name[PATH_MAX];
};

struct superblock {
    /** @name On-disk or filesystem-intrinsic data */
    ///@{
    ino_t s_root_ino; ///< inode number of root inode
    ///@}

    /** @name Live data */
    ///@{
    dev_t s_bdev;               ///< Device number of the FS's block device
    char  s_name[DEBUGSTR_MAX]; ///< String description of superblock
    char  s_mountpath[PATH_MAX];
    struct list_head s_mount_list;
    ///@}

    /** @name Driver polymorphism */
    ///@{
    const struct fs_operations *s_op;
    void                       *s_driver_data;
    ///@}
};

/** File metadata from the filesystem (inode data) */
struct fstat {
    ino_t        f_ino;
    enum dirtype f_type;
    dev_t        f_rdev;
    loff_t       f_size;
};

struct file {
    /** @name Metadata from disk (inode data) */
    ///@{
    struct fstat f_stat;
    ///@}

    /** @name Live data for a file in use */
    ///@{
    struct inode *f_inode; ///< Link to owning inode (may be null for chrdev)
    loff_t        f_pos;   ///< Current read/write position
    ///@}

    /** @name Driver polymorphism */
    ///@{
    const struct file_operations *f_op;
    void                         *f_driver_data;
    ///@}
};

struct fs_operations {
    const char *name;
    int (*sb_open)(struct superblock *sb);
    int (*sb_release)(struct superblock *sb);
    const struct file_operations *fs_file_ops;
};

struct file_operations {
    const char *name;
    int (*open_dev)(struct file *f, unsigned min);

    int (*stat_path
    )(struct fstat *fstat, struct superblock *sb, const char *relpath);
    int (*open_path
    )(struct file *f, struct superblock *sb, const char *relpath);

    int (*release)(struct file *f);

    int (*debugstr)(char *descbuf, size_t n, struct file *f);
    ssize_t (*read)(struct file *f, void *dst, size_t count, loff_t *off);
    int (*readdir)(struct file *f, struct dirent *d);

    ssize_t (*write
    )(struct file *f, const void *src, size_t count, loff_t *off);
    loff_t (*lseek)(struct file *f, loff_t off, int whence);

    int (*ioctl)(struct file *f, unsigned cmd, uintptr_t arg);
};

extern struct list_head vfs_mount_list;

int fs_register(unsigned fstypeid, const struct fs_operations *ops);
int fs_mountdev(dev_t blockdev, unsigned fstypeid, const char *mpath);

int chrdev_register(unsigned maj, const struct file_operations *fops);

int     file_stat(struct fstat *fstat, const char *cwd, const char *path);
int     file_open_dev(struct file *file, dev_t rdev);
int     file_open_path(struct file *file, const char *cwd, const char *path);
int     file_close(struct file *file);
ssize_t file_read(struct file *f, void *dst, size_t count);
ssize_t file_pread(struct file *f, void *dst, size_t count, loff_t off);
int     file_readdir(struct file *f, struct dirent *d);
int     file_debugstr(char *descbuf, size_t n, struct file *f);
ssize_t file_write(struct file *f, const void *src, size_t count);
ssize_t file_pwrite(struct file *f, const void *src, size_t count, loff_t off);
loff_t  file_lseek(struct file *f, loff_t off, int whence);
int     file_ioctl(struct file *f, unsigned cmd, uintptr_t arg);

int file_readstr(struct file *f, char *dst, size_t n);

ATTR_PRINTFLIKE(2, 3)
int file_printf(struct file *f, const char *fmt, ...);
int file_vprintf(struct file *f, const char *fmt, va_list va);

#endif /* VFS_H */
