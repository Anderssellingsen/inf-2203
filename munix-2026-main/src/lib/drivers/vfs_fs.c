// #define LOG_LEVEL LOG_DEBUG

#include "vfs.h"

#include <drivers/devices.h>
#include <drivers/log.h>

#include <core/errno.h>
#include <core/macros.h>
#include <core/path.h>
#include <core/sprintf.h>
#include <core/string.h>

/** @name VFS filesystem driver registration */
///@{

static const struct fs_operations *fs_drivers[FSTYPES_MAX];

static int
fs_register_inner(unsigned fstypeid, const struct fs_operations *ops)
{
    if (fstypeid <= 0 || FSTYPES_MAX <= fstypeid) return -EINVAL;
    if (fs_drivers[fstypeid]) return fs_drivers[fstypeid] == ops ? 0 : -EBUSY;
    fs_drivers[fstypeid] = ops;
    return 0;
}

int fs_register(unsigned fstypeid, const struct fs_operations *ops)
{
    int res = fs_register_inner(fstypeid, ops);
    log_result(
            res, "registered filesystem driver: fstypeid #%u = %s\n", fstypeid,
            ops->name
    );
    return res;
}

///@}

/** @name superblock operations */
///@{

#define SB_MAX 4
static struct superblock superblocks[SB_MAX];

static struct superblock *sb_alloc(void)
{
    for (size_t i = 0; i < ARRAY_SIZE(superblocks); i++) {
        struct superblock *sb = &superblocks[i];
        if (!sb->s_op) return sb;
    }
    return NULL;
}

static void sb_free(struct superblock *sb) { sb->s_op = NULL; }

static int sb_open(struct superblock *sb, dev_t blockdev, unsigned fstypeid)
{
    int res;

    /* Get driver. */
    const struct fs_operations *s_op = fs_drivers[fstypeid];
    if (!s_op) return -ENODEV;

    /* Reset struct. */
    *sb = (struct superblock){
            .s_bdev = blockdev,
            .s_op   = s_op,
    };
    INIT_LIST_HEAD(&sb->s_mount_list);

    /* Call driver's open method. */
    if (sb->s_op->sb_open) {
        res = sb->s_op->sb_open(sb);
        if (res < 0) return res;
    }

    return 0;
}

static int sb_release(struct superblock *sb)
{
    /* Call driver method. */
    if (sb->s_op->sb_release) return sb->s_op->sb_release(sb);
    else return 0;
}

///@}

/** @name FS mounting and lookup */
///@{

struct list_head vfs_mount_list = LIST_HEAD_INIT(vfs_mount_list);

static void vfs_mount_list_add(struct superblock *sb)
{
    struct superblock *pos;
    list_for_each_entry(pos, &vfs_mount_list, s_mount_list)
    {
        /* Keep list sorted by mount path. */
        if (strcmp(pos->s_mountpath, sb->s_mountpath) > 0) break;
    }
    list_add_tail(&sb->s_mount_list, &vfs_mount_list);
}

int fs_mountdev(dev_t blockdev, unsigned fstypeid, const char *mpath)
{
    int                res;
    int                sb_isopen = 0;
    struct superblock *sb        = NULL;

    /* Allocat superblock. */
    sb  = sb_alloc();
    res = sb ? 0 : -ENOMEM;
    if (res < 0) goto exit;

    /* Allocate and open superblock. */
    res = sb_open(sb, blockdev, fstypeid);
    if (res < 0) goto exit;
    sb_isopen = 1;

    /* Copy mount path. */
    snprintf(sb->s_mountpath, sizeof(sb->s_mountpath), "%s", mpath);

    /* Add to mount list. */
    vfs_mount_list_add(sb);

    res = 0;
exit:
    log_result(res, "mount device %u on %s\n", blockdev, mpath);
    if (res < 0) {
        if (sb && sb_isopen) sb_release(sb);
        if (sb) sb_free(sb);
    }
    return res;
}

static struct superblock *find_mount_for_path(const char *abspath)
{
    struct superblock *sb;
    list_for_each_entry_prev(sb, &vfs_mount_list, s_mount_list)
    {
        if (strstr(abspath, sb->s_mountpath) == abspath) return sb;
    }
    return NULL;
}

static int file_open_sb_path(
        struct file *file, struct superblock *sb, const char *relpath
)
{
    int res;

    /* Check inputs. */
    res = file && sb && sb->s_op ? 0 : -EINVAL;
    if (res < 0) goto exit;

    /* Check if superblock supports operation. */
    const struct file_operations *f_op = sb->s_op->fs_file_ops;
    res = f_op && f_op->open_path ? 0 : -ENOTSUP;
    if (res < 0) goto exit;

    /* Reset struct. */
    *file = (struct file){
            .f_op = sb->s_op->fs_file_ops,
    };

    /* Call driver method. */
    if (file->f_op->open_path) {
        res = file->f_op->open_path(file, sb, relpath);
        if (res < 0) goto exit;
    }

    res = 0;
exit:
    debug_result(res, "open via superblock: %s:%s\n", sb->s_name, relpath);
    return res;
}

static int file_open_path_abs(struct file *file, const char *abspath)
{
    struct superblock *sb = find_mount_for_path(abspath);
    if (!sb) return -ENOENT;
    if (!sb->s_op->fs_file_ops->open_path) return -ENOTSUP;

    const char *relpath = path_strip_prefix(abspath, sb->s_mountpath);
    return file_open_sb_path(file, sb, relpath);
}

int file_open_path(struct file *file, const char *cwd, const char *path)
{
    int  n = PATH_MAX;
    char absbuf[n];
    path_join(absbuf, n, cwd, path);
    return file_open_path_abs(file, absbuf);
}

int file_stat(struct fstat *fstat, const char *cwd, const char *path)
{
    struct file f;
    int         res = file_open_path(&f, cwd, path);
    if (res < 0) return res;
    *fstat = f.f_stat;
    file_close(&f);
    return res;
}

int file_readdir(struct file *f, struct dirent *d)
{
    if (!f || !f->f_op || !f->f_op->readdir) return -EINVAL;
    if (!d) return -EINVAL;
    if (f->f_stat.f_type != DT_DIR) return -ENOTDIR;
    return f->f_op->readdir(f, d);
}

///@}
