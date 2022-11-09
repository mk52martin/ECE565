/* clkhandler.c - clkhandler */

#include <xinu.h>
uint32 temp_counter;
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

	proctab[currpid].runtime++;
	ctr1000++;
	/* Handle sleeping processes if any exist */

	if(!isempty(sleepq)) {

		/* Decrement the delay for the first process on the	*/
		/*   sleep queue, and awaken if the count reaches zero	*/

		if((--queuetab[firstid(sleepq)].qkey) <= 0) {
			wakeup();
		}
	}

	/* Decrement the preemption counter, and reschedule when the */
	/*   remaining time reaches zero			     */

	if((--preempt) <= 0) {
		preempt = QUANTUM;
		// temp_counter++;
		// if(temp_counter > 1000){
		// 	kprintf("*%d\n", currpid);
		// 	print_queue(test_lock->queue);
		// 	print_queue(readylist);
		// 	temp_counter = 0;
		// }
		resched();
	}
}
