from tgraph_util import duplicate, get_entry, visualize
from  msg_binarize import binarize
import networkx as nx

def place_milestone(g, benchmark, blackout_window = 10000, inplace=False):
    if not inplace:
        g = duplicate(g)

    mg = dijkstra_solver(g, blackout_window = blackout_window)
    visualize(g, f'{benchmark}.tmsg.full.dot', label=['cost', 'iters'])
    visualize(g, f'{benchmark}.tmsg.dot', label='full')
    binarize(mg, f'{benchmark}.tmsg')
    return g

def in_cycle(node, cycles):
    """ cycles is the list of simple cycles, i.e. list(nx.alg.simple_cycles) """
    for cycle in cycles:
        if node in cycle:
            return True
    return False

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
    nx.set_node_attributes(g, True, 'is_ms')  # equivalency of coloring all nodes red in paper
    cycles = list(nx.algorithms.simple_cycles(g))
    self_costs = calculate_min_weight(g, cycles)
    entry = get_entry(g)
    red_nodes = list(nx.dfs_preorder_nodes(g, entry))
    while(red_nodes):
        red_n = red_nodes[0]
        if red_n in self_costs:
            if self_costs[red_n] > blackout_window:
                """TODO: the logic is slightly complicated. THink it over tmrw"""
        else:
            red_n = red_nodes.pop(0)
            distances = nx.single_source_dijkstra_path_length(g, red_n, weight = 'cost')
            for n in red_nodes:
                g.nodes[n]['is_ms'] = False if distances[n] <= blackout_window else True
            red_nodes = [node for node in red_nodes if distances[node] > blackout_window]

    """TODO: one peculiar thing is that
    A BB could loop back to itself eventually, 
    thus it has a  well-defined shortest distance to itself.
    the algorithm above would not consider this case, 
    due to dijkstra considering this zero
    thus need to patch it.
    otherwise unsafe milestone would be placed. """

    





