from utils import get_dmesg, get_syslog
import time

DUR = 5

def main():
    dmesg = get_syslog()
    total = 0
    correct = 0
    incorrect = 0
    for line in dmesg:
        line = line.split(' ')
        if 'can_migrate' in line:
            total += 1
            base = line.index('can_migrate')
            ori_cm, jc_cm = line[base+1], line[base+2]
            if ori_cm == jc_cm:
                correct += 1
            else:
                incorrect += 1
                #  print(line)
    print(f'{incorrect} incorrect decisions in {total} triggers')
    return total

start = main()
time.sleep(DUR)
end = main()
triggers = end - start
print(f'{triggers} triggers in {DUR} second. {triggers /DUR} /s')
