/*  main.c  - main */

#include <xinu.h>

#define N 12 // Note: N must be <= NALOCKS 
#define ACTIVE  TRUE
#define TRY     FALSE

pid32 pid[N];		        // threads 
al_lock_t mutex[N];			// mutexes

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

process p2(al_lock_t *l1, al_lock_t *l2, bool8 lock_type){
	//sync_printf("P%d:: acquiring: l1=%d l2=%d\n", currpid, l1, l2);	
	if(lock_type){
        sync_printf("P%d:: Regular lock.\n", currpid);
    	sleepms(1000);
		al_lock(l1);
    	run_for_ms(50);
		al_lock(l2);	
    } else{
        sync_printf("P%d:: Try lock.\n", currpid);
		sleepms(1000);
		lock_l1:
		al_lock(l1);
    	run_for_ms(50);
		if(!al_trylock(l2)) {
			al_unlock(l1);
			sleepms(QUANTUM);
			goto lock_l1;
		}
    }	
	run_for_ms(50);
	al_unlock(l1);
	run_for_ms(10);
	al_unlock(l2);
	sync_printf("P%d completed!\n", currpid);
	return OK;
}

process p2_print_info(al_lock_t *l1, al_lock_t *l2, bool8 lock_type){
	sync_printf("P%d:: acquiring: l1=%d l2=%d, prio: %d\n", currpid, l1, l2, proctab[currpid].prprio);	
	if(lock_type){
        sync_printf("P%d:: Regular lock.\n", currpid);
    	sleepms(500);
		al_lock(l1);
    	run_for_ms(50);
		al_lock(l2);	
    } else{
        sync_printf("P%d:: Try lock.\n", currpid);
		sleepms(500);
		lock_l1:
		al_lock(l1);
    	run_for_ms(50);
		if(!al_trylock(l2)) {
			al_unlock(l1);
			sleepms(QUANTUM);
			goto lock_l1;
		}
    }	
	run_for_ms(50);
	al_unlock(l1);
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


	

	/* first deadlock: 3 threads */	
	pid[0] = create((void *)p2, INITSTK, 2, "p2", 3, &mutex[0], &mutex[1], ACTIVE);
	pid[1] = create((void *)p2, INITSTK, 2, "p2", 3, &mutex[1], &mutex[2], ACTIVE);
    pid[2] = create((void *)p2, INITSTK, 2, "p2", 3, &mutex[2], &mutex[0], ACTIVE);

	/* second deadlock: but with trylock to avoid */
	pid[3] = create((void *)p2, INITSTK, 2, "p2", 3, &mutex[3], &mutex[4], TRY);
	pid[4] = create((void *)p2, INITSTK, 2, "p2", 3, &mutex[4], &mutex[5], TRY);
    pid[5] = create((void *)p2, INITSTK, 2, "p2", 3, &mutex[5], &mutex[3], TRY);

	/* varing priorities */
	pid[6] = create((void *)p2_print_info, INITSTK, 3, "p2", 3, &mutex[6], &mutex[7], TRY);
	pid[7] = create((void *)p2_print_info, INITSTK, 4, "p2", 3, &mutex[7], &mutex[8], TRY);
    pid[8] = create((void *)p2_print_info, INITSTK, 5, "p2", 3, &mutex[8], &mutex[9], TRY);
	pid[9] = create((void *)p2_print_info, INITSTK, 6, "p2", 3, &mutex[9], &mutex[10], TRY);
	pid[10] = create((void *)p2_print_info, INITSTK, 7, "p2", 3, &mutex[10], &mutex[11], TRY);
    pid[11] = create((void *)p2_print_info, INITSTK, 8, "p2", 3, &mutex[11], &mutex[6], TRY);


	kprintf("\n\n================ TEST for deadlocks  ========================\n\n");
	/* starts all the threads */
	for (i = 0; i < 3; i++) {
		resume(pid[i]);
	}

	sleepms(1500);

	kprintf("\n\n============= TEST for trylock (basic)  =====================\n\n");
	for (i = 3; i < 6; i++) {
		resume(pid[i]);
	}
	for(i = 3; i < 6; i++) {
		receive();
	}

	kprintf("\n\n============= TEST for trylock (multiple priorities)  =====================\n\n");
	for (i = 6; i < 12; i++) {
		resume(pid[i]);
	}
	for(i = 6; i < 12; i++) {
		receive();
	}

	return OK;
}