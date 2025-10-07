.segment bootloader
; just been given control by the BIOS
; print hello world

GPU_BASE = 8192

.dw $55AA


code:
	call init_GPU
	; print "Hello World!"
	mov r0, #$48
	mov r1, #$F0
	mov GPU_BASE, r0
	mov GPU_BASE+1, r1

	mov r0, #$65
	mov GPU_BASE+2, r0
	mov GPU_BASE+3, r1

	mov r0, #$6C
	mov GPU_BASE+4, r0
	mov GPU_BASE+5, r1

	mov r0, #$6C
	mov GPU_BASE+6, r0
	mov GPU_BASE+7, r1

	mov r0, #$6F
	mov GPU_BASE+8, r0
	mov GPU_BASE+9, r1

	hlt

init_GPU:
	mov r0, #0
	mov r1, #15
	mov r2, #$3F
	mov GPU_BASE+400, r0 ; set VRAM page index to 0
	mov GPU_BASE+401, r0 ; set mode reg to 0

	mov GPU_BASE+402, r0 ; set palette entry index to 0
	mov GPU_BASE+403, r0 ; set background colour

	mov GPU_BASE+402, r1 ; set palette entry index 15
	mov GPU_BASE+403, r2 ; set foreground colour to white

	mov GPU_BASE+405, r0 ; set control register
	ret
