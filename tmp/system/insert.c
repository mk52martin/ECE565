/* insert.c - insert, remove */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  insert  -  Insert a process into a queue in descending key order
 *------------------------------------------------------------------------
 */
status	insert(
	  pid32		pid,		/* ID of process to insert	*/
	  qid16		q,		/* ID of queue to use		*/
	  int32		key		/* Key for the inserted process	*/
	)
{
	qid16	curr;			/* Runs through items in a queue*/
	qid16	prev;			/* Holds previous node index	*/

	if (isbadqid(q) || isbadpid(pid)) {
		return SYSERR;
	}

	curr = firstid(q);
	while (queuetab[curr].qkey >= key) {
		curr = queuetab[curr].qnext;
	}

	/* Insert process between curr node and previous node */

	prev = queuetab[curr].qprev;	/* Get index of previous node	*/
	queuetab[pid].qnext = curr;
	queuetab[pid].qprev = prev;
	queuetab[pid].qkey = key;
	queuetab[prev].qnext = pid;
	queuetab[curr].qprev = pid;
	return OK;
}


status	remove(
	  pid32		pid			/* ID of process to rmeove from queue	*/
	)
{
	qid16	next;			/* Holds next node index 		*/
	qid16	prev;			/* Holds previous node index	*/

	if (isbadpid(pid)) {
		return SYSERR;
	}


	/* Remove process from between previous nodes */	
	prev = queuetab[pid].qprev;	/* Get index of previous node	*/
	next = queuetab[pid].qnext;	/* Get index of next node	*/
	queuetab[prev].qnext = next;
	queuetab[next].qprev = prev;

	return OK;
}