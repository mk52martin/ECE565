/* resched.c - resched, resched_cntl */
#define DEBUG_CTXSW 	1
#define MLFQ			1
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

	#if MLFQ
	if(ptold->timeallotment > TIME_ALLOTMENT) {
		demote(currpid);
	}
	if (ptold->prstate == PR_CURR) {  /* Process remains eligible */
		/* Old process will no longer remain current */
		ptold->prstate = PR_READY;
		insert(currpid, ptold->queue, ptold->prprio);
	}
	//sync_printf("currpid = %d, status: %d, ptqueue = %d\n", currpid, ptold->prstate, ptold->queue);
	// if(currpid == 4 || currpid == 5 || currpid == 6){
	// 	print_ready_list();
	// }

	if(!check_empty(readylist_service)) {
		currpid = dequeue(readylist_service);
		preempt = QUANTUM;
	} else if (!check_empty(readylist_high)) {
		currpid = dequeue(readylist_high);
		preempt = QUANTUM;
	} else if (!check_empty(readylist_med)) {
		currpid = dequeue(readylist_med);
		preempt = QUANTUM * 2;
	} else if (!check_empty(readylist_low)) {
		currpid = dequeue(readylist_low);
		preempt = QUANTUM * 4;
	} else {
		currpid = 0;
		preempt = QUANTUM * 4;
	}
	ptnew = &proctab[currpid];
	ptnew->prstate = PR_CURR;
	
	if(ptnew == ptold) {
		return;
	}
	#endif
	#if !MLFQ
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
	#endif


	ptnew->num_ctxsw++;		
	#ifdef DEBUG_CTXSW
	sync_printf("ctxsw::%d-%d\n", ptold->pid, ptnew->pid);
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

void demote(pid32 pid) {
	qid16 newq;
	struct	procent	*prptr;	
	prptr = &proctab[pid];
	prptr->timeallotment = 0;
	if(pid == 0) {
		return;
	}
	if(prptr->queue == readylist_low) {
		return;
	}
	if(prptr->queue == readylist_service) {
		return;
	}
	sync_printf("Demote %d.\n", pid);
	if(prptr->queue == readylist_high) {
		newq = readylist_med;
	} else if (prptr->queue == readylist_med) {
		newq = readylist_low;
	}
	prptr->queue = newq;
	//getitem(pid);
	//insert(pid, newq, prptr->prprio);
	return;
}

void boost_priority(void) {
	int i = 0;
	struct	procent	*prptr;	
	while(i < NPROC) {
		prptr = &proctab[i];
		if(prptr->queue != readylist_service){
			if(prptr->prstate == PR_READY) {
				getitem(i);
				insert(i, readylist_high, prptr->prprio);
			}
			prptr->queue = readylist_high;
		}
		i++;
	}
	//print_ready_list();
	return;
}