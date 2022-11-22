import networkx as nx
import sys

from tgraph_util import *
from milestone_trace_graph import init_nxg, proc_bb, preprocess_tcfg, post_msg_dec
from msg_binarize import binarize

"""
Before running deploy, first run decorator.py, so that g has all trace history
decorate_flow would calculate the cost


Do as following
1. generate the top level graph and watchpts. 
2. decorate_flow
3. find the edge whose cost is proportionally heavy (must be some routine)

for those in 3:
    repoeat 1,2,3, until 

No nononononoon do the following

unproc_rts = [top_level-rt]
rtgs = []

while(unproc_rts is not empty):
    rt = rts.pop
    cfg, g, watchpt = initialize
    g = accept_all_strip
    decorate_flow(g)
    rtg.append(g)
    if cost > thresh:
        rts.push(associate rt) 

for g in rtgs:

"""

msg = nx.DiGraph()

def pipe(g, msg):
    ng = duplicate(g) # g is nx.digraph, msg is output to be added
    #ng = decorate_flow(ng, ['getDisparity.def1.strip', 'getDisparity.deg1.strip'])
    ng = decorate_flow(ng, ['mser.strip'])
    visualize(ng, f'mser.iters.dot', label = 'iters')

    ng = init_nxg(ng)
    entry = get_entry(ng)
    proc_bb(ng, msg, entry)
    #binarize(msg, 'getDisparity.tmsg')
    binarize(msg, 'mser.tmsg')
    return ng

def pipe2(g):
    g = full2cost(g)
    remove_null(g)
    embed_dom(g)
    visual_cost(g)

def gen_msg(g, msg, benchmark,inplace=False):
    if not inplace:
        g = duplicate(g)
    preprocess_tcfg(g)
    
    g = init_nxg(g)
    entry = get_entry(g)
    proc_bb(g, msg, entry)

    if benchmark == 'mser':
        post_msg_dec(msg)

    visualize(g, f'{benchmark}.tmsg.full.dot', label=['cost', 'iters'])
    visualize(msg, f'{benchmark}.tmsg.dot', label='full')
    binarize(msg, f'{benchmark}.tmsg')
    return g

"""Right now proc_bb is the primitive algorithm to place MS
According to the paper, we can use nx.single_source_dijkstra_length 
to calculate the distance from a source to all other node 
and continue the placement
TODO: implement this paper approach"""
    










