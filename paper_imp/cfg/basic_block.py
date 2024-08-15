from as_cf_utils import ALL_BRANCH_INS, BL_INS

class Lean_BB:

    def __init__(self, content):
        assert content[-1].ins in ALL_BRANCH_INS
        self.content = content
        # TODO purify would break code, due to padding code for routine like <_start>. So the precise entry point of rt is unprecise!!!
        # whenever bl, check whether the branching bb has consistent rt
        # self.purify()
        self.inst_cnt = len(self.content)
        self.rt = self.content[-1].rt
        self.e_succ_bb = None
        self.n_succ_bb = None
        self.pred_bbs = []
        self.out_tunnel = None
        self.in_tunnel = None

        self.natural_succ = None
        self.n_oedge = None
        self.end_ins = self.content[-1]
 
        ########################
        #  ms. g. gen. records #
        ########################

        self.visited = False
        self.is_ms = False
        self.ms_description = None

        #######################
        #  tracer use         #
        #######################
        self.total_hit = 0

    def update_natural_succ(self):
        if self.out_tunnel:
            self.natural_succ = self.out_tunnel
        else:
            self.natural_succ = self.e_succ_bb 

    def __repr__(self) -> str:
        return f'BB {self.content[-1].rt.name} {hex(self.content[0].addr)} - {hex(self.content[-1].addr)} type: {self.content[-1].ins}'

    def update_link_e_to(self, e_bb):
        self.e_succ_bb = e_bb
        if self not in e_bb.pred_bbs:
            e_bb.pred_bbs.append(self)

    def update_link_n_to(self, n_bb):
        self.n_succ_bb = n_bb
        if self not in n_bb.pred_bbs:
            n_bb.pred_bbs.append(self)

    def update_link_ret_to(self, ret_bb):
        self.out_tunnel = ret_bb
        ret_bb.in_tunnel = self

    def update_n_oedge(self):
        if self.content[-1].ins == 'ret':
            self.n_oedge = 0 
        elif self.n_succ_bb:
            self.n_oedge = 2
        else:
            self.n_oedge = 1

    def purify(self):
        true_rt = self.content[-1].rt
        head_rt = self.content[0].rt
        if true_rt is not head_rt:
            for i, asm in enumerate(self.content):
                if asm.rt is true_rt:
                    break
            self.content = self.content[i:]


    #####################
    #  state invariance #
    #####################

    def has_addr(self,addr):
        return True if addr >= self.content[0].addr and addr <= self.content[-1].addr else False

    def content_repr(self):
        repr_l = []
        if self.ms_description:
            repr_l.append('*** Reason for MS ***')
            repr_l.append(self.ms_description)
            repr_l.append('*** Block Info ***')
        repr_l += list(asm.__repr__() for asm in self.content)
        if self.content[-1].ins == 'bl':
            repr_l.append(self.content[-1].to_rt_name[1:-1])
        return '\n\l'.join(repr_l)

    def poke_plt_call(self):
        """ Comment: from a rt to a plt rt, it's not necessarily a bl call
            which means, assert self.content[-1].ins == bl might not pass
            example see iFreeHandle """
        if not self.rt.is_plt:
            if self.e_succ_bb:
                if self.e_succ_bb.rt.is_plt:
                    return self.e_succ_bb.rt
        return None



 
        


