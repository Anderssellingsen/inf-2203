#ifndef CHRDEV_H
#define CHRDEV_H

#include <stddef.h>

enum chrdev_majors {
    MAJ_NONE = 0,
    MAJ_MEM,
    MAJ_SERIAL,
    MAJ_TTY,
    MAJ_RAMDISK,

    MAJORS_MAX
};

enum ioctl_cmd {
    SRL_GETFLAGS, ///< Serial: get flags
    SRL_SETFLAGS, ///< Serial: set flags

    TTY_GETFLAGS, ///< TTY: get flags
    TTY_SETFLAGS, ///< TTY: set flags
};

enum fs_types {
    FS_NONE = 0,
    FS_DEV,
    FS_SYS,
    FS_CPIO,

    FSTYPES_MAX
};

#define SRL_ICRNL 0x0001 ///< Input: Convert CR to NL ("\r" -> "\n")
#define SRL_OCRNL 0x0002 ///< Output: Convert NL to CR+NL ("\n" -> "\r\n")

#define TTY_ECHO    0x0001 ///< Echo input characters.
#define TTY_ECHOCTL 0x0002 ///< Echo all characters as
#define TTY_COOKED  0x0004 ///< "Cooked" mode: read line-by-line w/ line editing

int init_driver_serial(void);
int init_driver_tty(void);

int init_driver_ramdisk(void);
int ramdisk_create(void *addr, size_t size, const char *name);

int init_driver_cpiofs(void);
#endif /* CHRDEV_H */
