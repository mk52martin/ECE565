/* lock.c - locking mechanisms */

#include <xinu.h>

syscall sync_print(char *fmt, ...)
{
        intmask mask = disable();
        void *arg = __builtin_apply_args();
        __builtin_apply((void*)kprintf, arg, 100);
        restore(mask);
        return OK;
}

syscall pi_initlock(struct pi_lock_t *l) {
    num_pi++;
    if(num_pi > NLOCKS) {
        return SYSERR;
    }
    l->flag = AVAILABLE;
    l->guard = AVAILABLE;
    l->queue = newqueue();
    return OK;
}

syscall pi_lock(struct pi_lock_t *l) {
    proctab[currpid].waiting_lock_pi = l;
    while(test_and_set(&l->guard, UNAVAILABLE)) {
        sleepms(QUANTUM);
    }
    if(l->flag == 0) {                                          // lock not taken
        l->flag = 1;                                            // take lock
        l->holding_pid = currpid;
        // sync_printf("%d grab.", currpid);
        l->guard = AVAILABLE;   
    } else {                                                    // lock taken
        // sync_printf("%d wait.\n", currpid, l);
        enqueue(currpid, l->queue);                             // put current thread into lock queue
        // intmask mask = disable();
        // kprintf("insert P%d into q\n", currpid);
        // print_queue_full(l->queue);
        // restore(mask);
        uint32 high = find_highest_prio(l->queue);
        // mask = disable();
        // kprintf("insert P%d into q\n", currpid);
        // print_queue(l->queue);
        // restore(mask);
        // sync_printf("high prio: %d\n", high);
        pid32 holding = l->holding_pid;
        if(high > proctab[holding].prprio) {
            sync_print("priority change=P%d::%d=%d\n", holding, proctab[holding].prprio, high);
            proctab[holding].prprio = high;
            queuetab[holding].qkey = high;
            // add code to loop through waiting locks
            pass_down_prio(l, high);
        }
        //kprintf("%d prio before park:%d\n", currpid, proctab[currpid].prprio);
        //kprintf("%d holding prio before park:%d\n", holding, proctab[holding].prprio);
        set_park();
        l->guard = AVAILABLE;                                   // allow other threads to proceed
        park();                                                 // park
    }
    return OK;
}

syscall pi_unlock(pi_lock_t *l) {
    while(test_and_set(&l->guard, UNAVAILABLE)) {
        sleepms(QUANTUM);
    }
    if(proctab[currpid].prprio != proctab[currpid].progprio) {
        sync_print("priority change=P%d::%d=%d\n", currpid, proctab[currpid].prprio, proctab[currpid].progprio);
    }
    if(isempty(l->queue)) {                                     // if queue empty
        l->flag = 0;
        l->holding_pid = 0;
    } else {                                                    // else wakeup thread from queue
        uint32 high = find_highest_prio(l->queue);
        // intmask mask = disable();
        // print_queue_full(l->queue);
        pid32 next = pop(l->queue); //problem spot
        // kprintf("next: P%d\n", next);
        // print_queue_full(l->queue);
        // restore(mask);
        l->holding_pid = next;
        if(high != proctab[next].prprio) {
            sync_print("priority change=P%d::%d=%d\n", next, proctab[next].prprio, high);
            proctab[next].prprio = high;
            queuetab[next].qkey = high;
            // pass_down_prio(l, high);
        }
        unpark(next);
    }                                          
    proctab[currpid].waiting_lock_pi = 0;
    proctab[currpid].prprio = proctab[currpid].progprio;
    queuetab[currpid].qkey = proctab[currpid].progprio;
    l->guard = AVAILABLE;                                       // reset guard
    return OK;
}

uint32 find_highest_prio(qid16 q) {
	qid16 tail = queuetail(q);												//find head
	qid16 it = firstid(q);	
    uint32 high = queuetab[it].qkey;
    // sync_printf("-it: %d tail: %d, key: %d\n", it, tail, queuetab[it].qkey);
	if(it == tail || it > NPROC || it <= 0) {
		return 0;
	}
	while(queuetab[it].qnext != tail && queuetab[it].qnext <= NPROC && queuetab[it].qnext > 0) {												// cycle through readylist
		it = queuetab[it].qnext;
        // sync_printf("--%d prio: %d\n", it, queuetab[it].qkey);
        if(high < queuetab[it].qkey) {
            high = queuetab[it].qkey;
        }
	}
	return high;
}

void print_queue_full(qid16 q) {
	qid16 tail = queuetail(q);												//find head
	qid16 it = q;//firstid(q);	
	if(it == tail) {
		kprintf("Empty.\n");
		return;
	}		
    //kprintf("tail: %d \n", tail);					
	kprintf("%d: ", it);
    kprintf("key:%d next:%d prev:%d\n", queuetab[it].qkey, queuetab[it].qnext, queuetab[it].qprev);
	while(queuetab[it].qnext != tail) {												// cycle through readylist
		it = queuetab[it].qnext;
        kprintf("%d: ", it);
        kprintf("key:%d next:%d prev:%d\n", queuetab[it].qkey, queuetab[it].qnext, queuetab[it].qprev);
	}
}

void pass_down_prio(pi_lock_t *l, uint32 prio) {
    // intmask mask = disable();
    //kprintf("pass down %d\n", prio);
    pi_lock_t *lock = l;
    pid32 pid = lock->holding_pid;
    lock = proctab[pid].waiting_lock_pi;
    //kprintf("pid:%d, lock: %d, l: %d\n", pid, lock, l);
    int i = 0;
    while(i < num_pi) { // will fail in the case of deadlock
        pid = lock->holding_pid;
        //kprintf("pass down %d to holding pid: %d\n", prio, pid);
        //kprintf("info:\npid:%d, lock->holding:%d\n lock:%d, wait_lock%d\n", pid, lock->holding_pid, lock, proctab[pid].waiting_lock_pi);
        if(pid == 0) {
            // restore(mask);
            return ;
        }
        if(proctab[pid].prprio < prio) {
            sync_print("priority change=P%d::%d=%d\n", pid, proctab[pid].prprio, prio);
            proctab[pid].prprio = prio;
            queuetab[pid].qkey = prio;
        }
        lock = proctab[pid].waiting_lock_pi;
        if(lock == 0) {
            // restore(mask);
            return;
        }
        i++;
    }
    // restore(mask);
    return;
}

