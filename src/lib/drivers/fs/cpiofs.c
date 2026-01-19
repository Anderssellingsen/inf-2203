// #define LOG_LEVEL LOG_DEBUG

#include <drivers/devices.h>
#include <drivers/log.h>
#include <drivers/vfs.h>

#include <core/ctype.h>
#include <core/errno.h>
#include <core/macros.h>
#include <core/path.h>
#include <core/sprintf.h>
#include <core/string.h>
#include <core/types.h>

#include <stdint.h>

/** @name CPIO mode field bits */
///@{
#define CM_FT_MASK 0060000
#define CM_FT_DIR  0040000
#define CM_FT_CHR  0020000
#define CM_FT_BLK  0060000
///@}

enum cpio_format {
    CF_UNKNOWN = 0,
    CF_NEWC,
};

/**
 * CPIO "New Character" format (aka "New ASCII") header struct
 *
 * - The magic number is the 6 characters "070701" (no terminator)
 * - All other fields are encoded in ASCII as hexadecimal numbers
 * - All fields are 8 characters (8 hex digits)
 * - After path name, pad to 4-byte alignment
 * - After file data, pad to 4-byte alignment
 *
 * @see
 * - Arch Linux has a detailed man 5 page for the CPIO file format:
 *      [cpio(5)](https://man.archlinux.org/man/cpio.5.en)
 */
struct cpio_newc_header {
    char c_magic[6];
    char c_ino[8];
    char c_mode[8];
    char c_uid[8];
    char c_gid[8];
    char c_nlink[8];
    char c_mtime[8];
    char c_filesize[8];
    char c_devmajor[8];
    char c_devminor[8];
    char c_rdevmajor[8];
    char c_rdevminor[8];
    char c_namesize[8];
    char c_check[8];
};

/** General CPIO header struct that can acommodate multiple formats */
struct cpio_header {
    /** Raw header data */
    ///@{
    enum cpio_format fmt; ///< CPIO format type
    union {
        struct cpio_newc_header newc; ///< Raw header as "newc" format.
        char bytes[sizeof(struct cpio_newc_header)]; ///< Raw header as bytes.
    };
    ///@}

    /** @name Important offsets and sizes */
    ///@{
    loff_t hoff;  ///< Start of CPIO header
    size_t hsize; ///< Size of CPIO header
    size_t psize; ///< Path length (including null terminator)
    size_t ppad;  ///< Padding after pathname before file data
    size_t fsize; ///< File size
    size_t fpad;  ///< Padding after file data before next header
    ///@}

    /** @name Path name and results */
    ///@{
    char pathname[PATH_MAX]; ///< Copy of pathname
    int  is_endmarker;       ///< Is this header the "TRAILER!!" entry?
    ///@}
};

/** Log format wrapper with CPIO header info */
#define CPIOH(HPTR, FMT) "%4lu %s\t" FMT, (HPTR)->hoff, (HPTR)->pathname

/** @name CPIO value decoding */
///@{

static long cpio_atoi(const char *field, size_t n, int base)
{
    int ret = 0;
    for (const char *pos = field; n; pos++, n--) {
        int  digit;
        char c = tolower(*pos);
        if ('0' <= c && c <= '9') digit = c - '0' + 0x0;
        else if ('a' <= c && c <= 'f') digit = c - 'a' + 0xa;
        else {
            pr_error("%s: non-digit '%c' in header field\n", __func__, c);
            return -EINVAL;
        }
        if (digit >= base) {
            pr_error("%s: digit %x >= base %u\n", __func__, digit, base);
            return -EINVAL;
        }
        ret = ret * base + digit;
    }
    return ret;
}

static long newc_atoi(const char *field) { return cpio_atoi(field, 8, 16); }

///@}

/** @name CPIO header reading and decoding */
///@{

/**
 * Check CPIO magic numbers and read in raw header data
 *
 * @pre     The file should be positioned at start of header.
 * @post    Raw data will be read in and the @ref cpio_header.fmt field will
 *          be set to indicate header type. The file will be positioned after
 *          the header, ready to read in the pathname.
 */
static ssize_t cpio_read_header_raw(struct file *f, struct cpio_header *h)
{
    ssize_t ct = 0, res;

    /* Reset header. */
    *h = (struct cpio_header){.hoff = f->f_pos};

    /* Read enough bytes to check ASCII magic numbers. */
    ct += res = file_read(f, h->bytes, 6);
    if (res < 0) return res;
    if (res == 0) {
        pr_error(CPIOH(h, "read past EOF\n"));
        return -EINVAL;
    }

    /* Check ASCII magic numbers. */
    if (memcmp(h->bytes, "070701", 6) == 0) {
        pr_debug(CPIOH(h, "found newc magic \"%.6s\"\n"), h->newc.c_magic);
        h->fmt   = CF_NEWC;
        h->hsize = sizeof(struct cpio_newc_header);
    } else {
        pr_error(CPIOH(h, "not a known CPIO magic: \"%.6s\"\n"), h->bytes);
        return -EINVAL;
    }

    /* Read rest of header. */
    ct += res = file_read(f, h->bytes + ct, h->hsize - ct);
    if (res < 0) return res;

    return ct;
}

/** Decode important size fields in header */
static int cpioh_decode_sizes(struct cpio_header *h)
{
    switch (h->fmt) {
    case CF_NEWC:
        h->psize = newc_atoi(h->newc.c_namesize);
        h->fsize = newc_atoi(h->newc.c_filesize);
        h->ppad  = ALIGN_UP(h->hsize + h->psize, 4) - (h->hsize + h->psize);
        h->fpad  = ALIGN_UP(h->fsize, 4) - h->fsize;
        return 0;
    case CF_UNKNOWN: return -EINVAL;
    };
    return -EINVAL;
}

/** Convert CPIO mode field to @ref dirtype value */
static int cpio_mode_to_dirtype(unsigned mode)
{
    switch (mode & CM_FT_MASK) {
    case 0: return DT_REG;
    case CM_FT_DIR: return DT_DIR;
    case CM_FT_CHR: return DT_CHR;
    case CM_FT_BLK: return DT_CHR;
    default: return -EINVAL;
    };
}

/** Populate file struct with values from CPIO header */
static int cpioh_fstat(const struct cpio_header *h, struct fstat *fstat)
{
    unsigned mode;
    switch (h->fmt) {
    case CF_NEWC:
        fstat->f_rdev =
                MAKEDEV(newc_atoi(h->newc.c_devmajor),
                        newc_atoi(h->newc.c_devminor));

        mode          = newc_atoi(h->newc.c_mode);
        fstat->f_type = cpio_mode_to_dirtype(mode);
        fstat->f_size = h->fsize;
        return 0;
    case CF_UNKNOWN: return -EINVAL;
    };
    return -EINVAL;
}

/** Read the pathname that follows a CPIO header */
static ssize_t cpio_read_pathname(struct file *f, struct cpio_header *h)
{
    ssize_t ct = 0, res;

    /* Read pathname. */
    ssize_t readsz = MIN(h->psize, sizeof(h->pathname));
    while (ct < readsz) {
        ct += res = file_read(f, h->pathname + ct, readsz - ct);
        if (res < 0) return res;
    }
    if (h->psize > sizeof(h->pathname)) return -EOVERFLOW;
    pr_debug(CPIOH(h, "got pathname\n"));

    /* Skip post-path padding. */
    if (h->ppad != 0) {
        res = file_lseek(f, h->ppad, SEEK_CUR);
        if (res < 0) return res;
        ct += h->ppad;
    }

    return ct;
}

/** Full header read: read header, decode sizes, and read pathname */
static ssize_t cpio_read_header(struct file *f, struct cpio_header *h)
{
    ssize_t ct = 0, res;

    ct += res = cpio_read_header_raw(f, h);
    if (res < 0) return res;

    res = cpioh_decode_sizes(h);
    if (res < 0) return res;

    ct += res = cpio_read_pathname(f, h);
    if (res < 0) return res;

    if (h->fsize == 0
        && strncmp(h->pathname, "TRAILER!!!", sizeof(h->pathname)) == 0) {
        h->is_endmarker = 1;
        pr_debug(CPIOH(h, "found end-of-archive marker\n"));
    }

    return ct;
}

/** Seek from beginning of file data to next header */
static ssize_t cpio_skip_fdata(struct file *f, struct cpio_header *h)
{
    return file_lseek(f, h->fsize + h->fpad, SEEK_CUR);
}

///@}

/** @name CPIO searches */
///@{

static int cpio_find_path(struct file *f, const char *p, struct cpio_header *h)
{
    int res;
    *h = (struct cpio_header){};
    pr_debug("find_path %s\n", p);
    for (size_t i = 0; !h->is_endmarker; i++) {
        res = cpio_read_header(f, h);
        if (res < 0) return res;

        if (strcmp(h->pathname, p) == 0) return i;

        res = cpio_skip_fdata(f, h);
        if (res < 0) return res;
    }
    return -ENOENT;
}

///@}

/** @name CPIOfs driver data for files */
///@{

struct cfdata {
    struct file        af;   ///< Archive file.
    struct cpio_header h;    ///< Header for target file.
    loff_t             foff; ///< Offset of target file inside archive file.
};

#define MAX_CPIO_OPEN 4
static struct cfdata cfdatas[MAX_CPIO_OPEN];

static struct cfdata *cfdata_alloc(void)
{
    for (size_t i = 0; i < ARRAY_SIZE(cfdatas); i++) {
        struct cfdata *cfdata = cfdatas + i;
        if (!cfdata->af.f_op) return cfdata;
    }
    return NULL;
}

static void cfdata_free(struct cfdata *cfdata) { cfdata->af.f_op = NULL; }

///@}

/** @name CPIOfs Operations */
///@{

static int cpio_sb_open(struct superblock *sb)
{
    int res;

    struct file af; // Archive File
    res = file_open_dev(&af, sb->s_bdev);
    if (res < 0) return res;

    file_debugstr(sb->s_name, sizeof(sb->s_name), &af);

    /* Find root inode. */
    struct cpio_header h = {};
    sb->s_root_ino = res = cpio_find_path(&af, ".", &h);
    debug_result(res, "find root dir: header #%u\n", sb->s_root_ino);
    if (res < 0) goto close_and_return;

close_and_return:
    file_close(&af);
    return res;
}

static int
cpio_file_open_path(struct file *f, struct superblock *sb, const char *path)
{
    int res, isopen_af = 0;
    if (!*path) path = ".";

    /* Allocate a CPIO file data structure. */
    struct cfdata *cfdata = cfdata_alloc();
    res                   = cfdata ? 0 : -ENOMEM;
    if (!cfdata) goto exit;

    /* Open the archive file. */
    res = file_open_dev(&cfdata->af, sb->s_bdev);
    if (res < 0) goto exit;
    isopen_af = 1;

    /* Search for the desired path. */
    res = cpio_find_path(&cfdata->af, path, &cfdata->h);
    if (res < 0) goto exit;

    /* Copy metadata fields. */
    res = cpioh_fstat(&cfdata->h, &f->f_stat);
    if (res < 0) goto exit;

    /* Finish file setup. */
    cfdata->foff = cfdata->h.hoff + cfdata->h.hsize + cfdata->h.psize
                   + cfdata->h.ppad;
    f->f_driver_data = cfdata;

    res = 0;
exit:
    if (res < 0) {
        if (cfdata && isopen_af) file_close(&cfdata->af);
        if (cfdata) cfdata_free(cfdata);
    }
    return res;
}

static int cpio_file_release(struct file *f)
{
    if (f->f_driver_data) cfdata_free(f->f_driver_data);
    return 0;
}

static ssize_t
cpio_file_read(struct file *f, void *dst, size_t count, loff_t *off)
{
    struct cfdata *cfdata = f->f_driver_data;

    /* Get target offset within archive file. */
    loff_t aoff = *off + cfdata->foff;

    /* Don't read archive file past end of target file. */
    if (*off + (loff_t) count > f->f_stat.f_size)
        count = f->f_stat.f_size - f->f_pos;

    /* Read target file within archive file. */
    int res = file_pread(&cfdata->af, dst, count, aoff);
    if (res < 0) return res;
    aoff += res;

    /* Set resulting offset. */
    *off = aoff - cfdata->foff;

    return res;
}

static int cpio_file_readdir(struct file *f, struct dirent *d)
{
    int res;

    struct cfdata *cfdata  = f->f_driver_data;
    const char    *dirpath = cfdata->h.pathname;
    if (strcmp(dirpath, ".") == 0) dirpath = "";
    int dirpathlen = strlen(dirpath);

    /* If the archive was just opened, the position should be just after
     * the directory's own header. Skip to the next header. */
    if (f->f_pos == 0) {
        res = cpio_skip_fdata(&cfdata->af, &cfdata->h);
        if (res < 0) return res;
    }

    /* Read entries from archive file starting at current position,
     * looking for an entry whos path begins with the directory's path. */
    struct cpio_header h = {};
    for (;;) {
        res = cpio_read_header(&cfdata->af, &h);
        if (res < 0) return res;
        res = cpio_skip_fdata(&cfdata->af, &h);
        if (res < 0) return res;

        if (h.is_endmarker) return 0;
        if (strncmp(h.pathname, dirpath, dirpathlen) == 0) break;
    }

    struct fstat fstat;
    cpioh_fstat(&h, &fstat);
    d->d_ino  = fstat.f_ino;
    d->d_type = fstat.f_type;

    const char *relpath = path_strip_prefix(h.pathname, dirpath);
    snprintf(d->d_name, PATH_MAX, "%s", relpath);
    return 1;
}

static const struct file_operations cpio_file_ops = {
        .name      = "cpio_file",
        .open_path = cpio_file_open_path,
        .release   = cpio_file_release,
        .read      = cpio_file_read,
        .readdir   = cpio_file_readdir,
};

static const struct fs_operations cpio_fs_ops = {
        .name        = "cpiofs",
        .sb_open     = cpio_sb_open,
        .fs_file_ops = &cpio_file_ops,
};

int init_driver_cpiofs(void) { return fs_register(FS_CPIO, &cpio_fs_ops); }

///@}
