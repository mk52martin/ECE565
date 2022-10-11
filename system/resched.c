/* resched.c - resched, resched_cntl */
#define DEBUG_CTXSW 	0
#define MLFQ			1
#define PRINT_READYLIST_SERVICE 	0
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
	if(proctab[currpid].prstate != PR_CURR) {
		preempt = QUANTUM;
		quantum_counter = 0;
		// sync_printf("currpid: %d\n", currpid);
		// print_ready_list();
		// sync_printf("Sleep: ");
		// print_queue(sleepq);
	} 
	//sync_printf("%d\n", quantum_counter);

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
		if(currpid != 0){
			insert(currpid, ptold->queue, ptold->prprio);
		}
	}
	//sync_printf("currpid = %d, status: %d, ptqueue = %d\n", currpid, ptold->prstate, ptold->queue);
	// if(currpid == 4 || currpid == 5 || currpid == 6){
	// 	print_ready_list();
	// }

	//print_ready_list();
	//sync_printf("currpid: %d, State: %d\n", currpid, ptold->prstate);

	
	// try adding a var that tracks if a process has been updated instead of using else if
	bool8 new_p = 0;
	if(!check_empty(readylist_service)) {
		currpid = dequeue(readylist_service);
		new_p = 1;
	} 
	if (!new_p && !check_empty(readylist_high)) {
		currpid = dequeue(readylist_high);
		new_p = 1;
	} 
	if (!new_p && !check_empty(readylist_med)) {
		if (quantum_counter % 2 == 0) {
			currpid = dequeue(readylist_med);
			new_p = 1;
			//kprintf("M Current Time: %d\n", ((clktime*1000) + ctr1000));
			//sync_printf("M SW\n");
		} else if (ptold->prstate == PR_READY) {
			getitem(currpid);
			new_p = 1;
		}
	} 
	if (!new_p && !check_empty(readylist_low)) {
		if (quantum_counter % 4 == 0) {
			currpid = dequeue(readylist_low);
			//sync_printf("L Current Time: %d\n", ((clktime*1000) + ctr1000));
			//sync_printf("L SW\n");
			new_p = 1;
		} else if (ptold->prstate == PR_READY) {
			getitem(currpid);
			new_p = 1;
		}
	}
	if(!new_p) {
		// sync_printf("NULL, currpid=%d, quantum_c: %d\n", currpid, quantum_counter);
		// print_ready_list();
		currpid = 0;
		quantum_counter = -1;
	}
	ptnew = &proctab[currpid];
	ptnew->prstate = PR_CURR;
	
	if(ptnew == ptold) {
		preempt = QUANTUM;
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
	//quantum_counter = 0;
	preempt = QUANTUM;	
	#if DEBUG_CTXSW
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
	
	#if PRINT_READYLIST_SERVICE	
	sync_printf("List of ready processes from readylist_service:\n");						
	print_queue(readylist_service);
	#endif

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

qid16 demote(pid32 pid) {
	qid16 newq;
	struct	procent	*prptr;	
	prptr = &proctab[pid];
	prptr->timeallotment = 0;
	if(pid == 0) {
		return;
	}
	if(prptr->queue == readylist_low) {
		return readylist_low;
	}
	if(prptr->queue == readylist_service) {
		return readylist_service;
	}
	//sync_printf("Demote %d.\n", pid);
	if(prptr->queue == readylist_high) {
		newq = readylist_med;
	} else if (prptr->queue == readylist_med) {
		newq = readylist_low;
	}
	prptr->queue = newq;
	quantum_counter = 0;
	//getitem(pid);
	//insert(pid, newq, prptr->prprio);
	return newq;
}

void boost_priority(void) {
	// intmask mask;
	// mask = disable();

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
		prptr->timeallotment = 0;
		i++;
	}

	// prptr = &proctab[currpid];
	// if(prptr->queue != readylist_service){
	// 	prptr->queue = readylist_high;
	// }
	// prptr->timeallotment = 0;
	//print_ready_list();
	preempt = QUANTUM;
	quantum_counter = 0;
	// restore(mask);
	//resched();
	return;
}