.segment bootloader
; just been given control by the BIOS
; print hello world... I have no screen :(

.dw $55AA


code:

str:
.ascii "Hello World!"
