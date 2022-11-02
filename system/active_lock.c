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
    while(test_and_set(&l->guard, UNAVAILABLE)) {
        //sync_printf("%d sleep\n", currpid);
        sleepms(QUANTUM);
    }
    //sync_printf("%d grabbed guard %d\n", currpid, l);
    if(l->flag == 0) {                          // lock not taken
        l->flag = 1;                            // take lock
        l->holding_pid = currpid;
        //check_deadlock(l);
        l->guard = AVAILABLE;   
        //sync_printf("-------%d grabbed lock %d\n", currpid, l);
    } else {                                    // lock taken
        if(!l->deadlocked) {
            if(check_deadlock(l)) {
                l->deadlocked = TRUE;
            }
        }
        //sync_printf("%d exit ded check \n", currpid);
        enqueue(currpid, l->queue);             // put current thread into lock queue
        //sync_printf("%d put into q\n", currpid);
        set_park();
        //sync_printf("%d park set\n", currpid);
        l->guard = AVAILABLE;                   // allow other threads to proceed
        park();                                 // park
    }
    return OK;
}

syscall al_unlock(al_lock_t *l) {
    while(test_and_set(&l->guard, UNAVAILABLE));                // loop wait 
    //sync_printf("--%d grabbed guard to release %d\n", currpid, );    
    if(isempty(l->queue)) {                                     // if queue empty
        l->flag = 0;                                            // release lock
        l->holding_pid = 0;
        //sync_printf("%d no waiting\n", currpid);
    } else {                                                    // else wakeup thread from queue
        //sync_printf("%d unpark someone\n", currpid);
        pid32 next = dequeue(l->queue);
        l->holding_pid = next;
        unpark(next);
    }
    proctab[currpid].waiting_lock = 0;
    l->guard = AVAILABLE;                                       // reset guard
    return OK;
}

// bool8 al_trylock(al_lock_t *l) {

// }

bool8 check_deadlock(al_lock_t *l) {
    intmask mask;
    mask = disable();
    //kprintf("%d check dead\n", currpid);
    uint16 i = 0;
    pid32 last;
    //qid16 q = newqueue();
    bool8 list[NPROC];
    while(i < NPROC) {
        list[i] = FALSE;
        i++;
    }
    i = 0;
    al_lock_t *lock = l;
    pid32 pid = lock->holding_pid;
    lock = proctab[pid].waiting_lock;
    //insert(pid, q, pid);
    list[pid] = TRUE;
    //kprintf("Check deadlock %d, %d\n", currpid, num_al);
    //kprintf("--%d, %d, %d\n", i, l, pid);
    while(i < num_al) {
        if(lock->deadlocked) {
            return TRUE;
        }
        last = pid;
        pid = lock->holding_pid;
        if(pid == 0 || pid == last) {
            //kprintf("**No deadlock %d", pid);
            return FALSE;
        }
        //insert_single(pid, q, pid);
        list[pid] = TRUE;
        if(pid == currpid || l == lock) {
            //kprintf("deadlock_detected=");
            //print_queue(q);
            // if(!l->deadlocked) {
            print_deadlock(list);
            //     l->deadlocked = TRUE;
            // }
            return TRUE;
        }
        lock = proctab[pid].waiting_lock;
        if(lock == 0) {
            //kprintf("*No deadlock %d\n", currpid);
            return FALSE;
        }
        //kprintf("%d, %d, %d\n", i, l, pid);
        i++;
    }
    //kprintf("No deadlock %d", currpid);
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
    kprintf("%d deadlock_detected=", currpid);
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