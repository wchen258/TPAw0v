from tgraph_util import duplicate, get_entry, visualize
from  msg_binarize import binarize
import networkx as nx

def place_milestone(g, benchmark, blackout_window = 10000, inplace=False):
    if not inplace:
        g = duplicate(g)

    mg = dijkstra_solver(g, blackout_window = blackout_window)
    visualize(g, f'output/{benchmark}.tmsg.full.dot', label=['cost', 'iters'])
    visualize(g, f'output/{benchmark}.tmsg.dot', label='full')
    binarize(mg, f'output/{benchmark}.tmsg')
    return g

def calculate_min_weight(g, cycles, weight='cost'):
    min_self_cost = {}
    for cycle in cycles:
        cost = 0
        for i, _ in enumerate(cycle):
            cost += g.edges[cycle[i], cycle[(i+1)%len(cycle)]][weight]
        for n in cycle:
            try:
                min_self_cost[n] = cost if cost < min_self_cost[n] else min_self_cost[n]
            except KeyError:
                min_self_cost[n] = cost
    return min_self_cost

def dijkstra_solver(g, blackout_window=10000):
    # set all nodes to red
    nx.set_node_attributes(g, True, 'is_ms')  

    # identify all cycles in the graph
    cycles = list(nx.algorithms.simple_cycles(g))

    # calculate the cost for each loop. The cost is defined to be the following:
    # pick any node in the loop, for that node, let PC travel from this node, until it loops back
    # the total cost is the cost for that loop 
    self_costs = calculate_min_weight(g, cycles)

    # traverse the graph
    entry = get_entry(g)
    red_nodes = list(nx.dfs_preorder_nodes(g, entry))

    # if the cost of a loop is smaller than the blackout window
    # color every node in the loop white
    for n in red_nodes:
        if n in self_costs and self_costs[n] <= blackout_window:
            g.nodes[n]['is_ms'] = False
    red_nodes = [n for n in red_nodes if g.nodes[n]['is_ms']]

    # the following are by and large follow the paper
    while(red_nodes):
        red_n = red_nodes.pop(0)
        distances = nx.single_source_dijkstra_path_length(g, red_n, weight = 'cost')
        for n in red_nodes:
            if n in distances and distances[n] <= blackout_window:
                g.nodes[n]['is_ms'] = False
        red_nodes = [n for n in red_nodes if g.nodes[n]['is_ms']]

    return g






