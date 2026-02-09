	.text
	.global isr_syscall
isr_syscall:
	/* Translate from syscall ABI (args in registers)
	 * to System V ABI (args on stack). */
	push	%edi	# arg5
	push	%esi	# arg4
	push	%edx	# arg3
	push	%ecx	# arg2
	push	%ebx	# arg1
	push	%eax	# Syscall number

	/* Call into C to complete transition to kernel space
	 * and dispatch syscall. */
	call	isr_syscall_stage2
	add	$(6*4), %esp

	/* Return value is already in EAX. */
	iret
