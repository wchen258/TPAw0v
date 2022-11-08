import sys
import angr
import networkx as nx


def get_routine_cfg(binary, entry_addr):
    p = angr.Project(binary)
    cfg = p.analyses.CFGFast()
    entry = cfg.get_any_node(entry_addr)
    ng = nx.DiGraph()
    build(entry, cfg, ng)
    return ng

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

if __name__=='__main__':
    import visual
    # 1st : binary path
    # 2nd : starting addr of the routine-cfg
    ng = get_routine_cfg(sys.argv[1], int(sys.argv[2], 16))
    visual.visualize(ng)



