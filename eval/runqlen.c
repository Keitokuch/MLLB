#include <uapi/linux/ptrace.h>
#include <linux/sched.h>

#include "sched.h"

#define KPROBE(fn)  _KPROBE(fn)
#define _KPROBE(fn) kprobe__##fn
#define KRETPROBE(fn)  _KRETPROBE(fn)
#define _KRETPROBE(fn) kretprobe__##fn

BPF_HISTOGRAM(hist, unsigned int);

int KPROBE(load_balance) (struct pt_regs *ctx, int this_cpu, struct rq *this_rq, 
        struct sched_domain *sd, enum cpu_idle_type idle,
        int *continue_balancing)
{
    unsigned int len = 0;

    len = this_rq->cfs.nr_running;

    hist.increment(len);

    return 0;
}

