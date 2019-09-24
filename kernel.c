/* kernel.c */


#include "common.h"

#include "th.h"
#include "tasks.c"
#include "kernel.h"

#include "scheduler.h"

#include "util.h"

extern void kernel_exit(void);


pcb_t *current_running;
pcb_t pcbs[2]; // here??

/* This function is the entry point for the kernel
 * It must be the first function in the file
 */
void _start(void)
{
    /* Set up the entry point for system calls */
    *ENTRY_POINT = &kernel_entry;

    clear_screen(0, 0, 80, 25);

    /* Initialize the pcbs and the ready queue */
    //static pcb_t pcbs[NUM_TASKS];
    ASSERT((STACK_MIN + STACK_SIZE *NUM_TASKS) <= STACK_MAX);
    int stack_portion = 0; // beginning of the stack for each task
    int taskIndex;
   // pcbs = pcb_t[NUM_TASKS];
    for(taskIndex = 0; taskIndex < 2; taskIndex ++){
        pcb_t current = pcbs[taskIndex];  ///pcb??? //pointer??
        struct task_info* info = task[3+taskIndex];
      /*  current.eax = 0;
        current.ebx = 0;
        current.ecx = 0;
        current.edx = 0;
        current.esi = 0;
        current.edi = 0;*/
        stack_portion+= STACK_SIZE;
        current.ebp = stack_portion;
        current.esp = stack_portion; //how to allocate kernel stack??
        current.eip = info->entry_point;
        current.isProcess = (int)(info->task_type == PROCESS);
        current.state = READY;
        // add to ready queue
        
        
    }
    
    /* Schedule the first task */
    //current_running = &pcbs[0];
  //  scheduler_count = 0;
  //  scheduler_entry();
    
    current_running = &pcbs[0];
    //set stack for process when run the first time
    
    
    void (*fun_ptr)()= (uint32_t)current_running->eip;
    fun_ptr();

    /* We shouldn't ever get here */
    ASSERT(0);
}

/* entry.S:kernel_entry calls this function to place syscall number fn from inside the kernel */
void kernel_entry_helper(int fn)
{
    switch (fn) {
    default:
        print_str(1, 1, "Invalid system call: process exiting");
        /* Fall through */
    case SYSCALL_EXIT:
        do_exit();
    case SYSCALL_YIELD:
        do_yield();
    }
}

