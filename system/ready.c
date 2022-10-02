/* ready.c - ready */

#include <xinu.h>

qid16	readylist_service;			/* Index of ready list		*/
qid16	readylist_high;				/* Index of high prio ready list*/
qid16	readylist_med;				/* Index of med prio ready list*/
qid16	readylist_low;				/* Index of low prio ready list*/

/*------------------------------------------------------------------------
 *  ready  -  Make a process eligible for CPU service
 *------------------------------------------------------------------------
 */
status	ready(
	  pid32		pid		/* ID of process to make ready	*/
	)
{
	register struct procent *prptr;

	if (isbadpid(pid)) {
		return SYSERR;
	}

	/* Set process state to indicate ready and add to ready list */

	prptr = &proctab[pid];
	prptr->prstate = PR_READY;
	//print_ready_list();
	insert(pid, readylist_service, prptr->prprio);
	//print_ready_list();
	resched();

	return OK;
}
