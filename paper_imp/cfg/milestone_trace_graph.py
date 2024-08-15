from as_cf_utils import CB_INS, UB_INS,BL_INS, RET_INS
from msg_binarize import binarize
# from tgraph_util import duplicate, decorate_flow, report_cost, visualize
from tgraph_util import *
import oracle_council as council
import sys
import lean_cfg
import as_cf_utils
import networkx as nx
import pickle
import os
import argparse

inst_thresh = 10000
valve_thresh = 10000
trace = True

# corner for mser
def corner(benchmark):
    if benchmark == 'mser':
        picks = [0x404ccc, 0x404994] 
        self_loops = [0x404ccc]
        force_link = [(0x404ccc, 0x404d5c)]
    elif benchmark == 'sift':
        picks = []
        self_loops = []
        force_link = []
    else:
        picks = []
        self_loops = []
        force_link = []


    return picks, self_loops, force_link


#picks, self_loops, force_link = corner(sys.argv[1])

def tprint(s):
    print(s) if trace else None

def get_parser():
    parser = argparse.ArgumentParser(description="input assembly")
    parser.add_argument('fname')
    parser.add_argument('-rt', default='main')
    args = parser.parse_args()
    return args

def preprocess_tcfg(g, inplace=True):
    if not inplace:
        g = duplicate(g)
    nx.set_node_attributes(g, False, 'is_transient_mono')
    nx.set_node_attributes(g, False, 'is_2gd_transient_mono')
    for n in g.nodes:
        if council.transient_mono(g,n):
            g.nodes[n]['is_transient_mono'] = True
            council.embed_transient_mono(g, n)
        elif council.transient_mono_2grade(g,n):
            g.nodes[n]['is_2gd_transient_mono'] = True
            council.embed_2gd_transient_mono(g,n)
        
    visualize(g, f'mser.prep.dot', label=['cost', 'iters'])
    return g

def init_nxg(nxg):
    # call this before proc_bb, otherwise expect KeyError
    nx.set_node_attributes(nxg, False, 'visited')
    nx.set_node_attributes(nxg, False, 'is_ms')
    return nxg

def proc_bb(nxg: nx.DiGraph, msg, bb, pred_bb=None, parent_node=None, acc_ct=0, rt_stack=[], hint=None):
    """
    nxg: is the control flow graph with consolidated information from all input samples
         information such as number of hit for a certain bb, 
         the transition cost from one bb to another are all available to use
    msg: this is the ouput - a milestone graph
    pred_bb: due to the method is recursive, pred_bb is the caller's argument for bb.
    parent_node: the latest placed milestone (also a bb)
    acc_ct: the number of branch instruction taken so far from parent_node to bb
    rt_stack: all the function(routine) calls from parent_node to bb  
    """


    # it's possible to use other return condition to allow the algo to explore things further
    tprint(f'\nCurrent bb: {bb}')
    tprint(f'pred_bb: {pred_bb}')
    tprint(f'Parent MS: {parent_node}')

    if pred_bb:
        if nxg.edges[pred_bb, bb]['cost'] == 0: # this edge never taken in samples
            return 
        acc_ct += nxg.edges[pred_bb, bb]['cost']

    proceed = historian(nxg, msg, bb, pred_bb,  parent_node, acc_ct, rt_stack, hint)
    if not proceed:
        return

    decision, rt_stack = oracle(nxg,msg, bb, pred_bb, parent_node, acc_ct, rt_stack, hint)

    nxg.nodes[bb]['visited'] = True

    tprint(f'oracle decision: {decision}')
    if decision is True:
        acc_ct = 0
        next_stack = [bb.e_succ_bb.rt] if bb.end_ins.ins == 'bl' else []
        
        if parent_node:
            tprint(f'Add edge {parent_node} -> {bb}')
            msg.add_edge(parent_node, bb)
        else:
            tprint(f'Add Entry as root of MSG')
            msg.add_node(bb)

        parent_node = bb
        nxg.nodes[bb]['is_ms'] = True
        # bb.is_ms = True

    else:
        if bb.end_ins.ins == 'bl':
            next_stack = rt_stack + [bb.e_succ_bb.rt]
        else:
            next_stack = rt_stack

    tprint(f'num of out edge: {bb.n_oedge}')
    # if bb.n_oedge == 1:
    if nxg.out_degree(bb) == 1:
        ## add some hints
        succs = list(nxg.successors(bb))
        assert nxg.edges[bb, succs[0]]['cost'] != 0, "if up stream has flow, this has to have flow"
        hint = 'succ of ub bb' if bb.content[-1].ins == 'b' else None
        proc_bb(nxg, msg, succs[0], bb, parent_node, acc_ct, next_stack, hint)

    # elif bb.n_oedge == 2:
    elif nxg.out_degree(bb) == 2:
        hint = 'succ of cb bb'
        succs = list(nxg.successors(bb))
        proc_bb(nxg, msg, succs[0], bb, parent_node, acc_ct, next_stack, hint)
        tprint(f'\n*** call split at {bb} ***\n')
        proc_bb(nxg, msg, succs[1], bb, parent_node, acc_ct, next_stack, hint)
    
    else: # even if it's exit, if does not sustain, then just ignore
        # the bb is ret
        # if not bb.is_ms:
            # msg.add_edge(parent_node, bb)
            # bb.is_ms = True
            # bb.ms_description = 'exit'
        return

def oracle(nxg, msg,bb, pred_bb, parent_node, acc_ct, rt_stack, hint):

    # legacy start
    def proc_hint():
        if hint == 'succ of cb bb':
            # by design, if b.cond BB is a MS, then two children cannot be MS, since the gap is too small to be meaningful  
            bb.ms_description = hint
            return False, rt_stack

        if hint == 'succ of ub bb':
            bb.ms_description = hint
            return False, rt_stack

    if hint:
        # return proc_hint()
        pass

    # legacy end

    # if bb.n_oedge == 2:
        # tprint('oracle: two outedge')
        # bb.ms_description = 'Conditional Branch'
        # return True, rt_stack

    if parent_node is None:
        # this is an entry node, thus make it a milestone
        tprint("oracle: entry node")
        bb.ms_description = 'Entry'
        return True, rt_stack

    # if bb.content[0].addr in picks:
    #     bb.ms_description = 'Manual Pick'
    #     return True, rt_stack

    # if bb.content[0].addr in picks:
        # bb.ms_description = 'picked block'
        # return True, rt_stack

    if acc_ct > inst_thresh:
        bb.ms_description = f'{acc_ct} has exceeded thresh {inst_thresh}'
        tprint("orcale: thresh met")
        return True, rt_stack

    if council.is_valve(nxg, bb, pred_bb, parent_node):
        if council.qualify_valve_ms(nxg, bb, pred_bb, parent_node, thresh=valve_thresh):
            bb.ms_description = f'a valid valve'
            return True, rt_stack

    return False, rt_stack

    # if len(rt_stack) == 1:
        # if rt_stack[0].name_strip in short_rt_preset:
            # return False, [] 
# 
    # if len(rt_stack) > 0:
        # desc = ' '.join(list(rt.name[1:-1] for rt in rt_stack))
        # bb.ms_description = f'{desc} called uptill here'
        # tprint("oracle: function call in between")
        # return True, rt_stack
    # 
    # return False, rt_stack

def historian(nxg, msg,bb, pred_bb, parent_node, acc_ct, rt_stack,  hint):
    """ Due to DFS search, some bb are already visited, 
        but milestone connection might still need to be established
        fix abnormality, and return a signal whether continue traversing
    """
    def handle_loop(path):
        head = path.pop(0)
        path.append(head)
        for bb in path:
            # if bb.is_ms:
            if nxg.nodes[bb]['is_ms']:
                msg.add_edge(parent_node, bb)
                return False
        return False

    def handle_dfs(bb):
        # while True:
        #     o_nodes = list( n for n in nxg.successors(bb))
        #     if len(o_nodes) > 1:
        #         assert False, "not likely dfs get a cb and no milestone placed"
        #     # if o_nodes[0].is_ms:
        #     if nxg.nodes[o_nodes[0]]['is_ms']:
        #         msg.add_edge(parent_node, o_nodes[0], label='asdfasdfasdf')
        #         return False
        #     else:
        #         bb = o_nodes[0]

        while True:
            o_nodes = list( n for n in nxg.successors(bb))
            print(f'handle_dfs out_nodes: {o_nodes}')
            if len(o_nodes) > 1:
                visualize(nxg, fname='mser.tmsg.dot', label=['cost', 'iters'])
                assert False, "not likely dfs get a cb and no milestone placed"
            # if o_nodes[0].is_ms:
            if nxg.nodes[o_nodes[0]]['is_ms']:
                msg.add_edge(parent_node, o_nodes[0], label='added by historian handle_dfs')
                return False
            else:
                bb = o_nodes[0]
        
    # if bb.is_ms:
    if nxg.nodes[bb]['is_ms']:
        msg.add_edge(parent_node, bb)
        return False
    # if bb.visited:
    if nxg.nodes[bb]['visited']:
        paths = get_path(nxg, bb, pred_bb)
        #print(paths)
        if len(paths) == 1:
            return handle_loop(paths[0])
        elif len(paths) == 0:
            return handle_dfs(bb)
        else:
            # this is a premature termination, this is far from ideal, 
            # basically means ignore/give up when the loop are complicated
            print(f'Unhandled handle_dfs bb due to complex loop: {bb}, pred: {pred_bb}')
            #visualize(nxg, f'mser.tmsg.dot', label=['cost', 'iters'])
            #raise NotImplemented("Not yet implemented, more than one path from bb to pred_bb")
            return False
    return True

def get_path(nxg, source,target):
    path_gen = nx.all_simple_paths(nxg, source, target)
    paths = list(p for p in path_gen)
    return paths

def get_bb_addr(msg, addr):
    for bb in msg.nodes:
        if bb.has_addr(addr):
            return bb
    assert False


def post_msg_dec(msg):
    for addr in self_loops:
        assert addr in self_loops
        for n in msg.nodes:
            if n.has_addr(addr):
                msg.add_edge(n,n)

    for u,v in force_link:
        ubb = get_bb_addr(msg, u)
        vbb = get_bb_addr(msg, v)
        msg.add_edge(ubb,vbb)
        
    
def gen_msg(g, msg, benchmark, rt=None,inplace=False):
    if not inplace:
        g = duplicate(g)
    preprocess_tcfg(g)
    
    g = init_nxg(g)
    entry = get_entry(g)
    proc_bb(g, msg, entry)

    if benchmark == 'mser':
        post_msg_dec(msg)

    if rt is None:
        rt = benchmark
    visualize(g, f'{benchmark}.{rt}.tmsg.full.dot', label=['cost', 'iters'])
    visualize(msg, f'{benchmark}.{rt}.tmsg.dot', label='full')
    binarize(msg, f'{benchmark}.{rt}.tmsg')
    return g
    

if __name__ == '__main__':
    msg = nx.DiGraph()
    

    
