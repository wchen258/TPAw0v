from tools import derive_end_point
import struct
import networkx as nx
import pygraphviz as pgv

def binarize(msg, routine_name=None):
    ms_entry, _ = derive_end_point(msg)
    br = bytearray()
    nx.set_node_attributes(msg, False, 'mapped')
    g_offset = 0  

    def core(ms, back_link):
        nonlocal g_offset
        if msg.nodes[ms]['mapped']:
            assert back_link is not None, "Only entry has no back_link"
            struct.pack_into('<I', br, back_link, msg.nodes[ms]['pos'])
            return 
        else:
            msg.nodes[ms]['mapped']  = True
            msg.nodes[ms]['pos'] = g_offset
            if back_link:
                struct.pack_into('<I', br, back_link, g_offset)
            cur_offset = g_offset
            n_out_degree = msg.out_degree(ms)
            g_offset += 4 * (n_out_degree + 2)

            pad_br(br, (n_out_degree + 2) * 4)  # self's addr, each succ, and terminator, each takes 4 bytes
            try:
                struct.pack_into('<I', br, cur_offset, ms.content[0].addr)
            except AttributeError:
                struct.pack_into('<I', br, cur_offset, ms)
            cur_offset += 4
            
            for succ in msg.successors(ms):
                core(succ, cur_offset)
                cur_offset += 4
        
    core(ms_entry, None)

    ofile = f'{routine_name}.bin' if routine_name else 'msg.bin'
    with open(ofile, 'wb') as f:
        f.write(br)

def pad_br(br, n_bytes):
    for _ in range(n_bytes):
        pad = struct.pack('<B', 0xff)
        [br.append(b) for b in pad]

def reduced_visualize(g, fname='reduced.dot'):
    vg = pgv.AGraph(strict=False, directed=True)
    vg.node_attr['style'] = 'filled'
    vg.node_attr['shape'] = 'record'
    for n in list(g.nodes):
        if 'tail_t' in g.nodes[n]:
            label = str(hex(n)) + f',{g.nodes[n]["tail_t"]},{hex(g.nodes[n]["tail_t"])}'
        else:
            label = str(hex(n))
        vg.add_node(n, label=label)
        if 'visited' in g.nodes[n] and g.nodes[n]['visited']:
            vg.get_node(n).attr['color'] = "#def2de"
        if 'is_dom' in g.nodes[n] and g.nodes[n]['is_dom']:
            vg.get_node(n).attr['color'] = "#aaffee"
        if 'is_transient_mono' in g.nodes[n] and g.nodes[n]['is_transient_mono']:
            vg.get_node(n).attr['color'] = "#f79862"
        if 'is_2gd_transient_mono' in g.nodes[n] and g.nodes[n]['is_2gd_transient_mono']:
            vg.get_node(n).attr['color'] = "#d43f24"
        if 'is_ms' in g.nodes[n] and g.nodes[n]['is_ms']:
            vg.get_node(n).attr['color'] = "#ff7e7e"
    for e in list(g.edges):
        # if label == 'full':
            # text = str(g.edges[e])
        # if isinstance(label, list):
            # texts = []
            # for l in label:
                # texts.append(f'{l}:{str(g.edges[e][l])}')
            # text = '\l'.join(texts)
        # else:
            # text = g.edges[e][label]
        e_data = g.get_edge_data(*e)
        if 'measures' in e_data:
            vg.add_edge(*e,label=str(e_data['measures'])+','+str(hex(e_data['measures'])))
        else:
            vg.add_edge(*e)
        #vg.add_edge(*e,label='hi')
    vg.write(fname)


def bin2msg(fname):
    with open(fname, 'rb') as f:
        data = f.read()
    iter = struct.iter_unpack('<I', data)
    data = list(item[0] for item in iter)
    
    msg = nx.DiGraph()
    
    is_addr = True
    cur_addr = None
    for val in data:
        if val == 0xffffffff:
            is_addr = True
            continue

        if is_addr:
            cur_addr = val
            msg.add_node(val)
            is_addr = False
            continue

        if not is_addr:
            tar_addr = data[val//4]
            msg.add_edge(cur_addr, tar_addr)
            continue

    return msg, data

















