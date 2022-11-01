/* insert.c - insert */

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


/*------------------------------------------------------------------------
 *  insert_back  -  Insert a process into the back of a queue
 *------------------------------------------------------------------------
 */
status	insert_back(
	  pid32		pid,		/* ID of process to insert	*/
	  qid16		q		/* ID of queue to use		*/
	)
{
	qid16	tail;			/* Runs through items in a queue*/
	qid16	prev;			/* Holds previous node index	*/

	if (isbadqid(q) || isbadpid(pid)) {
		return SYSERR;
	}

	tail = queuetail(q);
	prev = lastid(q);

	/* Insert process between curr tail and previous node */

	queuetab[pid].qnext = tail;
	queuetab[pid].qprev = prev;
	queuetab[pid].qkey = 0;
	queuetab[prev].qnext = pid;
	queuetab[tail].qprev = pid;
	return OK;
}