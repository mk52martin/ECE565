/*  main.c  - main */
/*note new name, main_og*/

#include <xinu.h>
#include <stdarg.h>

void sync_printf(char *fmt, ...)
{
    	intmask mask = disable();
	void *arg = __builtin_apply_args();
	__builtin_apply((void*)kprintf, arg, 100);
	restore(mask);
}

process	main(void)
{
	pid32	shpid;		/* Shell process ID */

	printf("\n\n");

	/* Create a local file system on the RAM disk */

	lfscreate(RAM0, 40, 20480);

	/* Run the Xinu shell */

	recvclr();
	resume(shpid = create(shell, 8192, 50, "shell", 1, CONSOLE));

	/* Wait for shell to exit and recreate it */

	while (TRUE) {
	    if (receive() == shpid) {
		sleepms(200);
		kprintf("\n\nMain process recreating shell\n\n");
		resume(shpid = create(shell, 4096, 20, "shell", 1, CONSOLE));
		//kprintf(shpid);
		//stacktrace(shpid);
	    }
	}
	return OK;
}





