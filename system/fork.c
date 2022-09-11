/* fork.c - fork */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  create  -  Create a process to start running a function on x86
 *------------------------------------------------------------------------
 */
pid32 fork()
{
    struct procent *parent_prptr = &proctab[currpid];
	//printf("Ptr: %x\n", parent_prptr->prstkptr);
    unsigned long *it;
    stacktrace(currpid);
    asm("movl %%ebp, %0\n" :"=r"(it));
    it++;
    unsigned long *pass = (*it);
    printf("@(%d) %d", pass, *pass);
    pid32 pid;
    struct procent *prptr;
    pid = create(
        pass,
        parent_prptr->prstklen,         // Stack Size
        parent_prptr->prprio,           // Stack Priority
        parent_prptr->prname,           // Stack Name
        1                           //TODO: Fix to arg number.
    );
    if(pid == SYSERR) return SYSERR;


    //stacktrace(pid);
    prptr = &proctab[pid];
    //child EAX
    *((prptr->prstkbase)-24) = NPROC;
    
    resume(pid);
    //printf("resumed!");
	return pid;
}
