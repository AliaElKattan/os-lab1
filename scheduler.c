/* scheduler.c */

#include "common.h"
#include "kernel.h"
#include "scheduler.h"
#include "util.h"

int scheduler_count;
void restore(void);

void scheduler(void)
{   // change current_running to the next task
    ++scheduler_count;
    //pop from ready queue
    
    
    
    current_running = &pcbs[(scheduler_count % 2)];

    switch (current_running->state) {
    
        case EXITTED:
            scheduler(); //skip the current task
        case READY:
	    ;
            void (*fun_ptr)() = (uint32_t)current_running->eip;
            fun_ptr();
        case YIELD:
            restore_pcb();
            void (*fun_ptr2)() = (uint32_t)current_running->eip;
            fun_ptr2();
    }
}

void do_yield(void)
{
    current_running->state = YIELD;
    if(current_running->isProcess){
        //schedule the next task;
        scheduler();
    }
    else{ scheduler_entry();}
   
}

void do_exit(void)
{
    current_running->state = EXITTED;
    scheduler();
}

void block(pcb_t * wait_queue)
{
}

void unblock(pcb_t * p)
{
}
