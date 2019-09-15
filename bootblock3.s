

    .equ    BOOT_SEGMENT,0x07c0
    .equ    DISPLAY_SEGMENT,0xb800

.text               # Code segment
.globl    _start    # The entry point must be global
.code16             # Real mode

#
# The first instruction to execute in a program is called the entry
# point. The linker expects to find the entry point in the "symbol" _start
# (with underscore).
#
_start:

    jmp     move_up_kernel
os_size:
    # Area reserved for createimage to write the OS size
    .word   0
    .word   0

move_up_kernel:
	movw $512,%cx #set counter reg to 512
	movw $BOOT_SEGMENT, %ax
	movw %ax,%ds
	movw $0,%si
	movw $0x09FE,%ax
	movw %ax,%es
	movw $2,%di
	rep movsb # while(%cx!=0){movsb; %cx--;}

load:
	movw $0x100,%ax
	mov %ax,%es
	movw $0x0000,%bx
	movw $os_size, %ax
	pushw %ax

check_size: 
	popw %ax #%al already contains the os_size
	cmpb $0,%al
	je check_copy
	mov %al,%bl
	cmpb $64,%bl
	jle copy
	movb $64,%al
	
copy:
	subb %al,%bl
	pushw %bx
	#to complete bios function
	movb $0x2, %ah

	#number of sectors to read
	
	#cylinder number
	movb $0x00, %ch
	#sector number
	movb $0x2,%cl

        #starting head number
	movb $0x00, %dh

	#drive number
	movb $0x0,%dl

	#pointer to memory

	int $0x13
	jmp check_size
 
check_copy:
    push %ax
    movw $string, %si
    call print
    pop %ax
    cmpb $0,%ah
    jne print_fail
    movw $sstring, %si
    call print
    jmp stack_setup

print_fail:
    movw $fstring, %si
    push %ax
    call print
    pop %ax
    cmpb $11, %ah
    jne forever
    movw $message,%si
    call print

stack_setup:
	movw $0x09FE,%ax
	mov %ax,%ss
	movw $0x0,%sp

kernel_jump:
	ljmp $0x100,$0


forever:
    jmp forever

print:
# routine to print a zero terminated string pointed to by esi
    
print_loop:
    lodsb
    cmpb $0,%al
    je print_done
    movb $14,%ah
    int $0x10
    jmp print_loop
print_done:
    retw

string:
   .asciz  "Reach the end of loading!\n\r"
sstring:
   .asciz  "Successful!\n\r"
fstring:
   .asciz  "Failed!\n\r"
message:
   .asciz  "ah is 11\n\r"
#0x55 0xaa

