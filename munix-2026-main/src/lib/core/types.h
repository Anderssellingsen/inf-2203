#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>

typedef long     ssize_t; ///< Signed size_t, for returning size or error
typedef long     loff_t;  ///< File position offset
typedef unsigned ino_t;   ///< Filesystem inode number
typedef int      pid_t;   ///< Process ID

/** @name Device numbers */
///@{

typedef unsigned int dev_t;

#define MAKEDEV(MAJ, MIN) ((MAJ << 8) | (MIN))
#define MAJOR(DEV)        (DEV >> 8)
#define MINOR(DEV)        (DEV & 0xff)
///@}

#endif /* TYPES_H */
