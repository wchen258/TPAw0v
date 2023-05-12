import lean_cfg, tracer
import networkx as nx
import setting
import sys
import argparse
from tgraph_util import *
import heuristic
# from deploy import pipe2


def get_parser():
    parser =  argparse.ArgumentParser()
    parser.add_argument('binary')
    parser.add_argument('routine')
    args = parser.parse_args()
    return args

def initialize(tar_bin, tar_rt, tar_strips, order=False):
    cfg = lean_cfg.CFGLean(f'../demo/application/{tar_bin}', section='all')
    #watch_points = cfg.find_rt_bb(tar_rt)
    d = cfg.solve_rt(tar_rt)
    g = d['nx']
    watch_points = list(g.nodes)
    print(f'watch_points length : {len(watch_points)}')

    if not order:
        for e in g.edges:
            for strip in tar_strips:
                g.edges[e][strip] = {}
                g.edges[e][strip]['flows'] = 0

    return cfg, g, watch_points

def accept_strip_flow(g, cfg, watch_points, tar_strip, order = False):
    if order:
        for up, down, data in g.edges(data=True):
            data[f'{tar_strip}'] = {}
    trc = tracer.Tracer(cfg, f'../demo/trace_data/{tar_strip}', limit=-1,watch_points = watch_points)
    flows = trc.watch_points_history
    return flows

def accept_strip_embed(g, watch_points, tar_strip, flows):
    wflow_ct = 0
    for i in range(1,len(flows)-1):
        transfer = (flows[i-1][0], flows[i][0])
        elements = flows[i][1] - flows[i-1][1]
        try:
            if 'flows' in g.edges[transfer][tar_strip]:
                g.edges[transfer][f'{tar_strip}']['flows'] += 1
            else:
                g.edges[transfer][f'{tar_strip}'][str(i)] = elements
        except KeyError:
            wflow_ct += 1

    print(f'# unrealizable branches: {wflow_ct}')
    print(f'This is due to trace data analysis. The imprecision should be negligible')
    return g

def embed_flow(g, strip, flows):
    wflow_ct = 0
    
    for e in g.edges:
        g.edges[e][strip]['atoms'] = []

    for i in range(1,len(flows)-1):
        transfer = (flows[i-1][0], flows[i][0])
        elements = flows[i][1] - flows[i-1][1]
        try:
            if 'flows' in g.edges[transfer][strip]:
                g.edges[transfer][strip]['flows'] += 1
                g.edges[transfer][strip]['atoms'].append(elements)
            else:
                g.edges[transfer][strip][str(i)] = elements
        except KeyError:
            wflow_ct += 1

    for e in g.edges:
        if g.get_edge_data(*e)[strip]['flows'] != 0:
            atoms = g.edges[e][strip]['atoms']
            atom_max, atom_min, atom_mean, atom_std = max(atoms), min(atoms), np.mean(atoms), np.std(atoms)
            g.edges[e][strip]['atom_max'] = atom_max
            g.edges[e][strip]['atom_min'] = atom_min
            g.edges[e][strip]['atom_mean'] = atom_mean
            g.edges[e][strip]['atom_std'] = atom_std
        else:
            g.edges[e][strip]['atom_max'] = None
            g.edges[e][strip]['atom_min'] = None
            g.edges[e][strip]['atom_mean'] =  None
            g.edges[e][strip]['atom_std'] = None
            
        del g.edges[e][strip]['atoms']

    print(f'# unrealizable branches: {wflow_ct}')
    print(f'This is due to trace data analysis. The imprecision should be negligible')
    return g


def accept_strips(tar_bin, tar_rt, tar_strips):
    cfg, g, watch_points = initialize(tar_bin, tar_rt, tar_strips, order=False)
    for strip in tar_strips:
        print(f'accepting {strip}...')
        flows = accept_strip_flow(g, cfg, watch_points, strip)
        g = embed_flow(g, strip, flows)
    return g

def post_accept_decorate(g, tar_strips, order=False):
    decorate_flow(g, tar_strips, order=False)
    remove_null(g)
    #embed_dom(g)
    return g

if __name__ == '__main__':
    args = get_parser()
    tar_bin, tar_rt, tar_strips = setting.populate(args.binary)
    g = accept_strips(tar_bin, tar_rt, tar_strips)
    g = post_accept_decorate(g, tar_strips, order=False)
    heuristic.apply_heuristic_1(g)
