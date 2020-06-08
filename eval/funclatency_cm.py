#!/usr/bin/env python3

from bcc import BPF
import sys
import logging
import argparse

from datasource import FuncLatencyDatasource

parser = argparse.ArgumentParser()
parser.add_argument('-t', '--tag', help='tag for output')
parser.add_argument('-o', '--output', help='output file name (overwrites -t)')
parser.add_argument('-a', '--append', action='store_true', help='append to output')
args = parser.parse_args()

write_file = args.output or args.tag and f'latency_cm_{args.tag}.json' or 'output'

cm_events = []
latency_datasource = FuncLatencyDatasource(append=args.append,
                                           write_file=write_file)

bpf_text = "funclatency.c"

# initialize BPF & probes
b = BPF(src_file=bpf_text)
b.attach_kprobe(event='can_migrate_task', fn_name='trace_func_enter')
b.attach_kretprobe(event='can_migrate_task', fn_name='trace_func_return')


def funclatency_handler(cpu, data, size):
    event = b['bpf_output'].event(data)
    cm_events.append(event)


# set up perf buffers
b['bpf_output'].open_perf_buffer(funclatency_handler, page_cnt=256)

logger = logging.getLogger('dump')
fhdlr = logging.FileHandler('dump.log')
shdlr = logging.StreamHandler()
logger.addHandler(fhdlr)
logger.addHandler(shdlr)
while True:
    try:
        b.perf_buffer_poll()
        for event in cm_events:
            latency_datasource.update(event)
        cm_events = []
    except KeyboardInterrupt:
        print(latency_datasource.dump())
        exit()
    except Exception as e:
        print(e)
        exit()
        logger.warn(e)
