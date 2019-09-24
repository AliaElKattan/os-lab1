/* kernel.h: definitions used by kernel code */

#ifndef KERNEL_H
#define KERNEL_H

#include "common.h"

/* ENTRY_POINT points to a location that holds a pointer to kernel_entry */
#define ENTRY_POINT ((void (**)(int)) 0x0f00)

/* System call numbers */
enum {
    SYSCALL_YIELD,
    SYSCALL_EXIT,
};

/* All stacks should be STACK_SIZE bytes large
 * The first stack should be placed at location STACK_MIN
 * Only memory below STACK_MAX should be used for stacks
 */
enum {
    STACK_MIN = 0x10000,
    STACK_SIZE = 0x1000,
    STACK_MAX = 0x20000,
};

/* The process control block is used for
 * - saving the contents of the registers
 * - maintaining queues of tasks
 * - ...
 */
enum {READY,YIELD,EXITTED};
typedef struct pcb {
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
     int isProcess; // check if process
     int state;
} pcb_t;

/* Queue of tasks that are ready to run */
extern pcb_t ready_queue;
/* The task currently running */
extern pcb_t *current_running;
extern pcb_t pcbs[2];
void kernel_entry(int fn);
void kernel_entry_helper(int fn);


#endif                          /* KERNEL_H */
