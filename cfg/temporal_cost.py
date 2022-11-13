def eval_call_cost(node, cfg, strategy='primitive'):
    if strategy == 'primitive':
        return eval_primitive(node, cfg)
    else:
        raise NotImplemented

def eval_primitive(node, cfg):
    pass
