/* lock.h - lock globals */

/* Maximum number of processes in the system */
#ifndef NSPINLOCKS
#define	NSPINLOCKS		20
#endif		

// spinlock globals
#define AVAILABLE       FALSE
#define UNAVAILABLE     TRUE

// counter for spinlocks
uint32	num_sl;		/* number of spinlocks	            */