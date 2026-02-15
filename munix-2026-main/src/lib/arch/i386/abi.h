#ifndef ABI_H
#define ABI_H

#define STACK_DOWN 1
#define STACK_UP   2

#if STACK_DIR == STACK_DOWN
#define PUSH(sp, val) *--(sp) = (val)
#elif STACK_DIR == STACK_UP
#define PUSH(sp, val) *++(sp) = (val)
#endif

#endif /* ABI_H */
