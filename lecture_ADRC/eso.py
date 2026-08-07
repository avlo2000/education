class ClassicalObserver2:
    """
    Classical (Luenberger) observer for the *nominal* second-order system:

        y_ddot = b0 * u

    It has NO disturbance model, so it estimates only:
        x1 ≈ y
        x2 ≈ y_dot

    Observer gains place both error poles at -wo:
        (s + wo)^2 = s^2 + 2*wo*s + wo^2   =>   l1 = 2*wo, l2 = wo^2
    """

    def __init__(self, b0: float, wo: float):
        self.b0 = b0
        self.wo = wo

        self.l1 = 2.0 * wo
        self.l2 = wo**2

        self.x1 = 0.0
        self.x2 = 0.0

    def reset(self, y0: float = 0.0, y_dot0: float = 0.0):
        self.x1 = y0
        self.x2 = y_dot0

    def update(self, y: float, u: float, dt: float):
        """
        Update observer using measured output y and control input u.

        Returns:
            estimated_y, estimated_y_dot
        """

        e = self.x1 - y

        x1_dot = self.x2 - self.l1 * e
        x2_dot = self.b0 * u - self.l2 * e

        self.x1 += dt * x1_dot
        self.x2 += dt * x2_dot

        return self.x1, self.x2


class LinearESO2:
    """
    Linear Extended State Observer for a second-order system:

        y_ddot = b0 * u + f

    Estimates:
        z1 ≈ y
        z2 ≈ y_dot
        z3 ≈ total disturbance f
    """

    def __init__(self, b0: float, wo: float):
        self.b0 = b0
        self.wo = wo

        self.beta1 = 3.0 * wo
        self.beta2 = 3.0 * wo**2
        self.beta3 = wo**3

        self.z1 = 0.0
        self.z2 = 0.0
        self.z3 = 0.0

    def reset(self, y0: float = 0.0, y_dot0: float = 0.0, disturbance0: float = 0.0):
        self.z1 = y0
        self.z2 = y_dot0
        self.z3 = disturbance0

    def update(self, y: float, u: float, dt: float):
        """
        Update ESO using measured output y and control input u.

        Returns:
            estimated_y, estimated_y_dot, estimated_disturbance
        """

        e = self.z1 - y

        z1_dot = self.z2 - self.beta1 * e
        z2_dot = self.z3 + self.b0 * u - self.beta2 * e
        z3_dot = -self.beta3 * e

        self.z1 += dt * z1_dot
        self.z2 += dt * z2_dot
        self.z3 += dt * z3_dot

        return self.z1, self.z2, self.z3
