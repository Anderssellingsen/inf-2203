#ifndef X86_SEG_H
#define X86_SEG_H

#include <core/compiler.h>

#include <stddef.h>
#include <stdint.h>

#define X86ST_CODE_R   0x1a ///< Segment type: Readable code segment
#define X86ST_DATA_W   0x12 ///< Segment type: Writable data segment
#define X86ST_TSS      0x09 ///< Segment type: Task State Segment (TSS)
#define X86ST_INTRGATE 0x0e ///< Desc type: Interrupt Gate
#define X86ST_TRAPGATE 0x0f ///< Desc type: Trap Gate

int x86st_tostr(char *buf, size_t n, unsigned segtype);

/** x86 "Pseudo Descriptor" used to load/read a descriptor table register */
struct ATTR_PACKED x86_pseudodesc32 {
    uint16_t limit;
    void    *base;
};

/** Segment Descriptor indexes for the kernel's GDT */
enum ksegidx {
    KSEG_NULL = 0,    ///< Required NULL descriptor
    KSEG_KERNEL_CODE, ///< Code at kernel level
    KSEG_KERNEL_DATA, ///< Data at kernel level
    KSEG_USER_CODE, ///< Code at user level
    KSEG_USER_DATA, ///< Data at user level
    KSEG_TSS,       ///< Single TSS used to set kernel stack location
};

/** x86 16-bit Segment Selector to choose a segment in a descriptor table */
typedef uint16_t x86_segsel_t;
#define X86_SEGSEL_INIT(idx, rpl) (((idx) << 3) | ((rpl) &0x3))
#define X86_SEGSEL_IDX(segsel)    ((segsel) >> 3)

int x86_segsel_tostr(char *buf, size_t n, x86_segsel_t segsel);

/**
 * Read a register with MOV (esp segment register)
 *
 * @param   REG     String constant register name to be used the inline
 *                  assembly, e.g. "cs" or "cr0".
 * @param   OUT     Variable to store the value in.
 */
#define x86_get_reg(REG, OUT) \
    asm inline volatile("mov	%%" REG ", %[out]" : [out] "=r"(OUT))

/**
 * Set a register with MOV (esp segment register)
 *
 * @param   REG     String constant register name to be used the inline
 *                  assembly, e.g. "cs" or "cr0".
 * @param   IN      Value to put into the register.
 */
#define x86_set_reg(REG, IN) \
    asm inline volatile("mov	%[in], %%" REG ::[in] "r"(IN))

/**
 * Load the CS register by doing a far jump to the next instruction
 *
 * The CS register can not be set directly. You have to do a far jump or far
 * call. So here we load the CS register by doing a far jump to the next
 * instruction.
 *
 * The "%=" bit in the asm generates a unique number that we can use
 * as a label. The generated assembly will look something like this:
 *
 * ```
 *          ljmp    $8, $1117f  // Far jump forward to label 1117...
 *  1117:                       // ... which is the next instruction.
 * ```
 *
 * @param SEGSEL    Segment selector to use. Must be a compile-time constant.
 */
#define x86_set_cs(SEGSEL) \
    asm inline volatile( \
            "\n" \
            "	ljmp    %[cs_sel], $%=f\n" /* Far jump forward ... */ \
            "%=:"                          /* ... to generated label */ \
            : \
            : [cs_sel] "i"(SEGSEL) \
    )

#endif /* X86_SEG_H */
