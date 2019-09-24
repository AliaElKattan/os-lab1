# entry.S
##

	.data


	.text
	.code32

	.globl	kernel_entry

// set up stack frame
/*push %ebp
mov %esp, %ebp
...

mov %ebp,%esp
pop %ebp
 */
kernel_entry:
//Saves registers + switches to kernel stack

//setting up stack frame

    push %ebp
    mov %esp, %ebp

store_pcb:
/*typedef struct pcb {
    int eip;
 
    int ecx;
    int edx;
    int esi;
    int edi;

    int ebp; // current stack cursor = end of stack in use
    int esp;// top of stack
    int ebx;
    int eax;
    // int eflags;
    bool isProcess; // check if process
    int state;
} pcb_t;*/



    push %eax
    push %ebx
    movl current_running, %eax //$??
    movl 4(%ebp),%ebx
// mov return addr to eip in pcb
    movl %ebx,(%eax)

//storing eflag into [current->esp - 4]
    mov %ebp,%esp
    pop %ebp
    pop %ebx
    pushf

    push %ebp
    mov %esp,%ebp
    subl $8,%esp

    addl $4, %eax
    movl %ecx,(%eax)
    addl $4, %eax
    movl %edx,(%eax)
    addl $4, %eax
    movl %esi,(%eax)
    addl $4, %eax
    movl %edi,(%eax)
    addl $4, %eax
//storing old ebp value
    movl (%ebp),%ebx
    movl %ebx,(%eax)
    addl $4, %eax
//storing old esp value
    mov %ebp,%ebx
    addl $8,%ebx
    movl %ebx,(%eax)
//storing ebx, eax
    pop %ebx
    addl $4, %eax
    movl %ebx,(%eax)
    movl %eax,%ebx
    pop %eax
    movl %eax,(%ebx)
    mov %ebp,%esp
    pop %ebp
// call helper to execute do_yield / do_exit
    push 4(%esp)
    call kernel_entry_helper

    ret

    .global restore_pcb
/*typedef struct pcb {
 int eip;
 
 int ecx;
 int edx;
 int esi;
 int edi;
 
 int ebp; // current stack cursor = end of stack in use
 int esp;// top of stack
 int ebx;
 int eax;
 // int eflags;
 } pcb_t;*/
restore_pcb:
    //setting up stack frame
//    push %ebp
  //  mov %esp,%ebp

    mov current_running,%eax
    mov 4(%eax),%ecx
    mov 8(%eax),%edx
    mov 12(%eax),%esi
    mov 16(%eax),%edi
    mov 20(%eax),%ebp
    mov 24(%eax),%esp
//restoring eflag
    subl $4,%esp
    popf

    mov 28(%eax),%ebx
    mov 32(%eax),%eax //???
// jmp current ->eip , in scheduler

    ret


	.globl	scheduler_entry
scheduler_entry:

//storing everything






	ret
