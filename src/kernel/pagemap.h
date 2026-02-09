#ifndef KERNEL_PAGEMAP_H
#define KERNEL_PAGEMAP_H

#include <cpu_pagemap.h>

struct physpage {
    paddr_t paddr;
    void   *vaddr;
};

struct addrspc {
    pme_t root_entry;
};

extern struct addrspc kernel_addrspc;

struct physpage *physpage_alloc(void);
void             physpage_free(struct physpage *page);
struct physpage *physpage_find(paddr_t paddr);
void            *physpage_access(struct physpage *page);
void             physpage_close(struct physpage *page);

int addrspc_init(struct addrspc *space);
int addrspc_cleanup(struct addrspc *space);
int addrspc_map(
        struct addrspc *space,
        void           *vaddr,
        paddr_t         paddr,
        size_t          size,
        pme_t           flags
);
int addrspc_unmap(struct addrspc *space, void *vaddr, size_t size);

int init_pm(void);

#endif /* KERNEL_PAGEMAP_H */
