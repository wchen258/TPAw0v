import sys
import networkx as nx
import struct
from msg_binarize import bin2msg, reduced_visualize, pad_br
from tools import derive_end_point

def preprocess(fname):
    with open(fname, 'r') as f:
        raw = f.read().splitlines()

    logs = []
    cur_log = []
    on = False
    for l in raw:
        if l == 'LOG BEGIN':
            on = True
            continue

        if l == 'LOG END':
            on = False
            logs.append(cur_log)
            cur_log = []

        if on:
            cur_log.append(l)

    return logs

def preprocess_log(log, msg):
    for pivot,val in enumerate(log):
        if val[0] == '#':
            break
    
    get_t = lambda x: int(x.split(',')[2])
    get_addr = lambda x: int(x.split(',')[1], 16)

    g_time = 0

    for i,val in enumerate(log):
        if i==pivot:
            break
        measure = get_t(val)
        g_time += measure
        u = get_addr(log[i + pivot])
        v = get_addr(log[i + pivot + 1])

        print(f'{hex(u)} -> {hex(v)}')
        edge_data = msg.get_edge_data(u,v)
        if 'measures' in edge_data:
            edge_data['measures'].append(measure)
        else:
            edge_data['measures'] = [measure]

        if 'tail_t' in msg.nodes[v]:
            cur_t = msg.nodes[v]['tail_t']
            msg.nodes[v]['tail_t'] = g_time if g_time > cur_t else cur_t
        else:
            msg.nodes[v]['tail_t'] = g_time

        # if 'tail_t' in edge_data:
            # cur_t = edge_data['tail_t']
            # edge_data['tail_t'] = g_time if g_time > cur_t else cur_t 
        # else:
            # edge_data['tail_t'] = g_time
        

    return msg

def gen_nominal(msg):
    for e in msg.edges:
        data = msg.get_edge_data(*e)
        if 'measures' in data:
            data['measures'] = max(data['measures'])
        else:
            data['measures'] = 0
    return msg

def binarize_relative_tail(msg, output_name=None):
    ms_entry, _ = derive_end_point(msg)
    br = bytearray()
    nx.set_node_attributes(msg, False, 'mapped')
    g_offset = 0  

    def core(ms, back_link, back_link_ms):
        nonlocal g_offset
        if msg.nodes[ms]['mapped']:
            assert back_link is not None, "Only entry has no back_link"
            struct.pack_into('<II', br, back_link, msg.nodes[ms]['pos'], msg.get_edge_data(back_link_ms, ms)['measures'])
            return 
        else:
            msg.nodes[ms]['mapped']  = True
            msg.nodes[ms]['pos'] = g_offset
            if back_link and back_link_ms:
                struct.pack_into('<II', br, back_link, g_offset, msg.get_edge_data(back_link_ms,ms)['measures'])
            cur_offset = g_offset
            n_out_degree = msg.out_degree(ms)
            g_offset += 8 * (n_out_degree) + 8 + 4 # 1 int for addr, 1 int for pad, 2 int for each link, in which one is pos, one is max measure , tail 4 for g_time
            pad_br(br, (n_out_degree) * 8 + 8 + 4)  # self's addr, each succ, and terminator, each takes 4 bytes
            struct.pack_into('<II', br, cur_offset, ms, msg.nodes[ms]['tail_t'])
            cur_offset += 8
            
            for succ in msg.successors(ms):
                core(succ, cur_offset, ms)
                cur_offset += 8
        
    core(ms_entry, None, None)

    with open(output_name, 'wb') as f:
        f.write(br)

def binarize_relative(msg, output_name=None):
    ms_entry, _ = derive_end_point(msg)
    br = bytearray()
    nx.set_node_attributes(msg, False, 'mapped')
    g_offset = 0  

    def core(ms, back_link, back_link_ms):
        nonlocal g_offset
        if msg.nodes[ms]['mapped']:
            assert back_link is not None, "Only entry has no back_link"
            struct.pack_into('<II', br, back_link, msg.nodes[ms]['pos'], msg.get_edge_data(back_link_ms, ms)['measures'])
            return 
        else:
            msg.nodes[ms]['mapped']  = True
            msg.nodes[ms]['pos'] = g_offset
            if back_link and back_link_ms:
                struct.pack_into('<II', br, back_link, g_offset, msg.get_edge_data(back_link_ms,ms)['measures'])
            cur_offset = g_offset
            n_out_degree = msg.out_degree(ms)
            g_offset += 8 * (n_out_degree) + 8 # 1 int for addr, 1 int for pad, 2 int for each link, in which one is pos, one is max measure 

            pad_br(br, (n_out_degree) * 8 + 8)  # self's addr, each succ, and terminator, each takes 4 bytes
            struct.pack_into('<I', br, cur_offset, ms)
            cur_offset += 4
            
            for succ in msg.successors(ms):
                core(succ, cur_offset, ms)
                cur_offset += 8
        
    core(ms_entry, None, None)

    with open(output_name, 'wb') as f:
        f.write(br)

if __name__=='__main__':
    logs = preprocess(sys.argv[1])
    msg,data = bin2msg(sys.argv[2])
    nx.set_node_attributes(msg, 0, 'tail_t')
    logs = logs[-9:]
    for log in logs:
        msg = preprocess_log(log, msg)
    gen_nominal(msg)
    reduced_visualize(msg)
    binarize_relative_tail(msg, f'output.ttmsg')

