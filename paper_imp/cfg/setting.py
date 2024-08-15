def populate(benchmark, tar_rt=None):
    if benchmark == 'sift':
        tar_bin = benchmark + '.dp'
        if tar_rt:
            tar_rt= tar_rt
        else:
            tar_rt = benchmark
        tar_strips = []
        for i in range(1):
            tar_strips.append(f'sift.{i}.strip')
        
    elif benchmark == 'mser':
        tar_bin = 'mser.dp'
        tar_rt = 'mser'
        tar_strips = []
        #for i in range(11):
        for i in range(1):
            tar_strips.append(f'mser.{i}.strip')

    elif benchmark == 'disparity':
        tar_bin = 'disparity.dp'
        tar_rt = 'getDisparity'
        tar_strips = []
        for i in range(9):
            tar_strips.append(f'disparity.{i}.strip')

        # dbg purpose
        # tar_strips = ['disparity.0.strip']

    elif benchmark == 'texture_synthesis':
        tar_bin = benchmark + '.dp'
        tar_rt = 'create_texture'
        tar_strips = []
        for i in range(10):
            tar_strips.append(f'texture_synthesis.{i}.strip')

    elif benchmark == 'tracking':
        tar_bin = benchmark + '.dp'
        tar_rt = 'main'
        tar_strips = ['tracking.range.0.strip']
        tar_strips = ['tracking.0.strip']

    else:
        pass

    return tar_bin, tar_rt, tar_strips
