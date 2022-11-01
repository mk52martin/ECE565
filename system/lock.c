/* spinlock.c - locking mechanisms */

#include <xinu.h>

syscall initlock(struct lock_t *l) {
    num_l++;
    if(num_l > NLOCKS) {
        return SYSERR;
    }
    l->flag = AVAILABLE;
    l->guard = AVAILABLE;
    l->queue = newqueue();
    return OK;
}

syscall lock(struct lock_t *l) {
    while(test_and_set(&l->guard, UNAVAILABLE)) {
        //sync_printf("%d sleep\n", currpid);
        sleepms(QUANTUM);
    }
    //sync_printf("%d grabbed guard\n", currpid);
    if(l->flag == 0) {                          // lock not taken
        l->flag = 1;                            // take lock
        l->guard = AVAILABLE;   
        //sync_printf("-------%d grabbed lock\n", currpid);
    } else {                                    // lock taken
        enqueue(currpid, l->queue);             // put current thread into lock queue
        set_park();
        l->guard = AVAILABLE;                   // allow other threads to proceed
        park();                                 // park
    }
    return OK;
}

syscall unlock(struct lock_t *l) {
    while(test_and_set(&l->guard, UNAVAILABLE));                // loop wait 
    //sync_printf("--%d grabbed guard to release %d\n", currpid, );    
    if(isempty(l->queue)) {                                     // if queue empty
        l->flag = 0;                                            // release lock
        //sync_printf("%d no waiting\n", currpid);
    } else {                                                    // else wakeup thread from queue
        //sync_printf("%d unpark someone\n", currpid);
        unpark(dequeue(l->queue));
    }
    l->guard = AVAILABLE;                                       // reset guard
    return OK;
}


syscall set_park() {
    proctab[currpid].park_flag = TRUE;
    return OK;
}

syscall park() {
    intmask mask;
    mask = disable();
    if(proctab[currpid].park_flag) {
        //kprintf("%d parking\n", currpid);
        proctab[currpid].prstate = PR_WAIT;
        resched();
    }
    restore(mask);
    return OK;
}

syscall unpark(pid32 pid) {
    //sync_printf("%d unparked\n", pid);
    intmask mask;
    mask = disable();
    proctab[currpid].park_flag = FALSE;
    ready(pid);
    restore(mask);
    return OK;
}