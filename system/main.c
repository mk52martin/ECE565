/*  main.c  - main */

#include <xinu.h>
#define TEST_SYSCALL_PRINT_READY_LIST 	0
#define TEST_RUNTIME_TURNAROUND_CTXSW 	1
#define TEST_USER_PROC					0

process spin(void) {
	while(1);
}


process spin_send(pid32 parent) {
	struct	procent	*prptr = &proctab[currpid];
	while(prptr->runtime < 5);
	send(parent, OK);
	while(1);
}

process hello(void) {
	struct	procent	*prptr = &proctab[currpid];
	printf("hello! I am process %d\n", currpid);
	if(prptr->user_process == USER_PROCESS){
		printf("I am a user process.\n");
	} else {
		printf("I am a system process.\n");
	}
	return OK;
}

process	main(void)
{
#if TEST_SYSCALL_PRINT_READY_LIST
	uint32 pid = create((void *)spin, 8192, 10, "spin", 0);
	resume(pid);
	uint32 pid2 = create((void *)spin, 8192, 10, "spin", 0);
	resume(pid2);
	printf("New Processes: %d, %d\n", pid, pid2);
	print_ready_list();
#endif
#if TEST_RUNTIME_TURNAROUND_CTXSW
	uint32 pid3 = create_user_process((void *)spin_send, 8192, "spin", 1, currpid);
	resume(pid3);
	uint32 pid4 = create_user_process((void *)spin_send, 8192, "spin", 1, currpid);
	resume(pid4);
	printf("New Processes: %d, %d\n", pid3, pid4);
    print_ready_list();
	receive();
	receive();
	kill(pid3);
	kill(pid4);
#endif
#if TEST_USER_PROC
	uint32 pid5 = create_user_process((void *)hello, 8192, "hello_user", 0);
	uint32 pid6 = create((void *)hello, 8192, 10, "hello_sys", 0);
	resume(pid5);
	resume(pid6);
#endif
	return OK;
}
