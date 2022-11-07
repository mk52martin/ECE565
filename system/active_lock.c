/* active_lock.c - locking mechanisms */

#include <xinu.h>

syscall al_initlock(al_lock_t *l) {
    num_al++;
    if(num_l > NLOCKS) {
        return SYSERR;
    }
    l->flag = AVAILABLE;
    l->guard = AVAILABLE;
    l->queue = newqueue();
    l->deadlocked = FALSE;
    return OK;
}

syscall al_lock(al_lock_t *l) {
    proctab[currpid].waiting_lock = l;
    //sync_printf("%d attempt lock %d\n", currpid, l);
    while(test_and_set(&l->guard, UNAVAILABLE)) {
        sleepms(QUANTUM);
    }
    //sync_printf("%d lock %d\n", currpid, l);
    if(l->flag == 0) {                          // lock not taken
        l->flag = 1;                            // take lock
        l->holding_pid = currpid;
        l->guard = AVAILABLE;   
    } else {                                    // lock taken
        if(!l->deadlocked) {
            if(check_deadlock(l)) {
                l->deadlocked = TRUE;
            }
        }
        enqueue(currpid, l->queue);             // put current thread into lock queue
        //sync_printf("%d put into q\n", currpid);
        set_park();
        l->guard = AVAILABLE;                   // allow other threads to proceed
        park();                                 // park
        //sync_printf("%d unparked!\n", currpid);
    }
    return OK;
}

syscall al_unlock(al_lock_t *l) {
    //sync_printf("**%d attempt unlock %d\n", currpid, l);
    while(test_and_set(&l->guard, UNAVAILABLE));                // loop wait 
    //sync_printf("**%d unlocking %d\n", currpid, l); 
    if(isempty(l->queue)) {                                     // if queue empty
        l->flag = 0;                                            // release lock
        l->holding_pid = 0;
        //sync_printf("%d no waiting for unlock\n", currpid);
    } else {                                                    // else wakeup thread from queue
        pid32 next = dequeue(l->queue);
        l->holding_pid = next;
        //sync_printf("%d unpark %d\n", currpid, next);
        unpark(next);
    }
    proctab[currpid].waiting_lock = 0;
    l->guard = AVAILABLE;                                       // reset guard
    return OK;
}

bool8 al_trylock(al_lock_t *l) {
    //proctab[currpid].waiting_lock = l;
    while(test_and_set(&l->guard, UNAVAILABLE)) {
        sleepms(QUANTUM);
    }
    if(l->flag == 0) {                          // lock not taken
        l->flag = 1;                            // take lock
        l->holding_pid = currpid;
        l->guard = AVAILABLE;   
        return TRUE;
    } else {                                    // lock taken
        l->guard = AVAILABLE;                   // allow other threads to proceed
        return FALSE;
    }
    return FALSE;
}

bool8 check_deadlock(al_lock_t *l) {
    intmask mask;
    mask = disable();
    uint16 i = 0;
    pid32 last;
    bool8 list[NPROC];
    while(i < NPROC) {
        list[i] = FALSE;
        i++;
    }
    i = 0;
    al_lock_t *lock = l;
    pid32 pid = lock->holding_pid;
    lock = proctab[pid].waiting_lock;
    list[pid] = TRUE;
    while(i < num_al) {
        if(lock->deadlocked) {
            restore(mask);
            return TRUE;
        }
        last = pid;
        pid = lock->holding_pid;
        if(pid == 0 || pid == last) {
            restore(mask);
            return FALSE;
        }
        list[pid] = TRUE;
        if(pid == currpid || l == lock) {
            print_deadlock(list);
            restore(mask);
            return TRUE;
        }
        lock = proctab[pid].waiting_lock;
        if(lock == 0) {
            restore(mask);
            return FALSE;
        }
        i++;
    }
    restore(mask);
    return FALSE;
}

void print_queue(qid16 q) {
	qid16 tail = queuetail(q);												//find head
	qid16 it = firstid(q);	
	if(it == tail) {
		kprintf("Empty.\n");
		return;
	}		
    //kprintf("tail: %d \n", tail);					
	kprintf("P%d", it);
	while(queuetab[it].qnext != tail) {												// cycle through readylist
		it = queuetab[it].qnext;
		kprintf("-P%d", it);	
	}
	kprintf("\n");
}

void print_deadlock(bool8 list[NPROC]) {
    kprintf("deadlock_detected=");
    int i = 0;
    bool8 first = TRUE;
    while(i < NPROC) {
        if(list[i]) {
            if(!first) {
                kprintf("-");
            } else {
                first = FALSE;
            }
            kprintf("P%d", i);
        }
        i++;
    }
    kprintf("\n");
}