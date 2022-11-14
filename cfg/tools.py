def derive_end_point(g):
    entries = []
    exits = []
    for node in g.nodes:
        if g.out_degree(node) == 0:
            exits.append(node) 
        if g.in_degree(node) == 0:
            entries.append(node)
    if len(entries) > 1:
        print("CFG WARNING: multiply entries detected")
        [ print(bb) for bb in entries]
    return entries[0], exits