/*  main.c  - main */

#include <xinu.h>

#define N 5 // Note: 3*N+4 must be <= NALOCKS 

pid32 pid[3*N+5];		// threads 
al_lock_t mutex[3*N+5];		// mutexes

syscall sync_printf(char *fmt, ...)
{
        intmask mask = disable();
        void *arg = __builtin_apply_args();
        __builtin_apply((void*)kprintf, arg, 100);
        restore(mask);
        return OK;
}


void run_for_ms(uint32 time){
	uint32 start = proctab[currpid].runtime;
	while ((proctab[currpid].runtime-start) < time);
}

process p2(al_lock_t *l1, al_lock_t *l2){
//	sync_printf("P%d:: acquiring: l1=%d l2=%d\n", currpid, l1->id, l2->id);	
	al_lock(l1);
	run_for_ms(1000);
	al_lock(l2);		
	run_for_ms(1000);
	al_unlock(l1);
	run_for_ms(1000);
	al_unlock(l2);		
	run_for_ms(1000);
	return OK;
}
	
process	main(void)
{

	uint32 i;
	
	/* initialize al_locks */
	for (i=0; i<3*N+5; i++) al_initlock(&mutex[i]);

	kprintf("\n\n=========== TEST for deadlocks  ===================\n\n");

	/* first deadlock: 2 threads */	
	pid[0] = create((void *)p2, INITSTK, 5, "p2", 2, &mutex[0], &mutex[1]); //5
	pid[1] = create((void *)p2, INITSTK, 5, "p2", 2, &mutex[1], &mutex[0]); //6

	/* second deadlock: N threads in sequence */
	for (i = 2; i < N+1; i++) 	
		pid[i] = create((void *)p2, INITSTK, 4,"p2", 2, &mutex[i], &mutex[i+1]); //7,8,9,10
	pid[N+1] = create((void *)p2, INITSTK, 4,"p2", 2, &mutex[N+1], &mutex[2]); //11

	/* third deadlock: N threads, not in sequence */
	for (i = N+2; i < 2*N+1; i++) 	
		pid[i] = create((void *)p2, INITSTK, 3,"p2", 2, &mutex[i], &mutex[i+1]); //12, 13, 14, 15
	pid[2*N+1] = create((void *)p2, INITSTK, 3,"p2", 2, &mutex[2*N+1], &mutex[N+2]); //16

	/* fourth deadlock: N threads - N-2 in a loop, two connected to loop but not part of it */
	pid[2*N+2] = create((void *)p2, INITSTK, 2,"p2", 2, &mutex[2*N+2], &mutex[2*N+3]);//17
	pid[2*N+3] = create((void *)p2, INITSTK, 2,"p2", 2, &mutex[2*N+3], &mutex[2*N+4]);//18
	for (i = 2*N+4; i < 3*N+1; i++) 
		pid[i] = create((void *)p2, INITSTK, 2,"p2", 2, &mutex[i-2], &mutex[i-1]); //19,20
	pid[3*N+1] = create((void *)p2, INITSTK, 2,"p2", 2, &mutex[3*N-1], &mutex[2*N+2]); //21

	/* two threads connected to existing loops but not part of them - no additional lock detected */
	pid[3*N+2] = create((void *)p2, INITSTK, 1,"p2", 2, &mutex[1], &mutex[3*N]); //22
	pid[3*N+3] = create((void *)p2, INITSTK, 1,"p2", 2, &mutex[3*N+1], &mutex[N]); //23

	/* one thread disconnected from everybody else */	
	pid[3*N+4] = create((void *)p2, INITSTK, 1,"p2", 2, &mutex[3*N+2], &mutex[3*N+3]); //24

	/* starts all the threads  - threads of third deadlock start out-of-order*/
	for (i = 0; i < 2*N+2; i++) 
		if (i!=N+2 && i!=2*N) resume(pid[i]);
	sleepms(100);
	resume(pid[2*N]);
	sleepms(100);
	resume(pid[N+2]);
	
	for (i = 2*N+2; i < 3*N+5; i++){ 
		resume(pid[i]);
		sleepms(5);
	}

	kprintf("P%d completed\n", receive());
	
	return OK;
}
