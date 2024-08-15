import networkx as nx
import pygraphviz as pgv
from as_cf_utils import *
from colorama import Fore
from colorama import Style

def to_graph(self, bbs):
    """ Take linked bbs, assume entry is the first bb, and end with ret """
    ng = nx.DiGraph()
    if len(bbs) == 1:
        end_mnemonic = bbs[0].content[-1].ins
        if end_mnemonic == 'ret' or end_mnemonic == 'br':
            ng.add_node(bbs[0])
            return ng
        else:
            print(f'{Fore.YELLOW}Special ending routine {bbs[0]}, forming a none-return routine.{Style.RESET_ALL}')
            ng.add_node(bbs[0])
    for b in bbs:
        end_mnemonic = b.content[-1].ins
        if end_mnemonic == 'bl' or end_mnemonic == 'blr':
            try:
                ng.add_edge(b, b.out_tunnel)
            except ValueError:
                print(f'{b.content[-1].raw_line.strip}')
        elif end_mnemonic in CB_INS:
            ng.add_edge(b, b.e_succ_bb)    
            ng.add_edge(b, b.n_succ_bb)
        elif end_mnemonic in UB_INS:
            ng.add_edge(b, b.e_succ_bb)
        elif end_mnemonic == 'ret' or end_mnemonic == 'br' or end_mnemonic == 'blr':
            continue
        else:
            print(end_mnemonic)
            assert False
    return ng

def nxg2pgv(self, ng):
    """ Take a nx DiGraph and convert to pgv """
    g = pgv.AGraph(strict=False, directed=True)
    g.node_attr['style'] = 'filled'
    g.node_attr['shape'] = 'record'
    for n in list(ng.nodes):
       g.add_node(n, label=n.content_repr())
    for e in list(ng.edges):
        g.add_edge(*e)
    return g

def nxg2pgv_msg(self, ng):
    """ Take a nx DiGraph and convert to pgv """
    g = pgv.AGraph(strict=False, directed=True)
    g.node_attr['style'] = 'filled'
    g.node_attr['shape'] = 'record'
    for n in list(ng.nodes):
        g.add_node(n, label=n.content_repr())
    for e in list(ng.edges):
        g.add_edge(*e)
    return g

def nxg2pgv_flow(self, ng, label=None):
    """ Take a nx DiGraph and convert to pgv """
    g = pgv.AGraph(strict=False, directed=True)
    g.node_attr['style'] = 'filled'
    g.node_attr['shape'] = 'record'
    for n in list(ng.nodes):
        g.add_node(n, label=n.content_repr())
    for e in list(ng.edges):
        if label == 'full':
            text = str(ng.edges[e])
        else:
            text = ng.edges[e][label]
        g.add_edge(*e, label=text)
    return g


def visual_pgv(self, rt_stat, fname):
    rt_name = rt_stat['routine']
    vg = rt_stat['pgv']
    vg.write(f'{fname}/{rt_name}.dot')