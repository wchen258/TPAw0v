from as_cf_utils import BR_INS, UB_INS, CB_INS, BL_INS, RET_INS, ALL_BRANCH_INS
from as_cf_utils import read_as, parse_section
from as_cf_utils import inside
from basic_block import Lean_BB

class Tracer:

    def __init__(self, cfg, strip_trace, limit=-1, watch_points = None, trace_range=None):
        self.history = []
        self.strip_trace = strip_trace
        self.cfg = cfg
        self.cur_bb = None
        self.trace_range = trace_range
        self.stat = {'trace_on_cnt': 0, 'async_cnt': 0, 'interrupt_cnt': 0}
        self.watch_points = watch_points
        self.watch_points_history = []

        if trace_range is None:
            if watch_points is None:
                self.world_line(limit=limit)
            else:
                self.world_line_watch(limit=limit)
        else:
            self.world_line_addr(limit=limit, trace_range = trace_range)
        self.acc_history = self.get_accumulate_history()
        self.rt_access = self.get_rt_access()
        self.cur_elapse = (0, {})

        ## TODO: to save a rt call dict for all bb access is too expensive. We only need the instructions in between
        ## thus make a ongoing counting field, whenever a MS is placed, attach this field to the record BB. 

    def proc(self, e, i):
        if e[0] == 'S':    # Async
            self.stat['async_cnt'] += 1
            self.cur_bb = None
            return 'S'
        elif e[0] == 'A':   # Address
            addr =  int(e[1:], 16)
            self.cur_bb = self.cfg.find_bb(addr)
            self.history.append(self.cur_bb)
            if self.cur_bb is not None:
                self.cur_bb.total_hit += 1
        elif e == 'BE':
            if self.cur_bb is not None:
                end_asm = self.cur_bb.content[-1]
                if end_asm.ins == 'ret' or end_asm.ins == 'br' or end_asm.ins == 'blr':
                    return
                self.cur_bb = self.cur_bb.e_succ_bb
                self.cur_bb.total_hit += 1
            self.history.append(self.cur_bb)
        elif e == 'BN':
            if self.cur_bb is not None:    
                dbg_tmp = self.cur_bb
                try:
                    self.cur_bb = self.cur_bb.n_succ_bb
                    self.cur_bb.total_hit += 1
                except AttributeError:
                    print(self.history[-10:])
                    print(f'At {dbg_tmp} atom N received!')
                    exit()
            self.history.append(self.cur_bb)
        elif e == 'O':  # Trace On
            self.stat['trace_on_cnt'] += 1
            self.cur_bb = None
            return 'O'
        elif e == '>': # trace start
            self.cur_bb = None
            pass
        elif e[:2] == 'I:': # interrupt
            self.stat['interrupt_cnt'] += 1
            self.cur_bb = None
            return e
        elif e == 'IR':
            # self.cur_bb = None  # don't do this, since ETM emit the return address, then the IR package
            # self.watch_points_history.pop(-1)
            return 'IR'
        elif e == 'X':
            self.cur_bb = None
            print('OVERFLOW')
            return 'X'
        else:
            print(e)
            assert False

    def world_line_addr(self, limit=-1):
        """ the basic block access chronologically"""
        if self.trace_range:
            start_addr, end_addr = self.trace_range
        start_flag = False
        i = 0
        with open(self.strip_trace, 'r') as f:
            for i, e in enumerate(f):
                if i == limit:
                    break

                if e.strip() == f'A{hex(start_addr)}':
                    start_flag = True

                if start_flag:
                    self.proc(e.strip(), i)
                    if self.cur_bb and self.cur_bb.has_addr(end_addr):
                        print(f'exit at {hex(end_addr)}')
                        break
                #try:
                #    self.proc(e.strip(), i)
                #except AttributeError:
                #    print(f'Attribute Error at line {i}, handling {e}')
                #    exit(0)
        if not start_flag:
            print(f'failed to find entry point {hex(start_addr)}. It might not be in address packet')
        print(f'World line trace done')
        return self.history

    def world_line(self, limit=-1):
        i = 0
        with open(self.strip_trace, 'r') as f:
            for i, e in enumerate(f):
                if i == limit:
                    break
                self.proc(e.strip(), i)

        print(f'World line trace done')
        return self.history

    def world_line_watch(self, limit=-1, debug=False):
        i = 0
        with open(self.strip_trace, 'r') as f:
            for i, e in enumerate(f):
                if i == limit:
                    break
                symbol = self.proc(e.strip(), i)
                if debug and symbol:
                    self.watch_points_history.append(symbol)

                if self.cur_bb:
                    if self.cur_bb in self.watch_points:
                        # print(f'watch_pt #{i} {self.cur_bb}')
                        self.watch_points_history.append((self.cur_bb, i))

        print(f'World line trace done')
        return self.history


    def get_accumulate_history(self):
        acc_history = {}
        for bb in self.cfg.bbs:
            acc_history[bb] = 0
        for bb in self.history:
            if bb is None:
                continue
            acc_history[bb] += 1
        print('accumulated history done')
        return acc_history

    def get_rt_access(self):
        rt_access = {}
        for rt in self.cfg.routines:
            rt_access[rt] = 0
        for bb, n_acc in self.acc_history.items():
            if bb.content[-1].ins == 'bl':
                e_succ_rt = bb.e_succ_bb.rt
                rt_access[e_succ_rt] += 1
        print('rt access done')
        return rt_access

    def bb_visited(self, bb):
        if bb in self.history:
            return True
        return False

    def history_viewer(self):
        pt = 0
        print(f'Total records: {len(self.history)}')
        print(f'cur bb: {self.history[pt]}')
        while(True):
            op = input()
            if op=='n' or op=='':
                pt += 1
            elif op=='p':
                pt -= 1
            elif op=='info':
                print(f'Total {len(self.history)}')
            else:
                pt = int(op)
            
            print(f'pt {pt}: {self.history[pt]}')
            


