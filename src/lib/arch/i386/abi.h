#ifndef ABI_H
#define ABI_H

#define STACK_DOWN 1
#define STACK_UP   2

#if __i386__
#define STACK_DIR   STACK_DOWN
#define KSTACK_DFLT 0x10000
#define USTACK_DFLT 0x500000
#define PM_AREA  0x100000
#define KMAP_MIN 0x0
#define KMAP_MAX 0x400000
#endif /* __i386__ */

#if STACK_DIR == STACK_DOWN
#define PUSH(sp, val) *--(sp) = (val)
#elif STACK_DIR == STACK_UP
#define PUSH(sp, val) *++(sp) = (val)
#endif

#endif /* ABI_H */
