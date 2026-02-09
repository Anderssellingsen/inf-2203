#include <errno.h>		// Get error codes
#if __linux__
#include <asm/unistd_64.h>	// Get Linux syscall numbers
#endif

#if __linux__
	.equ	IVEC_SYSCALL,		0x80
#endif
#if __munix__
	.equ	IVEC_SYSCALL,		48
#endif

	/*
	 * System V args:	    f+8 +12 +16 +20 +24 +28 +32
	 * Linux syscall:	    eax ebx ecx edx esi edi ebp
	 */
	.text
	.global	syscall
	.type	syscall, @function
syscall:
	push	%ebp
	mov	%esp, %ebp

	/* Save nonvolatile registers. */
	push	%ebx
	push	%esi
	push	%edi

	mov	 +8(%ebp), %eax		// fn arg 1 -> syscall number
	mov	+12(%ebp), %ebx		// fn arg 2 -> syscall arg 1
	mov	+16(%ebp), %ecx		// fn arg 3 -> syscall arg 2
	mov	+20(%ebp), %edx		// fn arg 4 -> syscall arg 3
	mov	+24(%ebp), %esi		// fn arg 5 -> syscall arg 4
	mov	+28(%ebp), %edi		// fn arg 6 -> syscall arg 5
	push	%ebp
	mov	+32(%ebp), %ebp		// fn arg 7 -> syscall arg 6
	int	$IVEC_SYSCALL
	pop	%ebp

	/* Restore registers. */
	pop	%edi
	pop	%esi
	pop	%ebx

	pop	%ebp
	ret

