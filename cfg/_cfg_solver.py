from tools import derive_end_point
from as_cf_utils import *
from colorama import Fore, Style
import networkx as nx

def solve_rt(self, rt_name):
    """ Produce a dictionary containing all info for given routine, including:
        plot graph, networkx graph, immediate dominator path
        entry point, exit points, simple cycles
        hubs (a hub is a logical node for a outmost largest cycle in a routine
    """
    d_sc = self.gather_rt_stat(rt_name)
    g = d_sc['nx']
    entry, exits = derive_end_point(g)
    d_sc['idom'] = nx.algorithms.immediate_dominators(g, entry)
    d_sc['entry'] = entry
    d_sc['exits'] = exits

    # due to blockorization some garbage filler from prev rt are padded into the current one, remove such
    for i,val in enumerate(entry.content):
        if val.rt.name_strip == rt_name:
            break
        print(f'{Fore.YELLOW} Offend Border {rt_name} {i}, {val} removed. {Style.RESET_ALL}')
    entry.content = entry.content[i:]
    return d_sc


def gather_rt_stat(self, rt_name):
    rt_bbs = self.find_rt_bb(rt_name)
    ng = self.to_graph(rt_bbs)
    vg = self.nxg2pgv(ng)
    scg = nx.simple_cycles(ng)
    simple_cycles = list( list(bb for bb in sc) for sc in scg)
    return {'routine':rt_name, 'pgv': vg, 'nx': ng, 'sc':simple_cycles}

def get_milestones(self, rt_name):
    rt_info = self.solve_rt(rt_name)
    collection = {}
    for exit in rt_info['exits']:
        collection[exit] = self.produce_chain(rt_info['idom'], rt_info['entry'], exit, rt_info['sc'])
    return collection
def get_sub_milestones(self, d_sc, entry):
    collection = {}
    idom = nx.algorithms.immediate_dominators(d_sc['nx'], entry)
    keys = list(idom.keys())
    true_exits = list( exit for exit in d_sc['exits'] if exit in keys)
    for exit in true_exits:
        collection[exit] = self.produce_chain(idom, entry, exit, d_sc['sc'])
    return collection
def get_imm_milestone(self, d_sc, entry):
    """ This has vulnerability it's untrue that the get_imm_milestone is unique. A routine might have two returns, and thus a diverge"""
    collection = self.get_sub_milestones(d_sc, entry)
    if len(collection) > 1 :
        print('Two immediate milestone candidates. Only return one')
        for k, v in collection.items():
            print(k, v)
    for exit,chain in collection.items():
        if len(chain) == 1:
            print(f'Reaching the ending BB of routine. No front dominator anymore')
            return 
        for i, bb in enumerate(chain):
            if bb is entry:
                return chain[i+1]
def produce_chain(self, idom, entry, exit, cycles=None):
    def remove_cc(chain, cycles):
        for cycle in cycles:
            for bb in cycle:
                if bb in chain:
                    chain.remove(bb)
                
    def find_idominator(idom, bb):
        for k,v in idom.items():
            if k is bb:
                return v
        assert False, "Every bb must have one and only one immediate dominator!"
    def core2(idom, cur_bb):
        if cur_bb is entry:
            return [entry]
        else:
            v = find_idominator(idom, cur_bb)
            return [cur_bb] + core2(idom, v)
    
    solution = core2(idom, exit)
    solution.reverse()
    if cycles:
        remove_cc(solution, cycles)
    return solution
