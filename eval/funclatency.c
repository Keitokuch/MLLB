#include <uapi/linux/ptrace.h>
#include <linux/sched.h>
#include <bcc/proto.h>
#include <linux/bpf.h>
#include <linux/kernel.h>
#include <linux/sched/topology.h>
#include <linux/percpu.h>
#include <linux/sched/mm.h>
#include <linux/percpu.h>
#include <linux/bitmap.h>
#include <linux/lockdep.h>
#include <linux/migrate.h>
#include <linux/mm.h>
#include <linux/smp.h>
#include <linux/percpu-defs.h>
#include <uapi/linux/bpf.h>
#include <linux/jump_label.h>
#include <linux/nodemask.h>
#include <linux/sched/idle.h>


#define KPROBE(fn)  _KPROBE(fn)
#define _KPROBE(fn) kprobe__##fn
#define KRETPROBE(fn)  _KRETPROBE(fn)
#define _KRETPROBE(fn) kretprobe__##fn

struct perf_output {
    u64 delta;
};

struct pid_cpu_key {
    int pid;
    int cpu;
};

/* BPF_HASH(lb_instances, int, struct lb_context); */
BPF_HASH(start, struct pid_cpu_key, u64);

BPF_PERF_OUTPUT(bpf_output);


int trace_func_enter(struct pt_regs *ctx)
{
    int cpu = bpf_get_smp_processor_id();
    u32 pid = bpf_get_current_pid_tgid();
    u64 ts = bpf_ktime_get_ns();

    struct pid_cpu_key key = {pid, cpu};

    start.update(&key, &ts);

    return 0;
}

int trace_func_return (struct pt_regs *ctx)
{
    u64 *tsp, delta;
    int cpu = bpf_get_smp_processor_id();
    u32 pid = bpf_get_current_pid_tgid();
    struct perf_output output;

    struct pid_cpu_key key = {pid, cpu};

    tsp = start.lookup(&key);
    if (!tsp)
        return 0;

    delta = bpf_ktime_get_ns() - *tsp;
    start.delete(&key);

    output.delta = delta;

    bpf_output.perf_submit(ctx, &output, sizeof(output));

    return 0;
}
