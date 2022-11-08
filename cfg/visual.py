import pygraphviz as pgv

def visualize(g,fname='test.dot', label = 'full'):
    vg = pgv.AGraph(strict=False, directed=True)
    vg.node_attr['style'] = 'filled'
    vg.node_attr['shape'] = 'record'
    for n in list(g.nodes):
        if 'start' in g.nodes[n]:
            address = list(g.nodes[n]['start']) + [n]
            address.sort()
            address = list(map(lambda x: str(hex(x)), address))
            label = '\l'.join(address)
            vg.add_node(n, label=label)
        # texts = []
        # texts.append(n.content_repr())
        # metas = list( f'{key}:{val}' for key, val in g.nodes[n].items())
        # texts = texts + metas
        # texts = '\l'.join(texts)
        # vg.add_node(n, label=texts)
        # if 'visited' in g.nodes[n] and g.nodes[n]['visited']:
        #     vg.get_node(n).attr['color'] = "#def2de"
        # if 'is_dom' in g.nodes[n] and g.nodes[n]['is_dom']:
        #     vg.get_node(n).attr['color'] = "#aaffee"
        # if 'is_transient_mono' in g.nodes[n] and g.nodes[n]['is_transient_mono']:
        #     vg.get_node(n).attr['color'] = "#f79862"
        # if 'is_2gd_transient_mono' in g.nodes[n] and g.nodes[n]['is_2gd_transient_mono']:
        #     vg.get_node(n).attr['color'] = "#d43f24"
        # if 'is_ms' in g.nodes[n] and g.nodes[n]['is_ms']:
        #     vg.get_node(n).attr['color'] = "#ff7e7e"
    for e in list(g.edges):
        # if label == 'full':
        #     text = str(g.get_edge_data(*e))
        # elif isinstance(label, list):
        #     texts = []
        #     for l in label:
        #         texts.append(f'{l}:{str(g.edges[e][l])}')
        #     text = '\l'.join(texts)
        # elif label is None:
        #     text = None
        # else:
        #     text = g.edges[e][label]
        vg.add_edge(*e)
    vg.write(fname)


