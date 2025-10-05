.segment bios

MSR = 1
FIFO = 2

floppy_read:
    ;       expected stack
    ; SP+7->sector num
    ;       track num
    ;       head num
    ;       drive num
    ;       mem low num
    ;       mem high num
    ;       return address low
    ;   SP->return address high
    mov h, [sp + 2]         ; start FDC to read sector
    mov l, [sp + 3]
    outb FIFO, #1          ; command read sector
    mov r0, [sp + 4]    ; drive num
    mov r1, [sp + 5]    ; head num
    mov r2, [sp + 6]    ; track num
    mov r3, [sp + 7]    ; sector num
    outb FIFO, r0          ; push values to FIFO
    outb FIFO, r1
    outb FIFO, r2
    outb FIFO, r3
    ; all registers are released after this sequence above
    mov r3, #$2
    add r3, h   ; create an upper pointer for end of sector data
l0:
    inb r0, MSR ; read MSR
    and r0, #%00100000 ; check if read ready
    jz l0 ; if read ready continue, else keep waiting

    inb r1, FIFO ; read from FIFO

    mov (h,l), r1 ; store read byte to memory

    inc l ; increment lower byte of sector load location pointer
    jc l1 ; if carry increment upper byte of sector load location pointer
    jmp l0
l1:
    inc h ; increment upper byte of sector load location pointer
    cmp h, r3
    jz l2 ; if reached the end of sector
    jmp l0

l2:
    inb r0, FIFO    ; read status byte
    ret             ; pointer to mem location in HL if required

floppy_write:
    ;       expected stack
    ; SP+7->sector num
    ;       track num
    ;       head num
    ;       drive num
    ;       mem low num
    ;       mem high num
    ;       return address low
    ;   SP->return address high
    mov h, [sp + 2]         ; start FDC to read sector
    mov l, [sp + 3]
    outb FIFO, #1          ; command read sector
    mov r0, [sp + 4]    ; drive num
    mov r1, [sp + 5]    ; head num
    mov r2, [sp + 6]    ; track num
    mov r3, [sp + 7]    ; sector num
    outb FIFO, r0          ; push values to FIFO
    outb FIFO, r1
    outb FIFO, r2
    outb FIFO, r3
    ; all registers are released after this sequence above
    mov r3, #$2
    add r3, h   ; create an upper pointer for end of sector data
    mov r2, (h,l) ; buffer first data byte
    inc l
l3:
	inb r0, MSR
	and r0, #%00100000 ; check if write ready
	jz l3

	outb FIFO, r3 ; write to FIFO

	inc l
	jc l4
	jmp l3

l4:
	inc h
	cmp h, r3
	jz l5
	jmp l3
l5:
	inb r0, MSR
	ret


.segment code

drive_num:
	.db 0

load_drive:
	mov r0, drive_num

	push #1 ; sector num
	push #0 ; track num
	push #0 ; head num
	push r0 ; drive num
	push #0 ; load location low index
	push #$10 ; load location high index
	; call to read floppy disk
	call floppy_read
	add sp, #$6 ; clean stack

	; move pointer to start of file
	sub h, #$2

	ret

inc_drive_num:
	mov r0, drive_num
	inc r0
	mov drive_num, r0
	jmp find_bootable

start:

	; look for program to hand control over
	; push args
find_bootable:
	call load_drive
	; check if disk is bootable
	mov r0, $1000
	mov r1, $1001
	and r0, #$AA
	jnz inc_drive_num
	and r1, #$55
	jnz inc_drive_num

give_control:
	add h, #2
	jmp (h, l) ; go to read program

	hlt ; fallback

.segment vec_table
.dw start ; reset vec
