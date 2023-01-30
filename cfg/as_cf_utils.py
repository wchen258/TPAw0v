UB_INS = ['b']
CB_INS = ['b.ne', 'b.hi', 'b.le', 'b.lt', 'b.eq', 'b.ge', 'b.gt', 'b.mi', 'b.pl', 'b.ls', 'b.cs', 'cbz', 'cbnz', 'tbz', 'tbnz', 'b.cc', 'b.vs', 'b.vc', 'b.al']
BL_INS = ['bl']
RET_INS= ['ret'] # behavior is same to BR_INS, except if the build want to do a call stack to track the possible return address
BR_INS = ['br', 'blr'] # ins here would branch but is not decidable pre-run time, these instruction has no branch address in their assembly, they will be the leaf node of the graph
LINK_INS = ['bl', 'blr']
DUMMY_INS = ['.word','udf']
ADDRESSED_B_INS = UB_INS + CB_INS + BL_INS
ALL_BRANCH_INS = UB_INS + CB_INS + BL_INS + RET_INS + BR_INS

class INS:

    def __init__(self, addr:int,bitcode,ins, asl):
        self.addr = addr
        self.bitcode = bitcode
        self.ins = ins
        self.asl = asl
        self.esuccessor_addr = None
        self.raw_line = None
        self.rt = None
        self.is_cb = True if self.ins in CB_INS else False
        self.is_link = True if self.ins in LINK_INS else False
        self.to_rt_name = None  # the str of branching rt if ins is bl

        if ins in ADDRESSED_B_INS:
            tokens = asl.split(' ')
            if ins=='cbz' or ins=='cbnz':
                self.esuccessor_addr = int(tokens[1],16)
            elif ins=='tbz' or ins=='tbnz':
                self.esuccessor_addr = int(tokens[2], 16)
            else:
                self.esuccessor_addr = int(tokens[0],16)
            if ins=='bl':
                self.to_rt_name = tokens[1]

    @classmethod
    def from_line(cls,line):
        ncls = cls(*asl2ins(line))
        ncls.raw_line = line
        return ncls

    def __repr__(self) -> str:
        comp1 = f'{self.rt.name[1:-1]} {hex(self.addr)[2:]} {self.ins} ' 
        comp2 = f'E:{hex(self.esuccessor_addr)[2:]} ' if self.esuccessor_addr is not None else ' '
        return comp1 + comp2 


class Routine:

    def __init__(self, tag, content) -> None:
        tag_token = tag.split(' ')
        self.start_addr = int(tag_token[0],16)
        self.name = tag_token[1][:-2]
        self.name_strip = self.name[1:-1]
        self.is_plt = self.is_plt()
        if content[-1]=='\t...\n':
            content.pop()
        end_addr,bitcode,s,as_ins = asl2ins(content[-1])
        self.end_addr = end_addr
        self.prev_r = None
        self.next_r = None
        self.content = []
        self.rets = []
        for line in content:
            ins = INS.from_line(line)
            ins.rt = self
            self.content.append(ins)
            if ins.ins == 'ret':
                self.rets.append(ins)

        self.weight = None

    def detail_repr(self) -> str:
        s = f'{self.name}\nStart:{hex(self.start_addr)}\nEnd:{hex(self.end_addr)}\n'
        for l in self.content:
            s += l.raw_line
        s+= f'prev: {self.prev_r.name}\n' if self.prev_r is not None else f'prev: None\n'
        s+= f'next: {self.next_r.name}\n' if self.next_r is not None else f'next: None\n'
        return s
    
    def is_plt(self):
        if len(self.name_strip) > 4:
            postfix = self.name_strip[-4:]
            if postfix == '.plt' or postfix == '@plt':
                return True 
        return False

    def __repr__(self) -> str:
        return self.name

    def contains_addr(self, addr:int):
        return True if addr <= self.end_addr and addr >= self.start_addr else False

def read_as(fname, section='all'):
    '''
    read each section by searching keyword Disassembly of section range
    for example read_as(fname, '.text) would only produce .text section
    default all sections
    '''
    with open(fname,'r') as f:
        lines = f.readlines()
    if section!='all':
        for i, line in enumerate(lines):
            if f'Disassembly of section {section}' in line:
                lines = lines[i+2:]
                break
        for i, line in enumerate(lines):
            if 'Disassembly of section ' in line:
                lines = lines[:i]
    else:
        for i, l in enumerate(lines):
            if f'Disassembly of section ' in l:
                lines = lines[i:]
                break
        indice_to_remove = []
        for i, l in enumerate(lines):
            if f'Disassembly of section ' in l:
                indice_to_remove.append(i)
                indice_to_remove.append(i+1)
        for index in sorted(indice_to_remove, reverse=True):
            del lines[index]
        lines.append('\n')  ## this is to conform the format in 'section' strategy. 
    ##print(lines)
    return lines

def consume_subroutine(lines):
    if len(lines)==0:
        return None, None, None
    tag = lines[0]
    lines = lines[1:] # discard the tag
    for i, line in enumerate(lines):
        if line=='\n':
            content = lines[:i]
            rest = lines[i+1:]
            return tag, content, rest


def parse_section(lines):
    routines = []
    prev_r = None
    while(True):
        tag, content, lines = consume_subroutine(lines)
        if tag is None:
            break
        new_r = Routine(tag, content)
        new_r.prev_r = prev_r
        if prev_r is not None:
            prev_r.next_r = new_r
        routines.append(new_r)
        prev_r = new_r
    return routines

def asl2ins(line):
    #print('line:',line)
    addr = int(line[2:8],16)
    bitcode = line[10:18]
    s = ''
    tracker = 0
    for i in range(20,28):
        if line[i]!='\t' and line[i]!='\n':
            s += line[i]
        else:
            tracker = i
            break
    as_ins = line[tracker:].strip()
    return addr,bitcode,s,as_ins

def seek_ins_by_addr(instructions, addr:int):
    for ins in instructions:
        if ins.addr == addr:
            return ins
    print(f'seek instruction with address {addr} failed!')
    return None

def generate_ins(lines):
    instructions = list(INS(*asl2ins(l)) for l in lines)

    # generate instructions , and add their predecessor and successor respectively
    for i in range(len(instructions)-1):
        if i==0:
            instructions[i].predecessor = None
            instructions[i].nsuccesor = instructions[i+1]
        else:
            instructions[i].predecessor = instructions[i-1]
            instructions[i].nsuccesor = instructions[i+1]
    last_ins = instructions[len(instructions)-1]
    last_ins.predecessor = instructions[len(instructions)-2]
    last_ins.nsuccesor = None

    # add E-atom branch
    for instruction in instructions:
        if instruction.has_esucc:
            instruction.esuccessor = seek_ins_by_addr(instructions, instruction.esuccessor_addr)
    return instructions

def concat_routines_content(routines):
    content = []
    for r in routines:
        for line in r[1]:
            content.append(line)
    return content
    
def inside(obj, objs):
    for o in objs:
        if obj is o:
            return True
    return False

if __name__=='__main__':
    text_section = read_as('/home/wfc/pl-trace-minus/testfield/test_naive.ddp')
    routines = parse_section(text_section)
