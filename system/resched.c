/* resched.c - resched, resched_cntl */
#define DEBUG_CTXSW 1
#define DEBUG		0
#define LOTTERY		1
#include <xinu.h>
#include <stdlib.h>

struct	defer	Defer;

/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* Assumes interrupts are disabled	*/
{
	struct procent *ptold;		/* Ptr to table entry for old process			*/
	struct procent *ptnew;		/* Ptr to table entry for new process			*/
	struct procent *search;		/* Ptr to search processes						*/
	qid16	curr;				/* readylist pointer for queue					*/
	qid16	system_process = 0; /* id for system process with highest priority 	*/
	uint32 	lottery_size = 0;	/* lottery size									*/
	uint32 	lottery_number;

	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}



#if LOTTERY
#if DEBUG
	print_ready_list();
#endif

	// Check for system processes & find lottery size
	curr = firstid(readylist);
	while (curr != 301) {
		search = &proctab[curr];
		if(search->user_process == SYSTEM_PROCESS && queuetab[curr].qkey > queuetab[system_process].qkey) {
			system_process = curr;						// highest priority system queue id stored in system_process
#if DEBUG
			sync_printf("Found system process %d.\n", system_process);
#endif
		}
#if DEBUG
		sync_printf("Found process %d.\n", curr);
#endif
		lottery_size += queuetab[curr].qkey;
		curr = queuetab[curr].qnext;
	}

	/* Point to process table entry for the current (old) process */
	ptold = &proctab[currpid];

	//if system process is available or currently system process
	if(system_process > 0 && ptold->user_process == SYSTEM_PROCESS) {	//if there is a system process available and the currecnt process is a system process
#if DEBUG
		sync_printf("System process for both %d, %d.\n", currpid, system_process);
#endif
		if(ptold->prprio > queuetab[system_process].qkey && ptold->prstate == PR_CURR) {				// if the old process is higher priority and a system process
			return;
		} else {	// new system process is higher priority than old system process
			if (ptold->prstate == PR_CURR) { // eligible
				ptold->prstate = PR_READY;								// set old state to ready
				insert(currpid, readylist, ptold->prprio);	
			}															// if there is a user process
			currpid = system_process;
			ptnew = &proctab[system_process];							// set new process to ready
			remove(system_process);		
		}								
	} else if (system_process > 0) { 	// there is a system process to run
#if DEBUG
		sync_printf("System process to run: %d.\n", system_process);
#endif		
		if (ptold->prstate == PR_CURR) {
			ptold->prstate = PR_READY;									// set old state to ready
			insert(currpid, readylist, ptold->prprio);	
		}																// if there is a user process
		currpid = system_process;
		ptnew = &proctab[system_process];								// set new process to ready
		remove(system_process);	
	} else if (ptold->user_process == SYSTEM_PROCESS && ptold->prstate == PR_CURR) {
		// return to only running system process
#if DEBUG
		//sync_printf("Currently running only system process %d.\n", currpid);
#endif
		return;
	} else if(firstid(readylist) == 0 && ptold->prstate != PR_CURR) { // if null is only process
		remove(0);	
		currpid = 0;
		ptnew = &proctab[currpid];							// set new process to ready
	} else if(firstid(readylist) == 301) {
			if (ptold->prstate != PR_CURR) {
				remove(0);	
				currpid = 0;
				ptnew = &proctab[currpid];							// set new process to ready
			} else {
				return;
			}
	} else {	// lottery
#if DEBUG
		sync_printf("Lottery Scheduling.\n", system_process);
		print_ready_list();
#endif
		lottery_size += ptold->prprio;
		insert(currpid, readylist, ptold->prprio);	
		lottery_number = rand() % lottery_size;
		curr = firstid(readylist);
		while(lottery_number > queuetab[curr].qkey) {
			lottery_number -= queuetab[curr].qkey;
			curr = queuetab[curr].qnext;
		}
		currpid = curr;
		ptnew = &proctab[currpid];
		remove(curr);
		if(ptnew == ptold) {
			return;
		}
	}
#if DEBUG
	sync_printf("Running %d.\n", currpid);
#endif
	ptnew->prstate = PR_CURR;
	preempt = QUANTUM;		/* Reset time slice for process	*/
	ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
#else
/* Point to process table entry for the current (old) process */

	ptold = &proctab[currpid];

	if (ptold->prstate == PR_CURR) {  /* Process remains eligible */
		if (ptold->prprio > firstkey(readylist)) {
			return;
		}

		/* Old process will no longer remain current */

		ptold->prstate = PR_READY;
		insert(currpid, readylist, ptold->prprio);
	}

	/* Force context switch to highest priority ready process */

	currpid = dequeue(readylist);
	ptnew = &proctab[currpid];
	ptnew->prstate = PR_CURR;
	preempt = QUANTUM;		/* Reset time slice for process	*/
	ptnew->num_ctxsw++;		
#ifdef DEBUG_CTXSW
	printf("ctxsw::%d-%d\n", ptold->pid, ptnew->pid);
#endif
	ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

#endif
	
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
	

	//print readylist
	qid16 tail = queuetail(readylist);												//find head
	qid16 it = firstid(readylist);									

	sync_printf("List of ready processes from readylist:\n%d", it);						// print first item
	if(it == 301)  {
		sync_printf("\n");
		return OK;
	}
	it = queuetab[it].qnext;
	while(queuetab[it].qnext != tail) {												// cycle through readylist
		sync_printf(", %d", it);	
		if(it == 301)  {
			sync_printf("\n");
			return OK;
		}
		it = queuetab[it].qnext;
	}
	if(it != firstid(readylist)) {													// print tail if >1 process
		sync_printf(", %d", it);
	}
	sync_printf("\n");

	//reenable interrupts
	restore(mask);
	return OK;
}
