import networkx as nx
import sys

def rt_eval(self, rt):
    d = self.solve_rt(rt)
    g = d['nx']
    sub_rts = self.gather_rts(rt)
    for sub_rt in sub_rts:
        if sub_rt.weight is None:
            if not self.special_rt(sub_rt):
                sub_rt.weight, _, _ = self.rt_eval(sub_rt)
    self.attach_weight(g)
    total, ps = self.calc_smallest_weight(g, d)
    return total, ps, g

def gather_rts(self, rt):
    bbs = self.find_rt_bb(rt)
    sub_rts = set(list(bb.e_succ_bb.rt for bb in bbs if bb.end_ins.ins == 'bl'))
    return sub_rts

def attach_weight(self, g):
    for node in g.nodes:
        g.nodes[node]['weight'] = node.inst_cnt
        if node.end_ins.ins == 'bl':
            tar_rt = node.e_succ_bb.rt
            g.nodes[node]['weight'] += tar_rt.weight

    for node in g.nodes:
        for e in g.out_edges(node):
            g.edges[e]['weight'] = g.nodes[node]['weight']

def special_rt(self, rt):
    special_calls = ['fFreeHandle', 'iFreeHandle', 'free@plt']
    rt_name = rt.name_strip
    if rt_name in special_calls:
        rt.weight = 1 # here user can add dlib call eval
        return True
    else:
        return False 

def calc_smallest_weight(self, g, d):
    # TODO: resuem here the entry and exit should not be by some instruction, rather, by number of in-out degree
    entry = d['entry']
    res = sys.maxsize
    res_path = None
    if len(d['exits']) < 1:
        print('no exit occurss.....')
        [print(n) for n in g.nodes]
        assert False, "cannot find exit"
    for exit in d['exits']:
        try:
            ps = nx.shortest_path(g, entry, exit, weight = 'weight')
        except nx.exception.NetworkXNoPath:
            for n in g.nodes:
                print(n)
            assert False
        w = sum(list(g.nodes[n]['weight'] for n in ps))
        if res > w:
            res = w
            res_path = ps 
    return res, ps

##################
#   info tools   #
##################

def info_weight_path(self, w, ps, g):
    print(f'Total weight: {w}')
    print(f'Path:')
    for node in ps:
        sup = node.e_succ_bb.rt.name_strip if node.end_ins.ins=='bl' else ''
        print(f"\tWeight: {g.nodes[node]['weight']:7}     {node} " + sup)



