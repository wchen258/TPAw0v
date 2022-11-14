from tgraph_util import eval_iters
import networkx as nx

def flatten_list(input_list):
    res = []
    for l in input_list:
        for item in l:
            res.append(item)
    return res

def get_nodes_between(g, s, e):
    paths = nx.all_simple_paths(g, s,e)
    nodes = flatten_list(paths)
    print(f'get nodes btw {nodes}')
    return set(nodes)


def transient_mono(g, bb) -> bool:
    """ transient_mono is a basic block such that
        it has two out edges, and one points to itself
        furthermore, the selfloop has large iterations
        while the exit edge has only one flow
        thus locally this is a MS 
    """
    if g.out_degree(bb) != 2 or g.in_degree(bb) != 2:
        return False
    
    if ((bb, bb) in g.edges) is False:
        return False
        
    in_edges = list(g.in_edges(bb))
    out_edges = list(g.out_edges(bb))

    u = list(filter(lambda x: x[0] is not bb, in_edges))[0]
    v = list(filter(lambda x: x[1] is not bb, out_edges))[0]

    u_data = g.get_edge_data(*u)
    v_data = g.get_edge_data(*v)
    u_flow = eval_iters(u_data['iters'])
    v_flow = eval_iters(v_data['iters'])
    assert u_flow == v_flow, "flow conservation"

    if u_flow != 1 or v_flow != 1:
        return False

    return True

def transient_mono_2grade(g, bb) -> bool:
    """
    Same as tmono, except that the u,v's flow > 1
    it's similar to 2nd grade MS defined
    I will not handle it this time
    """
    if g.out_degree(bb) != 2 or g.in_degree(bb) != 2:
        return False
    
    if ((bb, bb) in g.edges) is False:
        return False
        
    in_edges = list(g.in_edges(bb))
    out_edges = list(g.out_edges(bb))

    u = list(filter(lambda x: x[0] is not bb, in_edges))[0]
    v = list(filter(lambda x: x[1] is not bb, out_edges))[0]

    u_data = g.get_edge_data(*u)
    v_data = g.get_edge_data(*v)
    u_flow = eval_iters(u_data['iters'])
    v_flow = eval_iters(v_data['iters'])
    assert u_flow == v_flow, "flow conservation"

    #if u_flow != 1 or v_flow != 1:
    if u_flow != 1 and v_flow != 1:
        return True

    assert False


def embed_transient_mono(g, bb, inplace=True):
    if not transient_mono(g, bb):
        return 
    out_edges = list(g.out_edges(bb))
    v = list(filter(lambda x: x[1] is not bb, out_edges))[0]
    v_data = g.get_edge_data(*v)
    s_data = g.get_edge_data(bb,bb)
    s_flow = eval_iters(s_data['iters'])
    v_data['cost'] += s_flow
    g.remove_edge(bb, bb)
    return g

def embed_2gd_transient_mono(g,bb,inplace=True):
    if not transient_mono_2grade(g,bb):
        return
    g.remove_edge(bb, bb)
    return g

def is_valve(g, bb, pred_bb, parent_ms):
    if nx.has_path(g, bb, pred_bb):
        return False
    data = g.get_edge_data(pred_bb, bb)
    n_flow = eval_iters(data['iters'])
    if n_flow != 1:
        return False
    return True

def qualify_valve_ms(g,bb,pred_bb, parent_ms, thresh=1000):
    if not is_valve(g,bb,pred_bb, parent_ms):
        return False
    nodes = get_nodes_between(g,parent_ms, bb)
    subg = g.subgraph(nodes)
    total_sub_flows = 0
    for e in subg.edges:
        d = subg.get_edge_data(*e)
        total_sub_flows += eval_iters(d['iters'])
    if total_sub_flows > thresh:
        return True
    else:
        return False

    
    


    

    

