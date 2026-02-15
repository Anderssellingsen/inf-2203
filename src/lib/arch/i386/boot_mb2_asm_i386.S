/**
 * Multiboot2 kernel start
 *
 * This is the true start point of the kernel.
 */
	.text
	.global _start
	.weak	_start
_start:
	/* Save values passed by the bootloader and call into C. */
	push	%ebx	// Pass pointer to MB2 data in EBX
	push	%eax	// Pass magic number in EAX
	call	_start_mb2
	add	$2*4,	%esp

	/* If kernel returns, something is wrong.
	 * Nothing else to do but try returning to the bootloader. */
	ret

#define ASM_FILE 1
#include <oss/multiboot2.h>

/**
 * Multiboot2 header data
 */
	.global mb2_header
	.align	8
mb2_header:
	/* Multiboot2 header: magic, architecture, header size, and checksum */
	.long	MULTIBOOT2_HEADER_MAGIC
	.long	MULTIBOOT_ARCHITECTURE_I386
	.long	mb2_header_end - mb2_header
	.long	0-(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 + \
			(mb2_header_end - mb2_header))

	/* End of header tags. */
	.short MULTIBOOT_HEADER_TAG_END
	.short 0
	.long 8
mb2_header_end:
