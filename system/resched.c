/* resched.c - resched, resched_cntl */
#define DEBUG_CTXSW 1
#include <xinu.h>

struct	defer	Defer;

/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* Assumes interrupts are disabled	*/
{
	struct procent *ptold;	/* Ptr to table entry for old process	*/
	struct procent *ptnew;	/* Ptr to table entry for new process	*/

	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	/* Point to process table entry for the current (old) process */

	ptold = &proctab[currpid];

	if (ptold->prstate == PR_CURR) {  /* Process remains eligible */
		if (ptold->prprio > firstkey(readylist_service)) {
			return;
		}

		/* Old process will no longer remain current */

		ptold->prstate = PR_READY;
		insert(currpid, readylist_service, ptold->prprio);
	}

	/* Force context switch to highest priority ready process */

	currpid = dequeue(readylist_service);
	ptnew = &proctab[currpid];
	ptnew->prstate = PR_CURR;
	preempt = QUANTUM;		/* Reset time slice for process	*/
	ptnew->num_ctxsw++;		
#ifdef DEBUG_CTXSW
	printf("ctxsw::%d-%d\n", ptold->pid, ptnew->pid);
#endif
	ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
	
	/* Old process returns here when resumed */

	return;
}

/*------------------------------------------------------------------------
 *  resched_cntl  -  Control whether rescheduling is deferred or allowed
 *------------------------------------------------------------------------
 */
status	resched_cntl(		/* Assumes interrupts are disabled	*/
	  int32	defer		/* Either DEFER_START or DEFER_STOP	*/
	)
{
	switch (defer) {

	    case DEFER_START:	/* Handle a deferral request */

		if (Defer.ndefers++ == 0) {
			Defer.attempt = FALSE;
		}
		return OK;

	    case DEFER_STOP:	/* Handle end of deferral */
		if (Defer.ndefers <= 0) {
			return SYSERR;
		}
		if ( (--Defer.ndefers == 0) && Defer.attempt ) {
			resched();
		}
		return OK;

	    default:
		return SYSERR;
	}
}


/*------------------------------------------------------------------------
 *  print_ready_list  -  Print list of ready functions (readylist)
 *------------------------------------------------------------------------
 */

syscall print_ready_list() {
	// disable interrupts
	intmask mask;
	mask = disable();
	
	sync_printf("List of ready processes from readylist_service:\n");						
	print_queue(readylist_service);

	sync_printf("List of ready processes from readylist_high:\n");						
	print_queue(readylist_high);

	sync_printf("List of ready processes from readylist_med:\n");						
	print_queue(readylist_med);

	sync_printf("List of ready processes from readylist_low:\n");						
	print_queue(readylist_low);

	//reenable interrupts
	restore(mask);
	return OK;
}


void print_queue(qid16 q) {
		//print readylist
	qid16 tail = queuetail(q);												//find head
	qid16 it = firstid(q);	
	if(it == tail) {
		sync_printf("Empty.\n");
		return;
	}							
	sync_printf("%d", it);
	while(queuetab[it].qnext != tail) {												// cycle through readylist
		it = queuetab[it].qnext;
		sync_printf(", %d", it);	
	}
	
	// if(it != firstid(q)) {													// print tail if >1 process
	// 	sync_printf(", %d", it);
	// }
	sync_printf("\n");
}

bool8 check_empty(qid16 q) {
	qid16 tail = queuetail(q);												//find head
	qid16 it = firstid(q);	
	if(it == tail) {
		return TRUE;
	}
	return FALSE;
}