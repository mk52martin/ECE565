/* resched.c - resched, resched_cntl */
#define DEBUG_CTXSW 	0
#define MLFQ			1
#define PRINT_READYLIST_SERVICE 	0
#include <xinu.h>

struct	defer	Defer;
bool8	sync_call = FALSE;

/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* Assumes interrupts are disabled	*/
{
	intmask mask;
	mask = disable();

	struct procent *ptold;	/* Ptr to table entry for old process	*/
	struct procent *ptnew;	/* Ptr to table entry for new process	*/
	sync_call = TRUE;
	if(preempt != QUANTUM ) {
		// if(proctab[currpid].prstate != PR_CURR){
		// 	preempt = QUANTUM;
		// } else if (currpid != 0){
		// 	proctab[currpid].prstate = PR_READY;
		// 	enqueue(currpid, proctab[currpid].queue);
		// }
		quantum_counter = 0;
		sync_call = FALSE;
	} 

	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	/* Point to process table entry for the current (old) process */

	ptold = &proctab[currpid];
	if(boost >= PRIORITY_BOOST_PERIOD) {
		//sync_printf("Current Time: %d\n", ((clktime*1000) + ctr1000));
		boost_priority();
		boost = 0;
	}
	#if MLFQ
	if(ptold->timeallotment >= TIME_ALLOTMENT) {
		demote(currpid);
	}
	if (ptold->prstate == PR_CURR) {  /* Process remains eligible */
		/* Old process will no longer remain current */
		ptold->prstate = PR_READY;
		if(currpid != 0){
			insert(currpid, ptold->queue, ptold->prprio);
		} else {
			quantum_counter = 0;					// curr pid = 0, allow anything to run
		}
	} else {
		quantum_counter = 0;						// no current process, allow anything to run
	}
	//sync_printf("currpid = %d, status: %d, ptqueue = %d\n", currpid, ptold->prstate, ptold->queue);

	//print_ready_list();
	//sync_printf("currpid: %d, State: %d\n", currpid, ptold->prstate);

	
	// try adding a var that tracks if a process has been updated instead of using else if
	bool8 new_p = 0;
	if(!check_empty(readylist_service)) {			// service highest prio
		currpid = dequeue(readylist_service);		// RR
		new_p = 1;
	} 
	if (!new_p && !check_empty(readylist_high)) {	// high prio 2nd
		currpid = dequeue(readylist_high);			// RR
		new_p = 1;
	} 
	if (!new_p && !check_empty(readylist_med)) {	// med prio next
		if (!(quantum_counter % 2)) {				// end of time slice?
			currpid = dequeue(readylist_med);			//RR
			new_p = 1;
			//kprintf("M Current Time: %d\n", ((clktime*1000) + ctr1000));
			//sync_printf("M SW\n");
		} else if (ptold->prstate == PR_READY) {	// else: return to current
			getitem(currpid);
			new_p = 1;
		}
	} 
	if (!new_p && !check_empty(readylist_low)) {	// finally low
		if (!(quantum_counter % 4)) {				// end of time slice?
			currpid = dequeue(readylist_low);			//RR
			//sync_printf("L Current Time: %d\n", ((clktime*1000) + ctr1000));
			//sync_printf("L SW\n");
			new_p = 1;
		} else if (ptold->prstate == PR_READY) {	// else: return to current
			getitem(currpid);
			new_p = 1;
		}
	}
	if(!new_p) {									// if nothing else, run null
		currpid = 0;
		//quantum_counter = -1;
	}
	ptnew = &proctab[currpid];						// get new process and set to current
	ptnew->prstate = PR_CURR;
	
	if(ptnew == ptold) {							// return to normal if no change in process
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

	//sync_printf("q counter: %d, q: %d->%d ", quantum_counter, ptold->queue, ptnew->queue);
	ptnew->num_ctxsw++;	
	quantum_counter = 0;		// reset counter on ctxsw
	preempt = QUANTUM;	
	#if DEBUG_CTXSW
	sync_printf("ctxsw::%d-%d (%d, %d)\n", ptold->pid, ptnew->pid, ptold->runtime, ((clktime*1000) + ctr1000));
	#endif

	restore(mask);

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
	if(pid == 0) {
		prptr->timeallotment = 0;
		return;
	}
	if(prptr->queue == readylist_low) {
		//sync_printf("No demote %d, ctxsw: %d.\n", pid, prptr->num_ctxsw);
		//sync_printf("No demote %d, ctxsw: %d, runtime: %d, cur time: %d.\n", pid, prptr->num_ctxsw, prptr->runtime, ((clktime*1000) + ctr1000));
		prptr->timeallotment = 0;
		return readylist_low;
	}
	if(prptr->queue == readylist_service) {
		prptr->timeallotment = 0;
		return readylist_service;
	}
	//sync_printf("Demote %d.\n", pid);
	if(prptr->queue == readylist_high) {
		newq = readylist_med;
		//sync_printf("Demote from high %d, ctxsw: %d, runtime: %d, cur time: %d.\n", pid, prptr->num_ctxsw, prptr->runtime, ((clktime*1000) + ctr1000));
	} else if (prptr->queue == readylist_med) {
		if((!sync_call) || !(quantum_counter % 2)){
			newq = readylist_low;
			// sync_printf("Demote from med %d, ctxsw: %d, ta: %d, pre: %d, qc: %d, sc: %d.\n", pid, prptr->num_ctxsw, prptr->timeallotment, preempt, quantum_counter, sync_call);
		} else {
			// sync_printf("No demote from med %d, ctxsw: %d, ta: %d, pre: %d, qc: %d, sc %d.\n", pid, prptr->num_ctxsw, prptr->timeallotment, preempt, quantum_counter, sync_call);
			// print_ready_list();
			return readylist_med;
		}
		//newq = readylist_low;
	}
	prptr->timeallotment = 0;
	prptr->queue = newq;
	quantum_counter = 0;
	//getitem(pid);
	//insert(pid, newq, prptr->prprio);
	return newq;
}

void boost_priority(void) {
	intmask mask;
	mask = disable();
	
	struct	procent	*prptr;	
	prptr = &proctab[currpid];
	if(prptr->prstate == PR_CURR) {
		prptr->prstate = PR_READY;
		insert(currpid, prptr->queue, prptr->prprio);
	}

	qid16 tail = queuetail(readylist_med);												//find head
	qid16 it = firstid(readylist_med);	
	qid16 next;
			
	while(it != tail) {												// cycle through readylist
		next = queuetab[it].qnext;
		getitem(it);
		insert(it, readylist_high, proctab[it].prprio);
		it = next;	
	}
	

	tail = queuetail(readylist_low);												//find head
	it = firstid(readylist_low);	

	while(it != tail) {												// cycle through readylist
		next = queuetab[it].qnext;
		getitem(it);
		insert(it, readylist_high, proctab[it].prprio);
		it = next;		
	}

	// if(proctab[currpid].queue != readylist_service && proctab[currpid].prstate == PR_CURR){
	// 	proctab[currpid].prstate = PR_READY;
	// 	enqueue(currpid, readylist_high);
	// }

	int i = 0;
	while(i < NPROC) {
		prptr = &proctab[i];
		if(prptr->queue != readylist_service){
			prptr->queue = readylist_high;
		}
		prptr->timeallotment = 0;
		i++;
	}

	quantum_counter = 0;
	preempt = QUANTUM;

	//reenable interrupts
	restore(mask);
	return;
}