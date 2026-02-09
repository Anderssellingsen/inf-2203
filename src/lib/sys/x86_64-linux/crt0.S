	.global	_start
_start:
	endbr64
	xor	%rbp, %rbp	// Zero frame pointer to mark deepest frame
	push	%rbp
	mov	%rsp, %rbp
	and	$~0xf,%rsp	// Ensure that stack is 16-byte aligned

	/*
	 * System V process start stack frame (after pushing EBP)
	 *
	 *	ebp+16	start of argv vector
	 *	ebp+ 8	argc passed by OS
	 *		(the typical return address is missing)
	 *	ebp+ 0	old EBP
	 */

	/* Call main */
	lea	+16(%rbp), %rsi	// Pass argv pointer
	mov	+8(%rbp), %rdi	// Pass argc
	call	main

	/* Call exit */
	mov	%rax, %rdi	// Pass main's return value to _exit
	call	_exit

	/* The exit call should not return, but if it does, try to halt.
	 * This is a privileged instruction, so it will either succeed
	 * and halt the processor, or it will cause a protection fault
	 * and return control to the kernel. */
1:	hlt
	jmp	1b
