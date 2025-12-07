import numpy as np


def quat_mul(q1: np.ndarray, q2: np.ndarray, w_first: bool = False) -> np.ndarray:
    if w_first:
        w1, x1, y1, z1 = q1
        w2, x2, y2, z2 = q2
    else:
        x1, y1, z1, w1 = q1
        x2, y2, z2, w2 = q2
    w = w1 * w2 - x1 * x2 - y1 * y2 - z1 * z2
    x = w1 * x2 + x1 * w2 + y1 * z2 - z1 * y2
    y = w1 * y2 - x1 * z2 + y1 * w2 + z1 * x2
    z = w1 * z2 + x1 * y2 - y1 * x2 + z1 * w2
    if w_first:
        return np.array([w, x, y, z])
    return np.array([x, y, z, w])


def quat_conj(q: np.ndarray, w_first: bool = False) -> np.ndarray:
    if w_first:
        w, x, y, z = q
        return np.array([w, -x, -y, -z])
    x, y, z, w = q
    return np.array([-x, -y, -z, w])