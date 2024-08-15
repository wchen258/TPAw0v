# This file provides a method to construct a milestone graph manually
# Combined with the visualization of the trace data on applications, this can be done meaningfully
# as longs as the number of unfolding function is not large

import networkx as nx

def draw_tracking():
    g = nx.DiGraph()
    spine = [0x4009a8, 0x4009b0, 0x4009d4, 0x4009dc, 0x4009e8, 0x4009f4, 0x400a0c, 0x400a24, 0x400a34, 0x400a3c, 0x400a48]

    for i in range(len(spine)):
        if i==0:
            pass
        else:
            g.add_edge(spine[i-1],spine[i])

    g.add_edge(spine[-1], 0x400b24)
    g.add_edge(0x400b24, 0x400b74)
    g.add_edge(0x400b74, 0x400c6c)
    # g.add_edge(0x400c6c, 0x400c74)
    g.add_edge(0x400c6c, 0x400d58)

    # g.add_edge(0x400c74, 0x400d44)

    # g.add_edge(0x400c74, 0x400d58)
    g.add_edge(0x400d58, 0x400b74)
    g.add_edge(0x400d58, 0x400e1c) 

    return g


def draw_texture():
    g = nx.DiGraph()
    spine = [0x405010, 0x4054e4, 0x4051d8, 0x4052a8, 0x40541c]
    for i in range(len(spine)):
        if i==0:
            pass
        else:
            g.add_edge(spine[i-1],spine[i])

    # loops
    sloops = [0x4051d8, 0x4052a8]
    for addr in sloops:
        g.add_edge(addr, addr)
    return g


def draw_precision():
    g = nx.DiGraph()
    #g.add_edge(0x400994, 0x400910)
    #g.add_edge(0x400910, 0x400910)
    #g.add_edge(0x400910, 0x400c1c)

    g.add_edge(0x400854, 0x4007d0)
    g.add_edge(0x4007d0, 0x4007d0)
    g.add_edge(0x4007d0, 0x4009f0)

    return g

def draw_response():
    g = nx.DiGraph()
    g.add_edge(0x402d2c, 0x402e58)
    g.add_edge(0x402e58, 0x402e58)
    g.add_edge(0x402e58, 0x402f00)
    return g
    
