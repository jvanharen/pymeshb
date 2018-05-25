import numpy as np
import pymeshb
import os

def test_write_sol_bin():
    ref = {}
    ref['SolAtVertices'] = []
    ref['SolAtVertices'].append(np.array([[1.,  6.,  8.], [2.,  4.,  8.],
                                          [3.,  2.,  6.], [4.,  5.,  6.],
                                          [5.,  2.,  6.]]))

    tmp = pymeshb.read('share/out.solb')
    pymeshb.write(tmp, 'share/tmp.solb')
    sol = pymeshb.read('share/tmp.solb')

    assert sol['Dimension'] == 3
    np.testing.assert_allclose(
        sol['SolAtVertices'][0], ref['SolAtVertices'][0], atol=1e-6)

    os.remove('share/tmp.solb')
