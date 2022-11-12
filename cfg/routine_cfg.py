import sys
import angr
import networkx as nx
from basicblock import BasicBlock


def get_routine_cfg(binary, entry_addr):
    p = angr.Project(binary)
    cfg = p.analyses.CFGFast()
    entry = cfg.get_any_node(entry_addr)
    ng = nx.DiGraph()
    build(entry, cfg, ng)
    return ng

def blockorize(cfg, routine_name):
    # step 1: collect all relevent node
    nodes = []
    for n in cfg.nodes():
        if n.name and n.name.split('+')[0] == routine_name:
            nodes.append(n)
    nodes.sort(key=lambda x: x.addr)

    # step 2: merge node with same end addr
    bins = {}
    for n in nodes:
        try:
            bins[n.instruction_addrs[-1]].append(n)
        except KeyError:
            bins[n.instruction_addrs[-1]] = [n]

    # for bins with many balls:
    # assert all balls have the same successor and jk (jumpkind)
    # below loop is for verification only
    for end_addr,ns in bins.items():
        if len(ns)==1:
            continue
        init_pack = ns[0].successors_and_jumpkinds()
        for i in range(1,len(ns)):
            pack = ns[i].successors_and_jumpkinds()
            if len(init_pack) == 0:
                assert len(pack) == 0
            else:
                _verify_succ_jk(init_pack, pack)

    # step 3: create basic block for each representation
    rt_bbs = []
    for end_addr, ns in bins.items():
        rt_bbs.append(BasicBlock(*ns))
    return rt_bbs

def get_rcfg(rt_bbs):
    ng = nx.DiGraph()
    rt_bbs.sort(key=lambda x: x.start_addr)
    entry = rt_bbs[0]
    return entry

def rcfg_build(node, ng):
    for succ,jk in node.successors_and_jumpkinds(): 
        if jk == 'Ijk_Boring':
            ng.add_edge(node, end_addr(succ))
            add_node_attribute(ng, end_addr(node), 'start', node.addr, set())
            add_node_attribute(ng, end_addr(succ), 'start', succ.addr, set())
            build(succ, cfg, ng)
        elif jk == 'Ijk_Call':
            ret_addr = list(node.instruction_addrs)[-1] + 4  # this is ARM specific by add 4 to return address
            ret_node = cfg.get_any_node(ret_addr)
            ng.add_edge(end_addr(node), end_addr(ret_node))
            add_node_attribute(ng, end_addr(node), 'start', node.addr, set())
            add_node_attribute(ng, end_addr(ret_node), 'start', ret_node.addr, set())
            build(ret_node, cfg, ng)
        else:
            print(ng.nodes)
            raise NotImplemented()






def _verify_succ_jk(p1,p2):
    assert len(p1) == len(p2)
    for t1, t2 in zip(p1,p2):
        assert t1[0] is t2[0]
        assert t1[1] == t2[1]

def add_node_attribute(ng, node, key, val, init):
    try:
        ng.nodes[node][key].add(val)
    except KeyError:
        ng.nodes[node][key] = init
        ng.nodes[node][key].add(val)

def build(node, cfg, ng):
    end_addr = lambda x : list(x.instruction_addrs)[-1]
    for succ,jk in cfg.get_successors_and_jumpkind(node): 
        if jk == 'Ijk_Boring':
            ng.add_edge(end_addr(node), end_addr(succ))
            add_node_attribute(ng, end_addr(node), 'start', node.addr, set())
            add_node_attribute(ng, end_addr(succ), 'start', succ.addr, set())
            build(succ, cfg, ng)
        elif jk == 'Ijk_Call':
            ret_addr = list(node.instruction_addrs)[-1] + 4  # this is ARM specific by add 4 to return address
            ret_node = cfg.get_any_node(ret_addr)
            ng.add_edge(end_addr(node), end_addr(ret_node))
            add_node_attribute(ng, end_addr(node), 'start', node.addr, set())
            add_node_attribute(ng, end_addr(ret_node), 'start', ret_node.addr, set())
            build(ret_node, cfg, ng)
        else:
            print(ng.nodes)
            raise NotImplemented()




