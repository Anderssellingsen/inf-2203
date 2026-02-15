#ifndef FILEFORMAT_ELF_H
#define FILEFORMAT_ELF_H

#include <oss/elf.h>

#include <drivers/vfs.h>

int elf_read_ehdr32(struct file *f, Elf32_Ehdr *ehdr);
int elf_read_phdr32(
        struct file *f, Elf32_Ehdr *ehdr, size_t i, Elf32_Phdr *phdr
);
int elf_load_seg32(struct file *f, Elf32_Phdr *phdr);

#endif /* FILEFORMAT_ELF_H */
