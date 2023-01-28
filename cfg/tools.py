def derive_end_point(g):
    exits = []
    for node in g.nodes:
        if g.out_degree(node) == 0:
            exits.append(node) 

    try:
        entry = min(list(g.nodes), key=lambda x:x.content[0].addr)
    except AttributeError:
        entry = min(list(g.nodes))
    return entry, exits


