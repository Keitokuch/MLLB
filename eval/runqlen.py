#!/usr/bin/env python3

from bcc import BPF, PerfType, PerfSWConfig
import argparse
from time import sleep
import json
from collections import Counter

parser = argparse.ArgumentParser()
parser.add_argument('-t', '--tag', help='tag for output')
parser.add_argument('-o', '--output', help='output file name (overwrites -t)')
parser.add_argument('-a', '--append', action='store_true', help='append to output')
parser.add_argument('-s', '--write_size', type=int, action='store')
parser.add_argument('--old', action='store_true', help='original kernel')
args = parser.parse_args()

write_file = args.output or args.tag and f'runqlen_{args.tag}.json' or 'runqlen_output.json'

frequency = 800

bpf_text = 'sample_qlen.c'

# initialize BPF & probes
b = BPF(src_file=bpf_text)
b.attach_perf_event(ev_type=PerfType.SOFTWARE,
    ev_config=PerfSWConfig.CPU_CLOCK, fn_name="do_perf_event",
    sample_period=0, sample_freq=frequency)

if args.append:
    try:
        with open(write_file, 'r') as f:
            hist_rqlen = Counter(json.load(f))
        print('Loaded from old histogram')
    except:
        hist_rqlen = Counter()
        print('Didn\'t load old histogram')
else:
    hist_rqlen = Counter()


hist = b.get_table("hist")
while True:
    try:
        sleep(3)
    except KeyboardInterrupt:
        new_hist = {str(k.value): v.value for k, v in hist.items()}
        hist_rqlen += Counter(new_hist)
        with open(write_file, 'w') as f:
            json.dump(hist_rqlen, f)
        print('written to', write_file)
        print(hist_rqlen)
        exit()
