/*  main.c  - main */

#include <xinu.h>
#define TEST_SYSCALL_PRINT_READY_LIST 1

process spin(void) {
	while(1);
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
	return OK;
}
