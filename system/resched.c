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
	// intmask mask;
	// mask = disable();

	struct procent *ptold;	/* Ptr to table entry for old process	*/
	struct procent *ptnew;	/* Ptr to table entry for new process	*/
	sync_call = TRUE;
	if(preempt != QUANTUM ) {
		quantum_counter = 0;
		sync_call = FALSE;
	} 
	// sync_printf("Currpid: %d, state: %d, rt: %d, ta:%d\n", currpid, proctab[currpid].prstate, proctab[currpid].runtime, proctab[currpid].timeallotment);
	// sync_printf("Current Time: %d\n", ((clktime*1000) + ctr1000));
	// print_ready_list();
	// sync_printf("sleepq: ");
	// print_queue(sleepq);

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
	}
	#if MLFQ
	if(ptold->timeallotment >= TIME_ALLOTMENT) {
		demote(currpid);
	}
	/*if (ptold->prstate == PR_CURR) {  //Process remains eligible
		// old process no longer current
		ptold->prstate = PR_READY;
		if(currpid != 0){
			insert(currpid, ptold->queue, ptold->prprio);
		} else {
			quantum_counter = 0;				// takes  values down A LOT (mlfq1 tc3 -> ctx ~130, rt ~ 1900)
		}
	} else {
		quantum_counter = 0;				// takes  values down A LOT (mlfq1 tc3 -> ctx ~130, rt ~ 1900)
	}*/


	
	bool8 new_p = 0;

	if(sync_call && ptold->prstate == PR_CURR) {
		if(ptold->queue == readylist_med && (quantum_counter % 2)) {
			new_p = 1;
		}
		if(ptold->queue == readylist_low && (quantum_counter % 4)) {
			new_p = 1;
		}
	}

	if (!new_p && ptold->prstate == PR_CURR) {  //Process remains eligible
		// old process no longer current
		ptold->prstate = PR_READY;
		if(currpid != 0){
			insert(currpid, ptold->queue, ptold->prprio);
		}
	}

	if(!new_p && !check_empty(readylist_service)) {
		currpid = dequeue(readylist_service);
	} else if (!new_p && !check_empty(readylist_high)) {
		currpid = dequeue(readylist_high);
	} else if (!new_p && !check_empty(readylist_med)) {
		currpid = dequeue(readylist_med);
	} else if (!new_p && !check_empty(readylist_low)) {
		currpid = dequeue(readylist_low);
	} else if (!new_p) {
		currpid = 0;
	}


	/*if(!check_empty(readylist_service)) {
		currpid = dequeue(readylist_service);
		new_p = 1;
	} 
	if (!new_p && !check_empty(readylist_high)) {
		currpid = dequeue(readylist_high);
		new_p = 1;
	} 
	if (!new_p && !check_empty(readylist_med)) {
		if(sync_call && (quantum_counter % 2) != 0) {
			getitem(currpid);
		} else {
			currpid = dequeue(readylist_med);
		}
		new_p = 1;
		// if ((!sync_call) || ((quantum_counter % 2) == 0)) {
		// 	currpid = dequeue(readylist_med);
		// 	new_p = 1;
		// 	//kprintf("M Current Time: %d\n", ((clktime*1000) + ctr1000));
		// 	//sync_printf("M SW\n");
		// } else if (ptold->prstate == PR_READY) {
		// 	getitem(currpid);
		// 	new_p = 1;
		// }
	} 
	if (!new_p && !check_empty(readylist_low)) {
		if(sync_call && (quantum_counter % 4) != 0) {
			getitem(currpid);
		} else {
			currpid = dequeue(readylist_low);
		}
		new_p = 1;
		
		// if ((!sync_call) || ((quantum_counter % 4) == 0)) {
		// 	currpid = dequeue(readylist_low);
		// 	//sync_printf("L Current Time: %d\n", ((clktime*1000) + ctr1000));
		// 	//sync_printf("L SW\n");
		// 	new_p = 1;
		// } else if (ptold->prstate == PR_READY) {
		// 	getitem(currpid);
		// 	new_p = 1;
		// }
	}*/
	// if(!new_p) {
	// 	currpid = 0;
	// }

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

	//sync_printf("q counter: %d, q: %d->%d ", quantum_counter, ptold->queue, ptnew->queue);
	ptnew->num_ctxsw++;	
	quantum_counter = 0;
	preempt = QUANTUM;	
	#if DEBUG_CTXSW
	sync_printf("ctxsw::%d-%d\n", ptold->pid, ptnew->pid);
	#endif

	//restore(mask);

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
	//quantum_counter = 0;
	//getitem(pid);
	//insert(pid, newq, prptr->prprio);
	return newq;
}

void boost_priority(void) {
	quantum_counter = 0;
	preempt = QUANTUM;

	struct	procent	*prptr;
	prptr = &proctab[currpid];
	if(prptr->prstate == PR_CURR) {
		prptr->prstate = PR_READY;
		enqueue(currpid, prptr->queue);
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


	int i = 0;	
	while(i < NPROC) {
		prptr = &proctab[i];
		if(prptr->queue != readylist_service){
			// if(prptr->prstate == PR_READY) {
			// 	getitem(i);
			// 	insert(i, readylist_high, prptr->prprio);
			// }
			prptr->queue = readylist_high;
		}
		prptr->timeallotment = 0;
		i++;
	}

	quantum_counter = 0;
	preempt = QUANTUM;
	boost = 0;
	sync_call = TRUE;
	//resched();
	//reenable interrupts
	//restore(mask);
	return;
}