#include <xinu.h>
#include <stdio.h>

void timed_execution(uint32 runtime){
	    while(proctab[currpid].runtime<runtime);
}

int main() {
	pid32 prA, prB, prC, prD;

	kprintf("\n");
	
	kprintf("=== TESTCASE 1::  Short Jobs =============================\n");

	prA = create_user_process(timed_execution, 1024, "timed_execution", 1, 100);
	prB = create_user_process(timed_execution, 1024, "timed_execution", 1, 100);

	set_tickets(prA, 50);
	set_tickets(prB, 50);
	
	resume(prA);
	resume(prB);
	receive();	
	receive();	

	sleepms(250); // wait for user processes to terminate	

	kprintf("process %d:: runtime=%d, turnaround time=%d, ctx=%d\n",prA, proctab[prA].runtime, proctab[prA].turnaroundtime, proctab[prA].num_ctxsw);
	kprintf("process %d:: runtime=%d, turnaround time=%d, ctx=%d\n",prB, proctab[prB].runtime, proctab[prB].turnaroundtime, proctab[prB].num_ctxsw);
    kprintf("process %d:: turnaround ratio = %d/%d = %f\n", prA, proctab[prA].turnaroundtime, proctab[prB].turnaroundtime, ((float)proctab[prA].turnaroundtime/(float)proctab[prB].turnaroundtime));
    kprintf("process %d:: turnaround ratio = %d/%d = %f\n", prB, proctab[prB].turnaroundtime, proctab[prA].turnaroundtime, ((float)proctab[prB].turnaroundtime/(float)proctab[prA].turnaroundtime));

	kprintf("==================================================================\n\n");

	kprintf("=== TESTCASE 2::  Medium Jobs ===============================\n");

	prA = create_user_process(timed_execution, 1024, "timed_execution", 1, 500);
	prB = create_user_process(timed_execution, 1024, "timed_execution", 1, 500);

	set_tickets(prA, 50);
	set_tickets(prB, 50);
	
	resume(prA);
	resume(prB);
	receive();	
	receive();	

	sleepms(1100); // wait for user processes to terminate	

	kprintf("process %d:: runtime=%d, turnaround time=%d, ctx=%d\n",prA, proctab[prA].runtime, proctab[prA].turnaroundtime, proctab[prA].num_ctxsw);
	kprintf("process %d:: runtime=%d, turnaround time=%d, ctx=%d\n",prB, proctab[prB].runtime, proctab[prB].turnaroundtime, proctab[prB].num_ctxsw);
    kprintf("process %d:: turnaround ratio = %d/%d = %f\n", prA, proctab[prA].turnaroundtime, proctab[prB].turnaroundtime, ((float)proctab[prA].turnaroundtime/(float)proctab[prB].turnaroundtime));
    kprintf("process %d:: turnaround ratio = %d/%d = %f\n", prB, proctab[prB].turnaroundtime, proctab[prA].turnaroundtime, ((float)proctab[prB].turnaroundtime/(float)proctab[prA].turnaroundtime));

	kprintf("==================================================================\n\n");

    kprintf("=== TESTCASE 2::  Long Jobs ===============================\n");

	prA = create_user_process(timed_execution, 1024, "timed_execution", 1, 1000);
	prB = create_user_process(timed_execution, 1024, "timed_execution", 1, 1000);

	set_tickets(prA, 50);
	set_tickets(prB, 50);
	
	resume(prA);
	resume(prB);
	receive();	
	receive();	

	sleepms(2200); // wait for user processes to terminate	

	kprintf("process %d:: runtime=%d, turnaround time=%d, ctx=%d\n",prA, proctab[prA].runtime, proctab[prA].turnaroundtime, proctab[prA].num_ctxsw);
	kprintf("process %d:: runtime=%d, turnaround time=%d, ctx=%d\n",prB, proctab[prB].runtime, proctab[prB].turnaroundtime, proctab[prB].num_ctxsw);
    kprintf("process %d:: turnaround ratio = %d/%d = %f\n", prA, proctab[prA].turnaroundtime, proctab[prB].turnaroundtime, ((float)proctab[prA].turnaroundtime/(float)proctab[prB].turnaroundtime));
    kprintf("process %d:: turnaround ratio = %d/%d = %f\n", prB, proctab[prB].turnaroundtime, proctab[prA].turnaroundtime, ((float)proctab[prB].turnaroundtime/(float)proctab[prA].turnaroundtime));

	kprintf("==================================================================\n\n");

	return OK;
}
