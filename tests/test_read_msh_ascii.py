import numpy as np
import pymeshb


def test_read_msh_ascii():
    ref = {}
    ref['Quadrilaterals'] = np.array([[1, 2, 3, 4, 1], [2, 5, 6, 3, 2]])
    ref['Vertices'] = []
    ref['Vertices'].append(np.array([[0.,  0.,  0.], [1.72,  0.,  0.], [1.35,  1.52,  0.],
                                     [0.,  1.62,  0.], [2.13,  0.,  0.], [2.1,  1.96,  0.]]))
    ref['Vertices'].append(np.array([[1], [2], [3], [4], [5], [6]]))
    msh = pymeshb.read('share/quad.mesh')

    assert msh['Dimension'] == 3
    np.testing.assert_allclose(
        msh['Quadrilaterals'], ref['Quadrilaterals'], atol=1e-6)
    np.testing.assert_allclose(
        msh['Vertices'][0], ref['Vertices'][0], atol=1e-6)
    np.testing.assert_allclose(
        msh['Vertices'][1], ref['Vertices'][1], atol=1e-6)
