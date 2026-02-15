	.text
	.global _start
_start:
	xor	%ebp, %ebp	// Zero frame pointer to mark deepest frame
	push	%ebp
	mov	%esp, %ebp

	/*
	 * System V process start stack frame (after pushing EBP)
	 *
	 *	ebp+ 8	start of argv vector
	 *	ebp+ 4	argc passed by OS
	 *		(the typical return address is missing)
	 *	ebp+ 0	old EBP
	 */

	/* Call main */
	lea	+8(%ebp), %eax	// Get pointer to start of argv vector
	push	%eax		// Pass argv pointer
	push	+4(%ebp)	// Pass argc
	call	main
	add	$(2*4), %esp

	/* Call exit */
	push	%eax		// Pass main's return value to _exit
	call	_exit
	add	$(1*4), %esp

	/* The exit call should not return, but if it does, try to halt.
	 * This is a privileged instruction, so it will either succeed
	 * and halt the processor, or it will cause a protection fault
	 * and return control to the kernel. */
1:	hlt
	jmp	1b
