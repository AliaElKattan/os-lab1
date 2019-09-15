

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

	jmp     move_up_bootblock
os_size:
    # Area reserved for createimage to write the OS size
	.word   0
	.word   0

move_up_bootblock:
	#movsb - DS:SI to ES:DI
	#set counter reg to 512
	movw $512,%cx 
	
	#set DS:SI to 0x7c00 (src addr of bootblock)
	movw $BOOT_SEGMENT, %ax
	movw %ax,%ds
	movw $0,%si

	#set ES:DI(destination of movsb) to 0xa0000 - 512(0x200) = 0x9fe00
	movw $0x9FE0,%ax
	movw %ax,%es
	movw $0,%di
	# while(%cx!=0){movsb; %cx--;} , moving 512 bytes
	rep movsb 

load:
	# ES:BX - pointer to the dest of BIOS int0x13 
	# set dest addr for kernel to 0x1000
	movw $0x100,%ax
	mov %ax,%es
	movw $0x0000,%bx

	#lodsw - DS:SI to AX
	#load the size of the kernel to ax
	movw $os_size,%si
	lodsw 
	pushw %ax

check_size: 
	#get remaining os_size to be copied from stack
	popw %ax 
	cmpb $0,%al
	je check_copy

	#stores remaining os_size to be copied to %bl
	mov %al,%bl
	
	#if remaining > 64 sectors, only copy the first 64 and leave the rest to the next turn
	cmpb $64,%bl
	jle copy
	movb $64,%al
	
copy:
	# subtract # of sectors being copied this turn from %bl (remaining sectors to be copied) and store the outcome to the stack
	subb %al,%bl
	pushw %bx
	#to complete bios function
	movb $0x2, %ah

	#number of sectors to read is already in %al
	
	#cylinder number
	movb $0x00, %ch
	#sector number (only the kernel)
	movb $0x2,%cl

        #starting head number
	movb $0x00, %dh

	#drive number
	movb $0x0,%dl

	#pointer to memory

	int $0x13
	jmp check_size
 
check_copy:	#check if BIOS int0x13 successes and print success / fail message 
	#store what's returned in %ah to the stack	
	push %ax

	#print message signifying the end of copying 
	movw $string, %si
	call print
	#get returned %ah value from the stack
	pop %ax
	
	#check if int0x13 successes ( if %ah = 0, successes) and print message accordingly
	cmpb $0,%ah
	jne print_fail
	movw $sstring, %si
	call print
	jmp stack_setup

print_fail: #printing fail message
	movw $fstring, %si
	push %ax
	call print
	pop %ax
	cmpb $11, %ah
	jne forever
	movw $message,%si
	call print

stack_setup:
	# set SS:SP to 0x9FE00 -2
	movw $0x9000,%ax
	movw %ax,%ss
	movw $0xFE00,%sp
	subw $2,%sp
	

kernel_jump:
	# set DS to 0x0
	mov $0x0,%ax
	mov %ax,%ds
	
	# ljmp CS, IP (jump to kernel at 0x1000)
	ljmp $0x0,$0x1000


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


