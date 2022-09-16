/* kill.c - kill */

#include <xinu.h>

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
	//sync_printf("Killing %d...\n", pid);
	mask = disable();
	if (isbadpid(pid) || (pid == NULLPROC)
	    || ((prptr = &proctab[pid])->prstate) == PR_FREE) {
		restore(mask);
		return SYSERR;
	}

	// cascading ternmination
	struct	procent *pot_child;
	prptr = &proctab[pid];
	bool8 cascading = FALSE;
	if(prptr->pr_user) {
		int j = 0;
		for(; j < NPROC; j++) {
			pot_child = &proctab[j];
			if(pot_child->prparent == pid) {
				//sync_printf("Process %d Kill: %d\n", currpid, j);
				kill(j);
				
				cascading = TRUE;
			}
		}
	}
	
	prptr = &proctab[pid];
	
	if (--prcount <= 1) {		/* Last user process completes	*/
		xdone();
	}

	if(!cascading){
		//sync_printf("return message\n");
		send(prptr->prparent, pid);
	}
	prptr->prparent = -1;
	for (i=0; i<3; i++) {
		close(prptr->prdesc[i]);
	}
	freestk(prptr->prstkbase, prptr->prstklen);

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
	//sync_printf("Killed %d\n", pid);
	return OK;
}
