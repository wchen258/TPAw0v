import os
import traceback
import argparse
import networkx as nx
import pygraphviz as pgv

from colorama import Fore
from colorama import Style
from matplotlib import pyplot as plt
from as_cf_utils import BR_INS, UB_INS, CB_INS, BL_INS, RET_INS, ALL_BRANCH_INS, DUMMY_INS
from as_cf_utils import read_as, parse_section
from as_cf_utils import inside
from basic_block import Lean_BB
from tracer import Tracer

class CFGLean:

    def __init__(self, fname, section='.text') -> None:
        """ routine refers to a function or subroutine
            routine.content can access individual line assembly code
            each assembly is represented by INS object
        """
        
        print(f'CFGLean initialized')
        print(f'Input  : {fname}')
        print(f'section: {section}')

        self.section = read_as(fname, section=section) 
        self.routines = parse_section(self.section)
        self.addr_continuity_check()  ## some binary has holes, e.g. discontinuity in the objdump file address, this function would raise a warning, in practice, this is not a problem.
        self.bbs = self.blockorize()  ## blockorize generates basic blocks  
        self.plain_link() ## upon blockorization, the branching info is missing. plain_link would link blocks together based on their branching info. This would however, not link `ret` nor `br` nor `blr` as these are not infereable statically
        self.tunnel_link() ## create a link from a `bl` basic block to the basic block at (address + 0x4). Which means each bl BB has two out edge. One for the true branch address, i.e. the address where bl branch to. One for the logical return address, i.e. addressOf(bl) + 0x4
        self.natural_link()
        self.oedge_ct_renew()

    from _cfg_solver import solve_rt, gather_rt_stat, get_milestones, get_sub_milestones, get_imm_milestone, produce_chain
    from _cfg_graph import to_graph, nxg2pgv, nxg2pgv_msg, visual_pgv
    from _cfg_rt_eval import rt_eval, gather_rts, attach_weight, special_rt, calc_smallest_weight, info_weight_path

    #####################
    # utils (invariance)#
    #####################

    def all_asm_iter(self):
        for r in self.routines:
            for i in r.content:
                yield i

    def find_bb(self, addr):
        low = 0
        high = len(self.bbs) - 1
        while (low <= high):
            mid = (low + high) >> 1
            if (self.bbs[mid].has_addr(addr)):
                return self.bbs[mid]
            elif (addr < self.bbs[mid].content[0].addr):
                high = mid - 1
            else:
                low = mid + 1
        return None

    def find_rt_bb(self, r_name):
        rt = self.find_routine(r_name)
        return list(bb for bb in self.bbs if bb.rt is rt)

    def find_routine(self, r_name):
        if type(r_name) is not str:
            r_name = r_name.name_strip
        for r in self.routines:
            if r.name == '<'+r_name+'>':
                return r
        assert False, f'{r_name} failed to be found in CFG'
    
    def addr_continuity_check(self):
        cur_end = self.routines[0].content[-1].addr
        for r in self.routines[1:]:
            try:
                assert cur_end + 4 == r.content[0].addr, "assembly has holes!"
            except AssertionError:
                print(f'WARNING: assembly has holes {hex(cur_end)}, be cautious if this is in user define routine')
            cur_end = r.content[-1].addr

        
    #####################
    #   initialization  #
    #####################

    def blockorize(self):
        just_close = True
        bbs, content = [], []
        for inst in self.all_asm_iter():
            if just_close and inst.ins in DUMMY_INS:
                continue
            just_close = False
            content.append(inst)
            if inst.ins in ALL_BRANCH_INS:
                bbs.append(Lean_BB(content))
                content = []
                just_close = True
        return bbs

    ####################################
    #   link methods (part of init)    #
    ####################################

    def plain_link(self):
        """ Link addr explicit BB branch, this excludes ret and br """
        for bb in self.bbs:
            if bb.content[-1].esuccessor_addr:
                e_bb = self.find_bb(bb.content[-1].esuccessor_addr)
                assert e_bb is not None, "plain link e_bb"
                bb.update_link_e_to(e_bb)
                if bb.content[-1].is_cb:
                    n_bb = self.find_bb(bb.content[-1].addr + 4)
                    assert n_bb is not None, "plain link check"
                    bb.update_link_n_to(n_bb)

    def tunnel_link(self):
        """ Create a tunnel between bl and returning bb"""
        for bb in self.bbs:
            if bb.content[-1].is_link:
                ret_bb = self.find_bb(bb.content[-1].addr + 4)
                if ret_bb is None:
                    offend_asm = bb.content[-1].raw_line.strip()
                    print(f'{Fore.YELLOW}{offend_asm} has no return BB.')
                    print(f'Ignore this message if it\' a routine call to <abort@plt>{Style.RESET_ALL}')
                    continue
                bb.update_link_ret_to(ret_bb)

    def natural_link(self):
        for bb in self.bbs:
            bb.update_natural_succ()

    def oedge_ct_renew(self):
        for bb in self.bbs:
            bb.update_n_oedge()


        
if __name__ == '__main__':
    import os

    parser = argparse.ArgumentParser(description='Analysis an objdump file')
    parser.add_argument('fname', type=str)
    parser.add_argument('--graph', action='store_true')
    parser.add_argument('-caller', type=str)
    args = parser.parse_args()

    fpath = f'../demo/application/{args.fname}.dp'
    if not os.path.exists(fpath):
        print(f'Require {args.fname}.dp presenting in the demo/application directory.')
        print(f'The .dp files have to be generated by specific objdump. See README')
        exit()

    cfg = CFGLean(fpath, section='all')

    if(args.graph):
        os.makedirs(args.fname, exist_ok=True)
        for rt in cfg.routines:
            rt_name = rt.name[1:-1]
            rt_stat = cfg.solve_rt(rt_name)
            cfg.visual_pgv(rt_stat, args.fname)
        print(f'Output visualization in {args.fname}')

    if(args.caller):
        l = []
        for bb in cfg.bbs:
            for asm in bb.content:
                if(asm.ins=='bl' and asm.to_rt_name == f'<{args.caller}>'):
                    l.append(asm.addr)
        l.sort()
        l = list(hex(addr) for addr in l)
        l = ','.join(l)
        with open('ms.txt', 'w') as f:
            f.write(l)
        



    
