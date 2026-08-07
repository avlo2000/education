from eso import LinearESO2
from tracking_diff import TrackingDiff


class ADRC:
    """
    Linear Active Disturbance Rejection Control for a second-order plant

        y_ddot = b0 * u + f

    where f lumps together all unknown/nonlinear dynamics and external
    disturbances ("total disturbance").

    Three building blocks:

      1. TrackingDiff  -- turns a (possibly step-like) setpoint into a smooth
         reference trajectory r1 ≈ setpoint and its derivative r2 ≈ d/dt setpoint.

      2. LinearESO2    -- estimates the state (z1 ≈ y, z2 ≈ y_dot) and the total
         disturbance (z3 ≈ f) from the measured output and the applied control.

      3. Control law   -- a PD law on the tracking error drives a *virtual*
         double integrator, then the disturbance estimate is cancelled and the
         result is scaled by 1/b0:

             u0 = kp * (r1 - z1) + kd * (r2 - z2)
             u  = (u0 - z3) / b0

    Because z3 cancels the real disturbance, the closed loop behaves like the
    nominal double integrator y_ddot = u0, for which the PD gains

        kp = wc**2,   kd = 2 * wc

    place both poles at the controller bandwidth -wc.
    """

    def __init__(
        self,
        b0: float,
        wo: float,
        wc: float,
        dt: float,
        td_omega: float = None,
        td_zeta: float = 1.0,
        u_min: float = None,
        u_max: float = None,
    ):
        self.b0 = b0
        self.wc = wc
        self.dt = dt

        if td_omega is None:
            td_omega = 2.0 * wc
        self.td = TrackingDiff(dt, td_omega, td_zeta)

        self.eso = LinearESO2(b0, wo)

        self.kp = wc**2
        self.kd = 2.0 * wc

        self.u_min = u_min
        self.u_max = u_max

        self.u = 0.0

        self.r1 = 0.0
        self.r2 = 0.0
        self.z1 = 0.0
        self.z2 = 0.0
        self.z3 = 0.0

    def reset(self, y0: float = 0.0, y_dot0: float = 0.0, disturbance0: float = 0.0):
        self.eso.reset(y0, y_dot0, disturbance0)
        self.td.x1 = y0
        self.td.x2 = y_dot0
        self.u = 0.0
        self.r1 = y0
        self.r2 = y_dot0
        self.z1 = y0
        self.z2 = y_dot0
        self.z3 = disturbance0

    def _saturate(self, u: float) -> float:
        if self.u_min is not None and u < self.u_min:
            return self.u_min
        if self.u_max is not None and u > self.u_max:
            return self.u_max
        return u

    def update(self, setpoint: float, y: float) -> float:
        self.r1, self.r2 = self.td.filter(setpoint)
        self.z1, self.z2, self.z3 = self.eso.update(y, self.u, self.dt)

        u0 = self.kp * (self.r1 - self.z1) + self.kd * (self.r2 - self.z2)
        u = (u0 - self.z3) / self.b0

        self.u = self._saturate(u)
        return self.u
