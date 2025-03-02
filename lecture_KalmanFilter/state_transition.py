import numpy as np


class StateTransition:
    def __init__(self,
                 a_mat: np.ndarray,
                 b_mat: np.ndarray,
                 c_mat: np.ndarray
                 ):
        self.a_mat = a_mat
        self.b_mat = b_mat
        self.c_mat = c_mat

    def next_x(self, x: np.ndarray, u: np.ndarray) -> np.ndarray:
        return self.a_mat @ x + self.b_mat @ u

    def next_y(self, x: np.ndarray) -> np.ndarray:
        return self.c_mat @ x

    @property
    def n_x(self) -> int:
        return self.a_mat.shape[0]

    @property
    def n_u(self) -> int:
        return self.a_mat.shape[1]

    @property
    def n_y(self) -> int:
        return self.a_mat.shape[1]

