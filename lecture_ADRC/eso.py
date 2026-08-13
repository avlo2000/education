import numpy as np


class ClassicalObserver2:
    """
    Classical (Luenberger) observer for the *nominal* second-order system:

        y_ddot = b0 * u

    State vector:  x = [x1, x2]^T  ≈  [y, y_dot]^T

    Continuous-time observer:
        x_dot = (A - L C) x + B u + L y

    where:
        A = [[0, 1],        B = [[0  ],      C = [1, 0]
             [0, 0]]             [b0 ]]

        L = [l1, l2]^T  with poles at -wo:
            l1 = 2*wo,  l2 = wo^2

    Integrated with forward Euler:
        x[k+1] = x[k] + dt * x_dot[k]
    """

    def __init__(self, b0: float, wo: float):
        self.b0 = b0
        self.wo = wo

        l1 = 2.0 * wo
        l2 = wo**2

        # A - L C
        self.A_lc = np.array([
            [-l1, 1.0],
            [-l2, 0.0],
        ])
        self.B = np.array([0.0, b0])
        self.L = np.array([l1, l2])

        self.x = np.zeros(2)

    def reset(self, y0: float = 0.0, y_dot0: float = 0.0):
        self.x = np.array([y0, y_dot0])

    def update(self, y: float, u: float, dt: float):
        x_dot = self.A_lc @ self.x + self.B * u + self.L * y
        self.x = self.x + dt * x_dot
        return self.x[0], self.x[1]


class LinearESO2:
    """
    Linear Extended State Observer for a second-order system:

        y_ddot = b0 * u + f

    State vector:  z = [z1, z2, z3]^T  ≈  [y, y_dot, f]^T

    Continuous-time observer:
        z_dot = (A - L C) z + B u + L y

    where:
        A = [[0, 1, 0],        B = [[0  ],      C = [1, 0, 0]
             [0, 0, 1],             [b0 ],
             [0, 0, 0]]             [0  ]]

        L = [beta1, beta2, beta3]^T  with triple pole at -wo:
            beta1 = 3*wo,  beta2 = 3*wo^2,  beta3 = wo^3

    Integrated with forward Euler:
        z[k+1] = z[k] + dt * z_dot[k]
    """

    def __init__(self, b0: float, wo: float):
        self.b0 = b0
        self.wo = wo

        beta1 = 6.0 * wo
        beta2 = 3.0 * wo**2
        beta3 = wo**3

        # A - L C
        self.A_lc = np.array([
            [-beta1, 1.0, 0.0],
            [-beta2, 0.0, 1.0],
            [-beta3, 0.0, 0.0],
        ])
        self.B = np.array([0.0, b0, 0.0])
        self.L = np.array([beta1, beta2, beta3])

        self.z = np.zeros(3)

    def reset(self, y0: float = 0.0, y_dot0: float = 0.0, disturbance0: float = 0.0):
        self.z = np.array([y0, y_dot0, disturbance0])

    def update(self, y: float, u: float, dt: float):
        """
        Update ESO using measured output y and control input u.

        Returns:
            estimated_y, estimated_y_dot, estimated_disturbance
        """
        z_dot = self.A_lc @ self.z + self.B * u + self.L * y
        self.z = self.z + dt * z_dot
        return self.z[0], self.z[1], self.z[2]
