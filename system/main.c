/*  main.c  - main */

#include <xinu.h>
#include <stdarg.h>

#define TESTCASE1
#define TESTCASE2
#define TESTCASE3
#define TESTCASE4

uint32 sum(uint32 a, uint32 b){
	return (a+b);
}

void sync_printf(char *fmt, ...)
{
    	intmask mask = disable();
	void *arg = __builtin_apply_args();
	__builtin_apply((void*)kprintf, arg, 100);
	restore(mask);
}

process create_x(int num_child, int grandparent, bool8 end){
	pid32 pid;
    num_child--;
    if (num_child != 0) {
        pid = create((void *)create_x, 8192, 50, "create_x", 2, num_child, grandparent, end);
        resume(pid);
        receive();
    } else {
        if(end) {
            send(grandparent, getpid());
        }
        receive();
    }
    
	return OK;
}

process create_x_with_buddy(int num_child, int grandparent){
	pid32 pid;
    num_child--;
    if (num_child != 0) {
        pid = create((void *)create_x_with_buddy, 8192, 50, "create_x", 2, num_child, grandparent);
        resume(pid);
        receive();
    } else {
        pid = create((void *)create_x, 8192, 50, "create_x", 3, 1, grandparent, FALSE);
        resume(pid);
        pid = create((void *)create_x, 8192, 50, "create_x", 3, 1, grandparent, TRUE);
        resume(pid);
        receive();
    }
    
	return OK;
}

process create_triplets(int num_child, int grandparent, bool8 end){
	pid32 pid;
    num_child--;
    if (num_child != 0) {
        pid = create((void *)create_triplets, 8192, 50, "create_x", 2, num_child, grandparent, FALSE);
        resume(pid);
        pid = create((void *)create_triplets, 8192, 50, "create_x", 2, num_child, grandparent, FALSE);
        resume(pid);
        if(num_child != 1){
            pid = create((void *)create_triplets, 8192, 50, "create_x", 2, num_child, grandparent, FALSE);
        } else {
            pid = create((void *)create_triplets, 8192, 50, "create_x", 2, num_child, grandparent, TRUE);
        }
        resume(pid);
    }
    if(end) {
        send(grandparent, FALSE);
    }
    receive();
	return OK;
}


process test1(uint32 parent){
	pid32 pid;
    pid = create((void *)create_x, 8192, 50, "create_x", 2, 2, parent, FALSE);
    resume(pid);
    pid = create((void *)create_x, 8192, 50, "create_x", 2, 2, parent, TRUE);
    resume(pid);
    receive();
	return OK;
}

process test2(uint32 parent){
	pid32 pid;
    pid = create((void *)create_x, 8192, 50, "create_x", 2, 2, parent, FALSE);
    resume(pid);
    pid = create((void *)create_x, 8192, 50, "create_x", 2, 2, parent, TRUE);
    resume(pid);
    receive();
	return OK;
}

process test3(uint32 parent){
	pid32 pid;
    pid = create((void *)create_x_with_buddy, 8192, 50, "create_x", 2, 3, parent, TRUE);
    resume(pid);
    receive();
	return OK;
}

process test4(uint32 parent){
	pid32 pid;
    pid = create((void *)create_triplets, 8192, 50, "create_triplets", 2, 3, parent, TRUE);
    resume(pid);
    receive();
	return OK;
}

void print_children (void) {
    int i = 4;
    int j;
    bool8 first;
    struct	procent *prptr_child;
    for(; i < NPROC; i++) {
        j = 4;
        first = TRUE;
        for(; j < NPROC; j++) {
            prptr_child = &proctab[j];
            if(prptr_child->prparent == i) {
                if(first) {
                    sync_printf("\n%d:: %d", i, j);
                    first = FALSE;
                } else {
                    sync_printf(", %d", j);
                }
            }
        }
    }
    sync_printf("\n");
    return;
}

process	main(void) {
    sync_printf("\nBEGIN\n");
    pid32 pid, kill_end;
    print_children();

#ifdef TESTCASE1        // passed
    sync_printf("\n[TESTCASE-1]\n");
    pid = create((void *)test1, 8192, 50, "test3", 1, currpid);
    resume(pid);
	receive();
    sync_printf("Structure:");
    print_children();
    kill(pid);
    sync_printf("After killing head (%d):", pid);
    print_children();
	sync_printf("[END-TESTCASE-1]\n\n");
#endif

#ifdef TESTCASE2        // passed
    sync_printf("\n[TESTCASE-2]\n");
    pid = create((void *)test2, 8192, 50, "test2", 1, currpid);
    resume(pid);
	receive();
    //receive();
    sync_printf("Structure:\n");
    print_children();
    sync_printf("After killing intermediate node (%d):", (pid + 1));
    kill(pid + 1);
    print_children();
    kill(pid);
    sync_printf("After killing head (%d):", pid);
    print_children();
	sync_printf("[END-TESTCASE-2]\n\n");
#endif

#ifdef TESTCASE3        // passed
	sync_printf("\n[TESTCASE-3]\n");
    pid = create((void *)test3, 8192, 50, "test3", 1, currpid);
    resume(pid);
	kill_end = receive();
    sync_printf("Structure:");
    print_children();
    kill(kill_end);
    sync_printf("After killing end node (%d):", kill_end);
    print_children();
    kill(pid);
    sync_printf("After killing head (%d):", pid);
    print_children();
	sync_printf("[END-TESTCASE-3]\n\n");
#endif
#ifdef TESTCASE4        // passed
	sync_printf("\n[TESTCASE-4]\n");
    pid = create((void *)test4, 8192, 50, "test3", 1, currpid);
    resume(pid);
	receive();
    sync_printf("Structure:");
    print_children();
    kill(pid);
    sync_printf("After killing head (%d):", pid);
    print_children();
	sync_printf("[END-TESTCASE-4]\n\n");
#endif
	
	return OK;
    
}
