/* lock.h - lock globals */

/* Maximum number of type locks in the system */
#ifndef NSPINLOCKS
#define	NSPINLOCKS		20
#endif		
#ifndef NLOCKS
#define	NLOCKS		    20
#endif	

// spinlock globals
#define AVAILABLE       FALSE
#define UNAVAILABLE     TRUE

// counter for type locks
uint32	num_sl;		/* number of spinlocks	            */
uint32  num_l;      /* number of locks                  */

// lock struct
typedef struct lock_t {
    uint32 flag;
    uint32 guard;
    qid16 queue;
} lock_t;