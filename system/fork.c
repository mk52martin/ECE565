/* fork.c - fork */

#include <xinu.h>
#define DEBUG_SIGNALS 1
/*------------------------------------------------------------------------
 *  create  -  Create a process to start running a function on x86
 *------------------------------------------------------------------------
 */
pid32 fork()
{
	pid32 pid;
    //pid32 parent_pid;
    //uint32 *saddr;
    //uint32 *parent_sadder;
    struct procent *parent_prptr;
    struct procent *prptr;
#ifdef DEBUG_SIGNALS
    printf("hit1");
#endif
    parent_prptr = &proctab[currpid];
    //printf("parent pid %d\n", currpid);
    //printf("INITRET = %x\n", INITRET);
    //int i = 0;
    /*
    for(i = 0; i <= 14; i++) {
        printf("%x: %x\n", (parent_prptr->prstkbase)-(i*4), *((parent_prptr->prstkbase)-(i*4)));
    }
    printf("----------");
    for(i = 0; i <= 14; i++) {
        printf("%x: %x\n", (parent_prptr->prstkbase)+(i*4), *((parent_prptr->prstkbase)+(i*4)));
    }
    */
   //*((parent_prptr->prstkbase)-4),    // FUNCADDR
   //void *ebp = *((parent_prptr->prstkbase)--);
   //printf("%x", INITRET);
#ifdef DEBUG_SIGNALS
    printf("hit2");
#endif
    pid = create(
        INITRET,
        parent_prptr->prstklen,         // Stack Size
        parent_prptr->prprio,           // Stack Priority
        parent_prptr->prname,           // Stack Name
        1                           //TODO: Fix to arg number.
    );
#ifdef DEBUG_SIGNALS
    printf("hit3");
#endif

    //printf("child pid %d\n", pid);
    //stacktrace(parent_pid);
    prptr = &proctab[pid];
    //child EAX
    *((prptr->prstkbase)-28) = NPROC;
    //printf("child proc:  %d\n", *((prptr->prstkbase)-8));
    /*
    //child EIP
    saddr = ((prptr->prstkbase)+8);
    printf("child address (++): %d\n", saddr);
    parent_sadder = ((parent_prptr->prstkbase)+8);
    printf("parent address (++): %d\n", parent_sadder);
    printf("parent base:  %d\n", *(parent_sadder));
    // *saddr = *parent_sadder;
    printf("child base:  %d\n", *(saddr));
    */
    //printf("about to resume...");
#ifdef DEBUG_SIGNALS
    printf("hit4");
#endif

    printf("%x", INITRET);
    resume(pid);
#ifdef DEBUG_SIGNALS
    printf("hit5");
#endif	
    //printf("resumed!");
	return pid;
}
