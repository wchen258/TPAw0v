import angr
from routine_cfg import *

if __name__=='__main__':
    disparity = angr.Project('../demo/application/disparity')
    cfg_disparity = disparity.analyses.CFGFast()
    d1 = blockorize(cfg_disparity, 'main' )
    d2 = blockorize(cfg_disparity, 'getDisparity' )
    d3 = blockorize(cfg_disparity, 'selfCheck' )

    mser = angr.Project('../demo/application/mser')
    cfg_mser = mser.analyses.CFGFast()
    m1 = blockorize(cfg_mser, 'main' )
    m2 = blockorize(cfg_mser, 'mser' )
    m3 = blockorize(cfg_mser, 'selfCheck' )

    sift = angr.Project('../demo/application/sift')
    cfg_sift = sift.analyses.CFGFast()
    s1 = blockorize(cfg_sift, 'main' )
    s2 = blockorize(cfg_sift, 'sift' )
    s3 = blockorize(cfg_sift, 'selfCheck' )

    texture_synthesis = angr.Project('../demo/application/texture_synthesis')
    cfg_texture_synthesis = texture_synthesis.analyses.CFGFast()
    t1 = blockorize(cfg_texture_synthesis, 'main' )
    t2 = blockorize(cfg_texture_synthesis, 'create_texture' )
    t3 = blockorize(cfg_texture_synthesis, 'selfCheck' )

    tracking = angr.Project('../demo/application/tracking')
    cfg_tracking = tracking.analyses.CFGFast()
    tr1 = blockorize(cfg_tracking, 'main' )
    tr2 = blockorize(cfg_tracking, 'tracking' )
    tr3 = blockorize(cfg_tracking, 'selfCheck' )