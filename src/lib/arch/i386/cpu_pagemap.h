#ifndef CPU_X86_PAGEMAP_H
#define CPU_X86_PAGEMAP_H

#include "cpu.h"
#include "x86_seg.h"

#include <core/inttypes.h>
#include <core/macros.h>

#define PAGESZ 4096

#if __i386__
#define PM_LVL_MAX (2 + 2)
#elif __x86_64__
#define PM_LVL_MAX (5 + 2)
#endif

typedef uint32_t paddr_t;      ///< CPU physical memory address
#define PRIdPADDR PRId32       ///< paddr format snippet: signed decimal
#define PRIuPADDR PRIu32       ///< paddr format snippet: unsigned decimal
#define PRIxPADDR PRIx32       ///< paddr format snippet: hex
#define FMT_PADDR "%#8" PRIx32 ///< paddr convenient printf format

typedef uint32_t pme_t;      ///< Page mapping entry
#define PRIdPME PRId32       ///< pme format snippet: signed decimal
#define PRIuPME PRIu32       ///< pme format snippet: unsigned decimal
#define PRIxPME PRIx32       ///< pme format snippet: hex
#define FMT_PME "%#8" PRIx32 ///< pme convenient printf format

#define PM_LVL_ROOT (0)

#define PML_CR3   0
#define PML_PGDIR 1
#define PML_PGTBL 2
#define PML_PAGE  3

struct pm_lvl {
    const char *name;
    unsigned    idx_bits;
    unsigned    is_root : 1;
    unsigned    is_tbl  : 1;
    unsigned    is_page : 1;
};

struct pm_mode {
    const char *name;
    unsigned offset_bits; ///< Number of bits that select a bye within a page
    size_t   entrysz;     ///< Size of one page mapping entry, in bytes
    unsigned lvlct;       ///< Number of page map levels (not including root)
    struct pm_lvl lvls[]; ///< Level descriptions (including root)
};

extern const struct pm_mode *pm_mode;

static inline size_t pm_tblsz(int lvl)
{
    return pm_mode->entrysz << pm_mode->lvls[lvl].idx_bits;
}

#define PME_PRESENT  (1 << 0)
#define PME_W        (1 << 1)
#define PME_USER     (1 << 2)
#define PME_ACCESSED (1 << 5)
#define PME_DIRTY    (1 << 6)

static inline paddr_t pme_paddr(pme_t pme, int lvl)
{
    UNUSED(lvl);
    return pme & ~0xfff;
}

static inline pme_t pme_set_flags(pme_t pme, pme_t flags, int lvl)
{
    if (lvl == PML_CR3)
        flags &= (1 << 3) | (1 << 4); /* Only PWT and PCD flags. */
    return pme | flags;
}

static inline pme_t pme_pack(paddr_t paddr, pme_t flags, int lvl)
{
    return pme_set_flags(paddr, flags, lvl);
}

static inline int pme_ispresent(pme_t pme, int lvl)
{
    if (lvl == 0) return pme != 0;
    else return pme & PME_PRESENT;
}

int pme_tostr(char *buf, size_t n, pme_t pme, int lvl);

static inline pme_t pm_get_root(void)
{
    pme_t cr3;
    x86_get_reg("cr3", cr3);
    return cr3;
}

void pm_set_root(pme_t root_pme);

int init_cpu_pm(pme_t root_pme);

#endif /* CPU_X86_PAGEMAP_H */
