/*  main.c  - main */

#include <xinu.h>
#ifndef main
#define N 3 // Note: N must be <= NALOCKS 
#define ACTIVE  TRUE
#define TRY     FALSE

pid32 pid[2*N];		        	// threads 
pi_lock_t mutex[2*N+1];			// mutexes

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

process delay_run(pi_lock_t *l1, uint32 sleep_time, uint32 runtime){
	//sync_printf("P%d:: acquiring: l1=%d l2=%d\n", currpid, l1, l2);	
	sync_printf("Staring P%d w/ priority %d and start delay of %dms.\n", currpid, proctab[currpid].prprio, sleep_time);
	sleepms(sleep_time);
	pi_lock(l1);
	run_for_ms(runtime);
	pi_unlock(l1);
	sync_printf("P%d completed!\n", currpid);
	return OK;
}

process delay_run_2_lock(pi_lock_t *l1, pi_lock_t *l2, uint32 sleep_time, uint32 runtime){
	//sync_printf("P%d:: acquiring: l1=%d l2=%d\n", currpid, l1, l2);	
	sync_printf("Staring P%d w/ priority %d and start delay of %dms. (2 locks)\n", currpid, proctab[currpid].prprio, sleep_time);
	// if(proctab[currpid].prprio == 1) {
	// 	print_queue(readylist);
	// }
	sleepms(sleep_time);
	pi_lock(l1);
	run_for_ms(runtime);
	pi_lock(l2);
	run_for_ms(runtime);
	pi_unlock(l2);
	pi_unlock(l1);
	sync_printf("P%d completed!\n", currpid);
	return OK;
}

process sleep_run(pi_lock_t *l1, uint32 sleep_time, uint32 runtime){
	//sync_printf("P%d:: acquiring: l1=%d l2=%d\n", currpid, l1, l2);	
	sync_printf("Staring P%d w/ priority %d and start delay of %dms.\n", currpid, proctab[currpid].prprio, sleep_time);
	sleepms(sleep_time);
	pi_lock(l1);
	sleepms(500);
	run_for_ms(runtime);
	pi_unlock(l1);
	sync_printf("P%d completed!\n", currpid);
	return OK;
}

process	main(void)
{

	uint32 i;
	
	/* initialize al_locks */
	for (i=0; i<2*N+1; i++) pi_initlock(&mutex[i]);

	test_lock = &mutex[0];

	// kprintf("\n\n=========== TEST with 6 increasing prio threads  ===================\n\n");

	// /* first deadlock: 3 threads */	
	// pid[0] = create((void *)delay_run, INITSTK, 1, "p1", 3, &mutex[0], QUANTUM, 1000);
	// pid[1] = create((void *)delay_run, INITSTK, 2, "p1", 3, &mutex[0], 10*QUANTUM, 1000);
    // pid[2] = create((void *)delay_run, INITSTK, 3, "p1", 3, &mutex[0], 20*QUANTUM, 1000);
	// pid[3] = create((void *)delay_run, INITSTK, 4, "p1", 3, &mutex[0], 30*QUANTUM, 1000);
	// pid[4] = create((void *)delay_run, INITSTK, 5, "p1", 3, &mutex[0], 40*QUANTUM, 1000);
    // pid[5] = create((void *)delay_run, INITSTK, 10, "p1", 3, &mutex[0], 60*QUANTUM, 1000);

	// /* starts 1st set of threads */
	// for (i = 0; i < 2*N; i++) {
	// 	resume(pid[i]);
	// }

	// for (i = 0; i < 2*N; i++) {
	// 	receive();
	// }


	kprintf("\n\n=========== TEST with 6 increasing prio threads  ===================\n\n"); // works

	/* first deadlock: 3 threads */	
	pid[0] = create((void *)delay_run, INITSTK, 1, "p1", 3, &mutex[0], QUANTUM, 1000);
	pid[1] = create((void *)delay_run, INITSTK, 2, "p1", 3, &mutex[0], 10*QUANTUM, 1000);
    pid[2] = create((void *)delay_run, INITSTK, 3, "p1", 3, &mutex[0], 20*QUANTUM, 1000);
	pid[3] = create((void *)delay_run, INITSTK, 4, "p1", 3, &mutex[0], 30*QUANTUM, 1000);
	pid[4] = create((void *)delay_run, INITSTK, 5, "p1", 3, &mutex[0], 40*QUANTUM, 1000);
    pid[5] = create((void *)delay_run, INITSTK, 10, "p1", 3, &mutex[0], 60*QUANTUM, 1000);

	/* starts 1st set of threads */
	for (i = 0; i < 2*N; i++) {
		resume(pid[i]);
	}

	for (i = 0; i < 2*N; i++) {
		receive();
	}
	

	kprintf("\n\n=========== TEST with 6 varying prio threads  ===================\n\n"); // last half doesn't work :(
	
	/* first deadlock: 3 threads */	
	pid[0] = create((void *)sleep_run, INITSTK, 1, "p1", 3, &mutex[1], QUANTUM, 1000);
	pid[1] = create((void *)sleep_run, INITSTK, 5, "p1", 3, &mutex[1], 10*QUANTUM, 1000);
    pid[2] = create((void *)sleep_run, INITSTK, 10, "p1", 3, &mutex[1], 20*QUANTUM, 1000);
	pid[3] = create((void *)sleep_run, INITSTK, 1, "p1", 3, &mutex[1], 30*QUANTUM, 1000);
	pid[4] = create((void *)sleep_run, INITSTK, 5, "p1", 3, &mutex[1], 40*QUANTUM, 1000);
    pid[5] = create((void *)sleep_run, INITSTK, 1, "p1", 3, &mutex[1], 50*QUANTUM, 1000);

	/* starts 1st set of threads */
	for (i = 0; i < 2*N; i++) {
		resume(pid[i]);
	}
	//print_queue(readylist);
	for (i = 0; i < 2*N; i++) {
		receive();
		//sync_printf("%d\n", i);
	}

	sleepms(100);

	kprintf("\n\n========== Transitive property with 3 threads (2 locks) ===========\n\n"); //works!
	// try one with multiple locks, 3 threads
	pid[0] = create((void *)delay_run, INITSTK, 1, "p1", 3, &mutex[2], QUANTUM, 1000);
	pid[1] = create((void *)delay_run_2_lock, INITSTK, 5, "p2", 4, &mutex[3], &mutex[2], 5*QUANTUM, 300);
    pid[2] = create((void *)delay_run, INITSTK, 10, "p1", 3, &mutex[3], 500, 2000);

	for (i = 0; i < N; i++) {
		resume(pid[i]);
	}
	
	for (i = 0; i < N; i++) {
		receive();
	}

	sleepms(100);

	kprintf("\n\n=========== Transitive property with 3 locks  ===================\n\n"); //works!
	// try one with multiple locks, 3 threads
	pid[0] = create((void *)delay_run, INITSTK, 1, "p1", 3, &mutex[4], QUANTUM, 1000);
	pid[1] = create((void *)delay_run_2_lock, INITSTK, 5, "p2", 4, &mutex[5], &mutex[4], 5*QUANTUM, 100);
	pid[2] = create((void *)delay_run_2_lock, INITSTK, 10, "p2", 4, &mutex[6], &mutex[5], 150, 100);
    pid[3] = create((void *)delay_run, INITSTK, 20, "p1", 3, &mutex[6], 300, 1000);

	for (i = 0; i < N+1; i++) {
		resume(pid[i]);
	}
	
	for (i = 0; i < N+1; i++) {
		receive();
	}

	kprintf("testcases complete.");

	/* start next set of threads */
	// for(i = N; i < 2*N; i++) {
	// 	resume(pid[i]);
	// }

	/* first deadlock: 3 threads, 2 locks */	
	// pid[3] = create((void *)delay_run, INITSTK, 1, "p1", 3, &mutex[1], QUANTUM, 2000);
	// pid[4] = create((void *)delay_run_2_lock, INITSTK, 5, "p2", 4, &mutex[1], &mutex[2], 10*QUANTUM, 2000);
    // pid[5] = create((void *)delay_run, INITSTK, 10, "p1", 3, &mutex[2], 20*QUANTUM, 2000);

	return OK;
}
#endif