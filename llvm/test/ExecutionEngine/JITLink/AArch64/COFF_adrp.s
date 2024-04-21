	.text
	.def	main;
	.scl	2;
	.type	32;
	.endef
	.globl	main
	.p2align	2
main:
	adrp	x8, retv
	ldr	w0, [x8, :lo12:retv]
	ret

	.bss
	.globl	retv
	.p2align	2, 0x0
retv:
	.word	0