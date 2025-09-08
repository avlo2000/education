import numpy as np
from state_transition import StateTransition


class KalmanFilter:
    def __init__(self,
                 st: StateTransition,
                 process_cov: np.ndarray,
                 measurement_cov: np.ndarray,
                 ):
        self.st = st
        self.q_mat = process_cov
        self.r_mat = measurement_cov

        self._x_pred = np.zeros(self.st.n_x)
        self._x = np.zeros(self.st.n_x)

        self._p_mat_pred = np.zeros([self.st.n_x, self.st.n_x])
        self._p_mat = np.zeros([self.st.n_x, self.st.n_x])

    def reset(self, x_initial: np.ndarray):
        self._x = x_initial
        self._x_pred = np.zeros(self.st.n_x)

        self._p_mat_pred = np.zeros([self.st.n_x, self.st.n_x])
        self._p_mat = np.zeros([self.st.n_x, self.st.n_x])

    def time_update(self, u: np.ndarray):
        self._x_pred = self.st.next_x(self._x, u)
        self._p_mat_pred = self.st.a_mat @ self._p_mat @ self.st.a_mat.T + self.q_mat

    def measurement_update(self, y: np.ndarray):
        k_gain = self._p_mat_pred @ self.st.c_mat.T @ np.linalg.inv(
            self.st.c_mat @ self._p_mat_pred @ self.st.c_mat.T + self.r_mat
        )
        self._x = self._x_pred + k_gain @ (y - self.st.next_y(self._x_pred))
        self._p_mat = (np.eye(self.st.n_x) - k_gain @ self.st.c_mat) @ self._p_mat_pred

    def get_prediction(self) -> np.ndarray:
        return self._x
