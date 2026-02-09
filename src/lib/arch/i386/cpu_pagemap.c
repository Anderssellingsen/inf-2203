#include "cpu_pagemap.h"

#include "cpu_interrupt.h"

#include <drivers/log.h>

#include <core/errno.h>
#include <core/inttypes.h>
#include <core/sprintf.h>

#define CR0_PG  (1 << 31)
#define CR4_PAE (1 << 5)
#define CR4_PSE (1 << 4)

const struct pm_mode X86_PAGING_32 = {
        .name    = "32-bit paging",
        .entrysz = 4,
        .lvlct   = 4,
        .lvls =
                {{.name = "cr3", .is_root = 1},
                 {.name = "pgdir", .idx_bits = 10, .is_tbl = 1},
                 {.name = "pgtbl", .idx_bits = 10, .is_tbl = 1},
                 {.name = "page", .idx_bits = 12, .is_page = 1}},
};

const struct pm_mode *pm_mode = &X86_PAGING_32;

int pme_tostr(char *buf, size_t n, pme_t pme, int lvl)
{
    paddr_t paddr = pme_paddr(pme, lvl);

    /* Decode flags. */
    char        flags[13];
    const char *flags0 = NULL, *flags1;
    if (lvl == PM_LVL_ROOT) {
        flags1 = ".......TC...";
    } else {
        flags1 = "...GLDATCUWP";
        flags0 = "---------sr-";
    }
    flagstr(flags, pme, 12, flags1, flags0);

    return snprintf(buf, n, FMT_PADDR "|%s", paddr, flags);
}

static int x86_pm_cpumode_tostr(char *buf, size_t n)
{
    char *pos = buf, *end = buf + n;

    const char *modename;
    ureg_t      cr0, cr3, cr4;
    x86_get_reg("cr0", cr0);
    x86_get_reg("cr3", cr3);
    x86_get_reg("cr4", cr4);

    int pgbit = !!(cr0 & CR0_PG);
    int pae   = !!(cr4 & CR4_PAE);
    int pse   = !!(cr4 & CR4_PSE);

    if (!(cr0 & CR0_PG)) modename = "page mapping disabled";
    else if (!(cr4 & CR4_PAE)) modename = "32-bit paging";
    else modename = "PAE paging";

    pos += snprintf(pos, BUFREM(pos, end), "%s", modename);
    pos += snprintf(
            pos, BUFREM(pos, end), "\n\tCR0.PG=%d, CR4.PAE=%d, CR4.PSE=%d",
            pgbit, pae, pse
    );
    pos += snprintf(pos, BUFREM(pos, end), "\n\tCR3 = " FMT_REG " (", cr3);
    pos += pme_tostr(pos, BUFREM(pos, end), cr3, PM_LVL_ROOT);
    pos += snprintf(pos, BUFREM(pos, end), ")");

    return pos - buf;
}

void pm_set_root(pme_t root_pme)
{
    const size_t DBGSZ = 256;
    char         dbgbuf[DBGSZ];
    pr_debug(
            "setting page map root " FMT_PME " (%s)...\n", root_pme,
            (pme_tostr(dbgbuf, DBGSZ, root_pme, PM_LVL_ROOT), dbgbuf)
    );

    x86_set_reg("cr3", root_pme);
}

int init_cpu_pm(pme_t root_pme)
{
    int res;

    int intrs_enabled = intr_isenabled();
    intr_setenabled(0);

    const size_t DBGSZ = 256;
    char         dbgbuf[DBGSZ];

    pr_debug(
            "current page mapping mode: %s\n",
            (x86_pm_cpumode_tostr(dbgbuf, DBGSZ), dbgbuf)
    );

    ureg_t cr0, cr4;
    x86_get_reg("cr0", cr0);
    x86_get_reg("cr4", cr4);
    int pgbit = cr0 & CR0_PG;
    int pae   = cr4 & CR4_PAE;

    res = -ENOTSUP;
    if (pgbit && pae) goto exit;

    if (pgbit) {
        pr_info("page mapping already enabled; setting root...\n");
        x86_set_reg("cr3", root_pme);

    } else {
        pr_info("turning on 32-bit paging...\n");

        cr0 |= CR0_PG;
        cr4 &= ~(CR4_PAE | CR4_PSE);

        if (!pme_paddr(root_pme, PM_LVL_ROOT)) return -EINVAL;
        x86_set_reg("cr4", cr4);
        x86_set_reg("cr3", root_pme);
        x86_set_reg("cr0", cr0);
    }

    pr_info("updated page mapping mode: %s\n",
            (x86_pm_cpumode_tostr(dbgbuf, DBGSZ), dbgbuf));

    res = 0;
exit:
    intr_setenabled(intrs_enabled);
    return res;
}
