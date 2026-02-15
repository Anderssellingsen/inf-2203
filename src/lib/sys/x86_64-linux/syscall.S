#include <errno.h>		// Get error codes
#if __linux__
#include <asm/unistd_64.h>	// Get Linux syscall numbers
#endif

	/*
	 * System V args:	%rdi %rsi %rdx %rcx %r8  %r9 s+8
	 * Linux syscall:	%rax %rdi %rsi %rdx %r10 %r8 %r9
	 */
	.text
	.global	syscall
	.type	syscall, @function
syscall:
	/* Shift registers from incoming call to syscall */
	mov	%rdi, %rax	// fn arg 1 -> syscall number
	mov	%rsi, %rdi	// fn arg 2 -> syscall arg 1
	mov	%rdx, %rsi	// fn arg 3 -> syscall arg 2
	mov	%rcx, %rdx	// fn arg 4 -> syscall arg 3
	mov	%r8,  %r10	// fn arg 5 -> syscall arg 4
	mov	%r9,  %r8	// fn arg 6 -> syscall arg 5
	mov	+8(%rsp), %r9	// fn arg 7 -> syscall arg 6
	syscall
	ret

