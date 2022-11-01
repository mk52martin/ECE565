/* spinlock.c - locking mechanisms */

#include <xinu.h>

syscall sl_initlock(sl_lock_t *l) {
    num_sl++;
    if(num_sl > 20) {
        return SYSERR;
    }
    *l = AVAILABLE;
    return OK;
}

syscall sl_lock(sl_lock_t *l) {
    // bool8 res;
    while(test_and_set(l, UNAVAILABLE)) {
        //sync_printf("p%d: %d\n", currpid, res);
    }
    //kprintf("p%d: obtained\n", currpid);
    return OK;
}

syscall sl_unlock(sl_lock_t *l) {
    *l = AVAILABLE;
    return OK;
}
