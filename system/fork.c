/* fork.c - fork */

#include <xinu.h>
#include <stdarg.h>
/*------------------------------------------------------------------------
 *  create  -  Create a process to start running a function on x86
 *------------------------------------------------------------------------
 */
pid32 fork_1()
{
    struct procent *parent_prptr = &proctab[currpid];
	//printf("Ptr: %x\n", parent_prptr->prstkptr);
    unsigned long *it;
    //stacktrace(currpid);
    asm("movl %%ebp, %0\n" :"=r"(it));
    it++;
    unsigned long *pass = (*it);
    //pass--;
    printf("@(%x) %x", pass, *pass);
    pid32 pid;
    struct procent *prptr;
    pid = create(
        pass,
        parent_prptr->prstklen,         // Stack Size
        parent_prptr->prprio,           // Stack Priority
        parent_prptr->prname,           // Stack Name
        1                           //TODO: Fix to arg number.
    );
    if(pid == SYSERR) return SYSERR;


    //stacktrace(pid);
    prptr = &proctab[pid];
    //child EAX
    //stacktrace(pid);
    printf("Base: %x, Ptr: %x", prptr->prstkbase, prptr->prstkptr);
    *((prptr->prstkbase)-24) = NPROC;
    
    resume(pid);
    //printf("resumed!");
	return pid;
}

pid32 fork_2() {
    struct procent *parent_prptr = &proctab[currpid];
    pid32 pid;
    pid = create(
        INITRET,
        parent_prptr->prstklen,         // Stack Size
        parent_prptr->prprio,           // Stack Priority
        parent_prptr->prname,           // Stack Name
        1                           //TODO: Fix to arg number.
    );

    if (pid != 0 && isbadpid(pid)) return SYSERR;
    sync_printf("Pid: %d\n", pid);
    struct procent *child_prptr = &proctab[pid];
    unsigned long **c_sp, **sp, **fp;
    // parent is current pid
	asm("movl %%ebp, %0\n" :"=r"(fp));
    sync_printf("1. sp: %x, fp: %x, sp*: %x, fp*: %x\n", sp, fp, *sp, *fp);
    sp = fp;
    sp -= 2;
    fp = *fp;
    //fp = *fp;
    sync_printf("2. sp: %x, fp: %x, parent_sp: %x\n", sp, fp, parent_prptr->prstkptr);
    //unsigned long size = (unsigned long)(parent_prptr->prstkbase) - (unsigned long)sp;
    void* it = sp;
    c_sp = child_prptr->prstkbase;
    sync_printf("it = %x, c_sp = %x\n", it, c_sp);
    while(it != parent_prptr->prstkbase) {      //equalize w/ parent stkptr
        c_sp--;
        it= it + 4;
        //sync_printf("--it = %x, c_sp = %x\n", it, c_sp);
    }
    //c_sp--;
    sync_printf("it = %x, c_sp = %x\n", it, c_sp);
    //c_sp = child_prptr->prstkbase-parent_prptr->prstklen;
    //child_prptr->prstkptr = (char*)c_sp;
    sync_printf("c_sp = %x, base= %x\n", c_sp, child_prptr->prstkbase);
    //stacktrace(currpid);
    child_prptr->prstkptr = c_sp;
    while((unsigned long)sp <= (unsigned long)(parent_prptr->prstkbase)) {
        *c_sp = *sp;
        sync_printf("c_sp=%x, *c_sp=%x, sp=%x, *sp=%x\n", c_sp, *c_sp, sp, *sp);
        c_sp++;
        sp++;
    }
    *c_sp = *sp;
    sync_printf("c_sp=%x, *c_sp=%x, sp=%x, *sp=%x\n", c_sp, *c_sp, sp, *sp);
    sync_printf("prstptr_child = %x, base_child = %x\n", child_prptr->prstkptr,child_prptr->prstkbase); 
    sync_printf("child stacktrace -----------\n");
    //stacktrace(pid);
    sync_printf("prstptr_child = %x, base_child = %x\n", child_prptr->prstkptr,child_prptr->prstkbase); 
    sync_printf("child stacktrace over-------\n");
    sync_printf("parent stacktrace -----------\n");
    //stacktrace(currpid);
    sync_printf("parent stacktrace over-------\n");
    
	// child pic
	/*
    child_sp = (unsigned long *)child_prptr->prstkptr;
	child_fp = child_sp + 2; 		
    sync_printf("Parent: sp (%x), fp (%x), stkbase (%x)", parent_sp, parent_fp, parent_prptr->prstkptr);
    sync_printf("Child: sp (%x), fp (%x), stkbase (%x)", child_sp, child_fp, child_prptr->prstkptr);
    while (child_sp < (unsigned long *)child_prptr->prstkbase) { 
        for (; child_sp < child_fp; child_sp++) {}
        if(*child_sp == STACKMAGIC) return SYSERR;
        child_fp = (unsigned long *) *child_sp++;
        if (child_fp <= child_sp) {
			sync_printf("bad stack, fp (%08X) <= sp (%08X) for pid %d\n", child_fp, child_sp, pid);
			return SYSERR;
		}
        child_sp++;
    }*/
    //sync_printf("Movement Happened-----");
    //sync_printf("Child: sp (%x), fp (%x), stkbase (%x)", child_sp, child_fp, child_prptr->prstkptr);
    //stacktrace(currpid);
    return pid;
}


pid32 fork_3() {
    sync_printf("\n\nIn Fork\n\n");
    stacktrace(currpid);
    //
    // create process (init stack)
    //
    struct procent *parent_prptr = &proctab[currpid];
    pid32 pid;
    pid = create(                       // create process (init stack)
        INITRET,
        parent_prptr->prstklen,         // Stack Size
        parent_prptr->prprio,           // Stack Priority
        parent_prptr->prname,           // Stack Name
        1                           //TODO: Fix to arg number.
    );
    if (pid != 0 && isbadpid(pid)) return SYSERR;
    sync_printf("Pid: %d\n", pid);

    //
    //gather info to create stack [bottom-up]
    //
    struct procent *child_prptr = &proctab[pid];
    unsigned long **c_sp, **sp, **fp;
	asm("movl %%esp, %0\n" :"=r"(sp));
    asm("movl %%ebp, %0\n" :"=r"(fp));
    sync_printf("1. sp: %x, fp: %x, sp*: %x, fp*: %x\n", sp, fp, *sp, *fp);
    void* it = sp;                                                              // iterator (moves w/ +-4)
    c_sp = child_prptr->prstkbase;
    sync_printf("2. it = %x, c_sp = %x\n", it, c_sp);
    while(it != parent_prptr->prstkbase) {                                      //equalize w/ parent stkptr
        c_sp--;
        it= it + 4;
        //sync_printf("--it = %x, c_sp = %x\n", it, c_sp);
    }
    //slow zone
    //c_sp++;
    //return to speed
    sync_printf("3. it = %x, c_sp = %x\n", it, c_sp);                           // HERE - c_sp should be at sp point for child
    child_prptr->prstkptr = c_sp;                                               // setting sp
    sync_printf("4. CHILD: base = %x, c_sp = %x\n", child_prptr->prstkbase, c_sp);

    //
    // time to iterate
    //
    unsigned long *temp;
    sync_printf("5. sp=%x, fp=%x, parent_base=%x\nIterate Begin\n", sp, fp, parent_prptr->prstkbase);
    //stacktrace(currpid);
    while(*sp != fp){
        *c_sp = *sp;
        sync_printf("-sp=%x, fp=%x, c_sp=(%x) %x\n", sp, fp, c_sp, *c_sp);
        sp++;
        c_sp++;
    }
    //Work in Progress
    temp = sp;
    //sync_printf("temp=%x, *sp=%x, *sp-temp=%x\n", temp, *sp, (*sp-temp));
    *c_sp = c_sp + (*fp-temp);
    //sync_printf("c_sp=%x, *c_sp=%x\n", c_sp, *c_sp);
    sp++;
    c_sp++;
    // Return to speed.
    //sync_printf("Init Over.\n");
    while(sp < parent_prptr->prstkbase) {
        //sync_printf("-sp=%x, fp=%x, c_sp=%x\n", sp, fp, c_sp);
        while (sp < fp) {
            *c_sp = *sp;
            //sync_printf("--sp=%x, fp=%x, c_sp=(%x) %x\n", sp, fp, c_sp, *c_sp);
            sp++;
            c_sp++;
            if(*sp == STACKMAGIC) break;
        }
        //sync_printf("hit fp\n");
        temp = fp;
        sync_printf("sp=%x, *fp=%x, *fp-temp=%x\n", sp, *fp, (*fp-temp));
        if(*fp == STACKMAGIC) {
            break;
        }
        *c_sp = c_sp + (*fp-temp);
        sync_printf("c_sp=%x, *c_sp=%x\n", c_sp, *c_sp);
        fp = *fp;
        sp++;
        c_sp++;
    }
    *c_sp = STACKMAGIC;
    //sync_printf("child base: %x, Value: %x\n", child_prptr->prstkbase, *(child_prptr->prstkbase));
    //*(child_prptr->prstkbase) = c_sp;                                   //corrupt????
    //stacktrace(pid);
    unsigned long *address = (child_prptr->prstkptr);
    //sync_printf("address: %x, shift: %x, val(shift): %d\n", address, address+7, *(child_prptr->prstkptr+7));
    *(address+7) = NPROC;
    //sync_printf("updated to: %d, supposed to be: %d\n", *(child_prptr->prstkptr+7), NPROC);
    sync_printf("\n\nFORK STACK\n\n\n");
    stacktrace(pid);
    //resume(pid);
    return pid;
}

pid32 fork() {
    sync_printf("\n\nIn Fork\n\n");
    stacktrace(currpid);
    
    //
    // create process (init stack)
    //
    struct procent *parent_prptr = &proctab[currpid];
    pid32 pid;
    pid = create(                       // create process (init stack)
        INITRET,
        parent_prptr->prstklen,         // Stack Size
        parent_prptr->prprio,           // Stack Priority
        parent_prptr->prname,           // Stack Name
        1                           //TODO: Fix to arg number.
    );
    if (pid != 0 && isbadpid(pid)) return SYSERR;
    sync_printf("Pid: %d\n", pid);
    struct procent *child_prptr = &proctab[pid];
    //
    //gathering info to create stack [bottom-up]
    //
    unsigned long **c_sp, **sp, **fp;
	asm("movl %%esp, %0\n" :"=r"(sp));
    asm("movl %%ebp, %0\n" :"=r"(fp));
    while(*sp != fp) {
        sp++;
    } 
    sp -= 2;
    sync_printf("1. sp: %x, fp: %x, sp*: %x, fp*: %x\n", sp, fp, *sp, *fp);
    void* it = sp;                                                              // iterator (moves w/ +-4)
    c_sp = child_prptr->prstkbase;
    sync_printf("2. it = %x, c_sp = %x\n", it, c_sp);
    while(it != parent_prptr->prstkbase) {                                      //equalize w/ parent stkptr
        c_sp--;
        it = it + 4;
        //sync_printf("--it = %x, c_sp = %x\n", it, c_sp);
    }
    //slow zone
    //c_sp++;
    //return to speed
    sync_printf("3. it = %x, c_sp = %x\n", it, c_sp);                           
    //*(--c_sp) = 0;
    //*(--c_sp) = 0;
    child_prptr->prstkptr = c_sp;       // setting sp, adjust later
    uint32 *max, *min;
    min = sp;
    max = parent_prptr->prstkbase;
    uint32 offset;
    if(max > (uint32*)child_prptr->prstkbase) {
        offset = max - (uint32*)child_prptr->prstkbase;
    } else {
        offset = (uint32*)child_prptr->prstkbase - max;
    }
    sync_printf("4. CHILD: base = %x, c_sp = %x\n", child_prptr->prstkbase, c_sp);
    sync_printf("Min = %x, Max = %x\n", min, max);
    //
    // time to iterate
    //
    unsigned long *temp;
    //fp = sp + 2;
    sync_printf("5. sp=%x, fp=%x, parent_base=%x\nIterate Begin\n", sp, fp, parent_prptr->prstkbase);
    //stacktrace(currpid);
    while(sp != fp){
        if(*sp >= min && *sp <= max) {
            *c_sp = (*sp) - offset;
        } else { *c_sp = *sp; }
        sync_printf("sp=%x, fp=%x, c_sp=(%x) %x\n", sp, fp, c_sp, *c_sp);
        sp++;
        c_sp++;
    }
    //Work in Progress
    //temp = sp;
    //sync_printf("temp=%x, *sp=%x, *sp-temp=%x\n", temp, *sp, (*sp-temp));
    //*c_sp = c_sp + (*fp-temp);
    //sync_printf("c_sp=%x, *c_sp=%x\n", c_sp, *c_sp);
    temp = fp;
    sync_printf("fp=%x, *fp=%x, *sp-temp=%x\n", fp, *fp, (*fp-temp));
    *c_sp = c_sp + (*fp-temp);
    fp = *fp;
    sp++;
    c_sp++;
    // Return to speed.5.
    //sync_printf("Init Over.\n");
    while(sp < parent_prptr->prstkbase) {
        //sync_printf("-sp=%x, fp=%x, c_sp=%x\n", sp, fp, c_sp);
        while (sp < fp) {
            if(*sp >= min && *sp <= max) {
                *c_sp = (*sp) - offset;
            } else { *c_sp = *sp; }
            //sync_printf("--sp=%x, fp=%x, c_sp=(%x) %x\n", sp, fp, c_sp, *c_sp);
            sp++;
            c_sp++;
            if(*sp == STACKMAGIC) break;
        }
        //sync_printf("hit fp\n");
        temp = fp;
        sync_printf("fp=%x, *fp=%x, *fp-temp=%x\n", fp, *fp, (*fp-temp));
        if(*fp == STACKMAGIC) {
            *c_sp = child_prptr->prstkbase;
        } else {
            *c_sp = c_sp + (*fp-temp);
        }
        sync_printf("c_sp=%x, *c_sp=%x\n", c_sp, *c_sp);
        fp = *fp;
        sp++;
        c_sp++;
    }
    *--c_sp = STACKMAGIC;
    //sync_printf("child base: %x, Value: %x\n", child_prptr->prstkbase, *(child_prptr->prstkbase));
    //*(child_prptr->prstkbase) = c_sp;                                   //corrupt????
    //stacktrace(pid);
    sync_printf("stack_ptr: %x\n",child_prptr->prstkptr);
    //child_prptr->prstkptr = (child_prptr->prstkptr);// + (4 * 12);                 // generates 12 local vars of GARBAGE
    sync_printf("stack_ptr: %x\n",child_prptr->prstkptr);
    unsigned long *address, *ptr;
    address = (child_prptr->prstkptr);
    //sync_printf("address: %x, shift: %x, val(shift): %d\n", address, address+7, *(child_prptr->prstkptr+7));
    //*(address+7) = NPROC;
    //sync_printf("updated to: %d, supposed to be: %d\n", *(child_prptr->prstkptr+7), NPROC);
    sync_printf("\n\nCOMP STACK\n\n\n");
    stacktrace(currpid);
    sync_printf("\n\nFORK STACK\n\n\n");
    stacktrace(pid);
    // adding register values back in....
    /*child_prptr->prstkptr = address+4;
    ptr = address + 2;
    *--address = 0x00000200;
    //sync_printf("(%x) %x\n", address, *address);
    *--address = NPROC;         // eax
    //sync_printf("(%x) %x\n", address, *address);
    *--address = 0;
    //sync_printf("(%x) %x\n", address, *address);
    *--address = 0;
    //sync_printf("(%x) %x\n", address, *address);
    *--address = 0;
    //sync_printf("(%x) %x\n", address, *address);
    *--address = 0;
    //sync_printf("(%x) %x\n", address, *address);
    *--address = ptr;
    //sync_printf("(%x) %x\n", address, *address);
    ptr = address - 1;
    *--address = 0;
    //sync_printf("(%x) %x\n", address, *address);
    *--address = 0;
    //sync_printf("(%x) %x\n", address, *address);
    *ptr = (unsigned long) (address);
    sync_printf("[BACK](%x) %x", ptr, *ptr);*/
    stacktrace(pid);
    resume(pid);
    return pid;
}
