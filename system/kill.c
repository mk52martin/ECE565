/* kill.c - kill */

#include <xinu.h>
#define DISPLAY_TURNAROUND_TIME 0
#define DISPLAY_ARRIVAL_CURR_TIME 0
#define DISPLAY_CTXSW_TO_PROCESS 0

/*------------------------------------------------------------------------
 *  kill  -  Kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
syscall	kill(
	  pid32		pid		/* ID of process to kill	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	int32	i;			/* Index into descriptors	*/
	//sync_printf("Kill %d\n", pid);
	mask = disable();
	if (isbadpid(pid) || (pid == NULLPROC)
	    || ((prptr = &proctab[pid])->prstate) == PR_FREE) {
		restore(mask);
		return SYSERR;
	}

	if (--prcount <= 1) {		/* Last user process completes	*/
		xdone();
	}

	send(prptr->prparent, pid);
	for (i=0; i<3; i++) {
		close(prptr->prdesc[i]);
	}
	freestk(prptr->prstkbase, prptr->prstklen);

	#if DISPLAY_ARRIVAL_CURR_TIME
	kprintf("Process %d arrival time: %d\n", pid, prptr->arrivaltime);
	kprintf("Current Time: %d\n", ((clktime*1000) + ctr1000));
	#endif

	// update turnaround time
	prptr->turnaroundtime = (clktime*1000) + ctr1000 - prptr->arrivaltime;

	#if DISPLAY_TURNAROUND_TIME
	kprintf("Process %d turnaround time: %d\n", pid, prptr->turnaroundtime);
	#endif
	#if DISPLAY_CTXSW_TO_PROCESS
	kprintf("Process %d was switched to %d times.\n", pid, prptr->num_ctxsw);
	#endif

	switch (prptr->prstate) {
	case PR_CURR:
		prptr->prstate = PR_FREE;	/* Suicide */
		resched();

	case PR_SLEEP:
	case PR_RECTIM:
		unsleep(pid);
		prptr->prstate = PR_FREE;
		break;

	case PR_WAIT:
		semtab[prptr->prsem].scount++;
		/* Fall through */

	case PR_READY:
		getitem(pid);		/* Remove from queue */
		/* Fall through */

	default:
		prptr->prstate = PR_FREE;
	}

	restore(mask);
	return OK;
}
