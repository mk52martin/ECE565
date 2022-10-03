/* clkhandler.c - clkhandler */

#include <xinu.h>

/*------------------------------------------------------------------------
 * clkhandler - high level clock interrupt handler
 *------------------------------------------------------------------------
 */
void	clkhandler()
{
	static	uint32	count1000 = 1000;	/* Count to 1000 ms	*/

	/* Decrement the ms counter, and see if a second has passed */
	if((--count1000) <= 0) {

		/* One second has passed, so increment seconds count */

		
		clktime++;

		/* Reset the local ms counter for the next second */

		count1000 = 1000;
	}
	ctr1000 = 1000 - count1000;
	struct	procent	*prptr = &proctab[currpid];
	prptr->runtime++;
	prptr->timeallotment++;
	/* Handle sleeping processes if any exist */

	if(!isempty(sleepq)) {

		/* Decrement the delay for the first process on the	*/
		/*   sleep queue, and awaken if the count reaches zero	*/

		if((--queuetab[firstid(sleepq)].qkey) <= 0) {
			wakeup();
		}
	}


	if((++boost) >= PRIORITY_BOOST_PERIOD) {
		//sync_printf("BOOST!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		boost_priority();
		boost = 0;
	}
	/* Decrement the preemption counter, and reschedule when the */
	/*   remaining time reaches zero			     */
	if((--preempt) <= 0) {
		quantum_counter++;
		preempt = QUANTUM;
		resched();
		if(quantum_counter == 4) {
			quantum_counter = 0;
		}
	}
}
