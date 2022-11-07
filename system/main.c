/*  main.c  - main */

#include <xinu.h>

#define N 6 // Note: N must be <= NALOCKS 
#define ACTIVE  TRUE
#define TRY     FALSE

pid32 pid[N];		        // threads 
al_lock_t mutex[N];		// mutexes

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
		//sync_printf("(%d), %d--%d\n", currpid, proctab[currpid].runtime, start);
}

process p2(al_lock_t *l1, al_lock_t *l2, bool8 lock_type){
	//sync_printf("P%d:: acquiring: l1=%d l2=%d\n", currpid, l1, l2);	
	if(lock_type){
        sync_printf("P%d:: Regular lock.\n", currpid);
    	run_for_ms(50);
		al_lock(l1);
    	run_for_ms(1000);
		al_lock(l2);	
    } else{
        sync_printf("P%d:: Try lock.\n", currpid);
		run_for_ms(50);
		lock_l1:
		al_lock(l1);
		//sync_printf("%d.\n", currpid);
    	run_for_ms(10);
		//sync_printf("%d...\n", currpid);
		if(!al_trylock(l2)) {
			al_unlock(l1);
			//sync_printf("P%d unlock\n", currpid);
			sleepms(QUANTUM);
			goto lock_l1;
		}
		//sync_printf("%d through trylock-----------------------------\n", currpid);
    }	
	run_for_ms(10);
	al_unlock(l1);
	//sync_printf("%d unlock l1\n", currpid);
	run_for_ms(10);
	al_unlock(l2);
	sync_printf("P%d completed!\n", currpid);
	return OK;
}
	
process	main(void)
{

	uint32 i;
	
	/* initialize al_locks */
	for (i=0; i<N; i++) al_initlock(&mutex[i]);

	kprintf("\n\n=========== TEST for deadlocks  ===================\n\n");

	/* first deadlock: 3 threads */	
	pid[0] = create((void *)p2, INITSTK, 5, "p2", 3, &mutex[0], &mutex[1], ACTIVE);
	pid[1] = create((void *)p2, INITSTK, 5, "p2", 3, &mutex[1], &mutex[2], ACTIVE);
    pid[2] = create((void *)p2, INITSTK, 5, "p2", 3, &mutex[2], &mutex[0], ACTIVE);

	/* second deadlock: but with trylock to avoid */
	pid[3] = create((void *)p2, INITSTK, 5, "p2", 3, &mutex[3], &mutex[4], TRY);
	pid[4] = create((void *)p2, INITSTK, 5, "p2", 3, &mutex[4], &mutex[5], TRY);
    pid[5] = create((void *)p2, INITSTK, 5, "p2", 3, &mutex[5], &mutex[3], TRY);

	/* starts all the threads */
	for (i = 0; i < N; i++) 
		resume(pid[i]);
	
	return OK;
}
