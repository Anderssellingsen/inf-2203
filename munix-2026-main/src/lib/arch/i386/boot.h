/**
 * @file
 * Common API for Boot Code
 */
#ifndef ARCH_BOOT_H
#define ARCH_BOOT_H

#include <stddef.h>

struct boot_info {
    void    *kernel_location;
    void    *initrd_addr;
    size_t   initrd_size;
    void    *text_fb_addr;
    unsigned text_fb_width;
    unsigned text_fb_height;
};

int kernel_main(void);
int read_boot_info(struct boot_info *b);

#endif /* ARCH_BOOT_H */
