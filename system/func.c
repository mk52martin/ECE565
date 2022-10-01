/* func.c - burst_execution */
#include <xinu.h>
#define DEBUG 0
#define MAIN_SYNC 0

#if !MAIN_SYNC
void sync_printf(char *fmt, ...)
{
        intmask mask = disable();
        void *arg = __builtin_apply_args();
        __builtin_apply((void*)kprintf, arg, 100);
        restore(mask);
}
#endif

void burst_execution(uint32 number_bursts, uint32 burst_duration, uint32 sleep_duration) {
    uint32 i = 0;
    uint32 counter;
    uint32 end_time;
    struct	procent	*prptr;
    prptr = &proctab[currpid];

#if DEBUG
    uint32 time = ((clktime*1000) + ctr1000);
#endif

    for(; i < number_bursts; i++) {
#if DEBUG
        sync_printf("new cycle for %d @%d, runtime: %d\n", currpid, ((clktime*1000) + ctr1000), prptr->runtime);
#endif
        counter = 0;
        end_time = prptr->runtime + burst_duration - 1;
        while(prptr->runtime < end_time) {
            counter++;
        }
#if DEBUG
        sync_printf("end burst cycle for p%d @ time: %d, @runtime: %d\n", currpid, ((clktime*1000) + ctr1000), prptr->runtime);
#endif
        sleepms(sleep_duration);
#if DEBUG
        sync_printf("end sleep cycle for p%d @time: %d\n", currpid, ((clktime*1000) + ctr1000));
#endif
    }

#if DEBUG
        sync_printf("Total burst function time for p%d: %d\n", currpid, ((clktime*1000) + ctr1000)-time);
#endif

    return;
}