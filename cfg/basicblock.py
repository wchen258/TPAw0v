class BasicBlock:
    def __init__(self, *nodes):
        self.angr_nodes = nodes
        self.core = self.merge_nodes(nodes)
        self.addrs = list(self.core.instruction_addrs)
        self.addrs.sort()
        self.start_addr = self.addrs[0]
        self.end_addr = self.addrs[-1]

    def merge_nodes(self, nodes):
        big_n = nodes[0]
        for i in range(1,len(nodes)):
            big_n = big_n.merge(nodes[i])
        return big_n

    def successors_and_jumpkinds(self):
        node = self.angr_nodes[0]
        return node.successors_and_jumpkinds()

        