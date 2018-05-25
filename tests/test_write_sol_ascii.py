import numpy as np
import pymeshb
import os

def test_write_sol_ascii():
    ref = {}
    ref['SolAtVertices'] = []
    ref['SolAtVertices'].append(np.array([[1.,  6.,  8.], [2.,  4.,  8.],
                                          [3.,  2.,  6.], [4.,  5.,  6.],
                                          [5.,  2.,  6.]]))

    tmp = pymeshb.read('share/out.sol')
    pymeshb.write(tmp, 'share/tmp.sol')
    sol = pymeshb.read('share/tmp.sol')

    assert sol['Dimension'] == 3
    np.testing.assert_allclose(
        sol['SolAtVertices'][0], ref['SolAtVertices'][0], atol=1e-6)

    os.remove('share/tmp.sol')
