#!/usr/bin/env python3

from bcc import BPF, PerfType, PerfSWConfig
from time import sleep
import sys
import logging
import argparse

from datasource import MaxImbalanceDatasource

parser = argparse.ArgumentParser()
parser.add_argument('-t', '--tag', help='tag for output')
parser.add_argument('-o', '--output', help='output file name (overwrites -t)')
parser.add_argument('-a', '--append', action='store_true', help='append to output')
parser.add_argument('-v', '--verbose', action='store_true', help='print output')
args = parser.parse_args()

frequency = 800
INTERVAL = 0.1

write_file = args.output or args.tag and f'imbalance_{args.tag}.json' or 'imbalance.json'

#  cm_events = []
imbalance_datasource = MaxImbalanceDatasource(append=args.append,
                                             write_file=write_file)

bpf_text = "sample_qlen.c"

# initialize BPF & probes
b = BPF(src_file=bpf_text)
b.attach_perf_event(ev_type=PerfType.SOFTWARE,
    ev_config=PerfSWConfig.CPU_CLOCK, fn_name="do_perf_event",
    sample_period=0, sample_freq=frequency)



logger = logging.getLogger('dump')
fhdlr = logging.FileHandler('dump.log')
shdlr = logging.StreamHandler()
logger.addHandler(fhdlr)
logger.addHandler(shdlr)

exiting = 0
hist = b.get_table("hist")
while True:
    try:
        sleep(INTERVAL)
    except KeyboardInterrupt:
        exiting = 1

    rqlen = [k.value for k, v in hist.items()]
    imbalance = max(rqlen) - min(rqlen) if rqlen else 0
    imbalance_datasource.update(imbalance)
    if args.verbose:
        print(rqlen)
        print(imbalance, end=' ')
    hist.clear()
    if exiting:
        print(imbalance_datasource.dump())
        exit()
