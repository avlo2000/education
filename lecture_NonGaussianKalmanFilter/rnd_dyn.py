from typing import List

import numpy as np

from state_transition import StateTransition


def create_imu6d_st(dt: float):
    """
    State vector of this system is:
    [0..2] -
    [3..5] - linear speed
    Parameters
    ----------
    dt: time step
    """
    pass


def random_st(nx: int, nu: int, ny: int) -> StateTransition:
    return StateTransition(
        np.random.rand(nx, nx),
        np.random.rand(nx, nu),
        np.random.rand(ny, nx),
    )


def rand_positive_semidefinite(n: int, scale: float) -> np.ndarray:
    mat = np.random.rand(n, n) * scale
    mat = mat @ mat.T
    return mat


def random_dynamics(
        st: StateTransition,
        x: np.ndarray,
        u: np.ndarray,
        process_cov: np.ndarray,
        measurement_cov: np.ndarray,
) -> tuple[np.ndarray, np.ndarray, np.ndarray, np.ndarray]:
    x_next = st.next_x(x, u)
    y = st.next_y(x)

    x_next_n = np.random.multivariate_normal(x_next, process_cov)
    y_n = np.random.multivariate_normal(y, measurement_cov)
    return x_next, y, x_next_n, y_n
