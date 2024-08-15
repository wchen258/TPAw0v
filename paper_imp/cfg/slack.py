"""
For final disparity run
    python3 slack.py ../rpu_output/disparity_bw.out

For final sift run
    python3 slack.py ../rpu_output/sift.final.nominal

For final tracking run
    python3 slack.py ../rpu_output/tracking.slack.out

For final mser run
    python3 slack.py ../rpu_output/mser.slack.out
"""



import sys
import matplotlib.pyplot as plt

def preprocess(fname):
    with open(fname, 'r') as f:
        raw = f.read().splitlines()

    logs = []
    cur_log = []
    on = False
    for l in raw:
        if l == 'LOG BEGIN':
            on = True
            continue

        if l == 'LOG END':
            on = False
            logs.append(cur_log)
            cur_log = []

        if on:
            cur_log.append(l)

    return logs

def preprocess_log(log):
    for pivot,val in enumerate(log):
        if val[0] == '#':
            break
    
    get_t = lambda x: int(x.split(',')[2])
    get_rt = lambda x: int(x.split(',')[3])/1000
    get_nt = lambda x: int(x.split(',')[4])/1000
    get_tt = lambda x: int(x.split(',')[5])/1000
    get_addr = lambda x: int(x.split(',')[1], 16)

    real_times = []
    nominal_times = []
    tail_times = []
    for i in range(pivot):
        real_times.append(get_rt(log[i]))
        nominal_times.append(get_nt(log[i]))
        tail_times.append(get_tt(log[i]))

    return real_times, nominal_times, tail_times

def plot_vshog(real_times, real_times_hog, nominal_times):
    xs = list(range(len(real_times)))
    plt.plot(xs, real_times, color='blue', label='real_time_normal')
    plt.plot(xs, real_times_hog, color='red', label='real_time_hog')
    plt.plot(xs, nominal_times, color='green', label='nominal')
    plt.xlabel('milestone hit')
    plt.ylabel('elapse (ms)')
    plt.title('Disparity Milestone Reached Diagram')
    plt.legend()
    plt.show()


def plot_vstail(real_times, tail_times, nominal_times):
    xs = list(range(len(real_times)))
    plt.plot(xs, real_times, color='#8fbc8f', label='real_time')
    plt.plot(xs, tail_times, color='#d73a22', label='tail')
    plt.plot(xs, nominal_times, color='#e37564', label='nominal')
    plt.xlabel('milestone hit')
    plt.ylabel('elapse (ms)')
    plt.title('Mser Milestone Reached Diagram')
    plt.legend()
    plt.show()

if __name__=='__main__':
    logs = preprocess(sys.argv[1])
    bm_name = sys.argv[2]
    print(len(logs))
    
    if bm_name == 'tracking':
        ctrl_rt, nt, tail_t = preprocess_log(logs[-2])
        uctrl_rt, nt, _ = preprocess_log(logs[-1])
    else:
        ctrl_rt, nt, tail_t = preprocess_log(logs[-1])
        uctrl_rt, nt, _ = preprocess_log(logs[-2])
    #normal_rt, nt, _ = preprocess_log(logs[-3])

    setpt_u = list(map(lambda x: 50 + 1.3 * x, nt))
    setpt_l = list(map(lambda x: -50 + 1.3 * x, nt))
    setpt = list(map(lambda x: 1.3 * x, nt))

    xs = list(range(len(ctrl_rt)))
    plt.plot(xs, nt, color = 'green', label='perfect run (nominal)')
    plt.plot(xs, setpt, color = '#707070', label='set-point')
    plt.plot(xs, setpt_u, color = '#c0c0c0', label='set-point upper bound', linestyle='dashed')
    plt.plot(xs, setpt_l, color = '#c0c0c0', label='set-point lower bound', linestyle='dashed')
    plt.plot(xs, ctrl_rt, color = '#0000ff', label='interferance controlled')
    plt.plot(xs, uctrl_rt, color = 'red', label='interferance uncontrolled')
    #plt.plot(xs, normal_rt, color = 'orange', label='normal')
    plt.legend()
    plt.xlabel('# milestone')
    plt.ylabel('execution time (ms)')
    plt.title(f'{bm_name} with Bandwidth Monitored by RPU')
    plt.savefig(f'{bm_name}_demo.png')
    





