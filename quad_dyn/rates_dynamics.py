from dataclasses import dataclass

import numpy as np
from scipy.spatial.transform import Rotation

from quad_dyn.spatial import quat_mul


G_CONST = 9.81


@dataclass
class RatesDynamicsState:
    q: np.ndarray
    v: np.ndarray
    p: np.ndarray

    def to_vec(self, use_grav: bool = False) -> np.ndarray:
        if not use_grav:
            return np.concatenate([self.q, self.v, self.p])
        return np.concatenate([self.q, self.v, self.p, np.array([G_CONST])])

    @classmethod
    def from_vec(cls, x):
        return cls(q=x[0:4], v=x[4:7], p=x[7:10])

    @classmethod
    def unit_state(cls):
        return cls(q=np.array([1.0, 0, 0, 0]), v=np.array([0.0, 0, 0]), p=np.array([0.0, 0, 0]))


class RatesDynamics:
    def __init__(self, mass: float, b: float, use_grav: bool = True):
        self.mass = mass
        self.b = b
        self.use_grav = use_grav

    @property
    def nx(self) -> int:
        return 11 if self.use_grav else 10

    @property
    def nu(self) -> int:
        return 4

    def state_from_vel(self, vel: np.ndarray):
        return (
            np.array([1, 0, 0, 0, *vel, 0.0, 0.0, 0.0, G_CONST])
            if self.use_grav
            else np.array([1, 0, 0, 0, *vel, 0.0, 0.0, 0.0])
        )

    def state_from_pos(self, pos: np.ndarray):
        return (
            np.array([1.0, 0, 0, 0, 0.0, 0.0, 0.0, *pos, G_CONST])
            if self.use_grav
            else np.array([1.0, 0, 0, 0, 0.0, 0.0, 0.0, *pos])
        )

    def dynamics(self, x: np.ndarray, u: np.ndarray) -> np.ndarray:
        """
        Compute the dynamics of the system.
        x: [q_w, q_x, q_y, q_z, v_x, v_y, v_z, p_x, p_y, p_z, grav]
        x[0:4]: orientation quaternion (scalar first)
        x[4:7]: velocity in NED frame
        x[7:10]: position in NED frame
        x[10]: gravity in NED frame (usually G_CONST)
        u: [w_x, w_y, w_z, thrust]
        """
        q_w = np.array([0.0, *u[0:3]])
        q = x[0:4]
        dq = quat_mul(q, q_w, w_first=True) * 0.5

        thrust_B = np.array([0, 0, u[3] / self.mass])
        q /= np.linalg.norm(q)
        rot = Rotation.from_quat(q, scalar_first=True)

        thrust_NED = rot.apply(thrust_B)
        drag_NED = -self.b * x[4:7]
        grav_NED = np.array([0, 0, G_CONST])
        dv = thrust_NED + grav_NED + drag_NED
        dp = x[4:7]
        return np.concatenate([dq, dv, dp, np.array([0.0])]) if self.use_grav else np.concatenate([dq, dv, dp])

    def dq_by_dq(self, w: np.ndarray) -> np.ndarray:
        return np.array(
            [
                [0.0, -w[0], -w[1], -w[2]],
                [w[0], 0.0, w[2], -w[1]],
                [w[1], -w[2], 0.0, w[0]],
                [w[2], w[1], -w[0], 0.0],
            ]
        )

    def dq_by_dw(self, q: np.ndarray) -> np.ndarray:
        return 0.5 * np.array(
            [
                [-q[0], -q[1], -q[2]],
                [q[3], -q[2], q[1]],
                [q[2], q[3], -q[0]],
                [-q[1], q[0], q[3]],
            ]
        )

    def xdot_by_dx(self, x: np.ndarray, u: np.ndarray) -> np.ndarray:
        """
        x: [q_w, q_x, q_y, q_z, v_x, v_y, v_z, p_x, p_y, p_z, grav]
        u: [w_x, w_y, w_z, thrust]
        """
        q_w, q_x, q_y, q_z = x[0:4]
        A = np.zeros((self.nx, self.nx))

        q_norm = np.sqrt(q_w**2 + q_x**2 + q_y**2 + q_z**2)
        partial_q = (np.eye(4) - np.outer(x[0:4], x[0:4]) / q_norm**2) / q_norm

        # orientation dynamics
        A[0:4, 0:4] = 0.5 * self.dq_by_dq(u[0:3]) @ partial_q

        # velocity dynamics
        dvdot_dq = np.array(
            [
                [q_y, q_z, q_w, q_x],
                [-q_x, -q_w, q_z, q_y],
                [q_w, -q_x, -q_y, q_z],
            ]
        )
        dvdot_dq = dvdot_dq * 2.0 * u[3] / self.mass
        A[4:7, 0:4] = (
            np.array(
                [
                    [q_y, q_z, q_w, q_x],
                    [-q_x, -q_w, q_z, q_y],
                    [q_w, -q_x, -q_y, q_z],
                ]
            )
            * 2.0
            * u[3]
            / self.mass
        )
        A[4:7, 0:4] = A[4:7, 0:4] @ partial_q
        A[4:7, 4:7] = -self.b * np.eye(3) / self.mass
        if self.use_grav:
            A[6, -1] = 1.0
        # position dynamics
        A[7:10, 4:7] = np.eye(3)
        return A

    def xdot_by_du(self, x: np.ndarray) -> np.ndarray:
        q_w, q_x, q_y, q_z = x[0:4]

        B = np.zeros((self.nx, self.nu))

        q_xyzw = np.array([q_x, q_y, q_z, q_w])
        B[0:4, 0:3] = self.dq_by_dw(q_xyzw)

        B[4:7, 3] = (
            np.array(
                [
                    2.0 * (q_w * q_y + q_x * q_z),
                    2.0 * (q_y * q_z - q_w * q_x),
                    q_w**2 - q_x**2 - q_y**2 + q_z**2,
                ]
            )
            / self.mass
        )
        return B


if __name__ == "__main__":
    dynamics_model = RatesDynamics(b=0.0, mass=1.0, use_grav=True)
    q_initial = Rotation.from_rotvec([0.1, 0.0, 0.1]).as_quat(scalar_first=True)
    state = (
        np.array([*q_initial, 0.0, 1, 0, 0, 0, 0, G_CONST])
        if dynamics_model.use_grav
        else np.array([*q_initial, 0, 0, 0, 0, 0, 0])
    )
    state_lin = state.copy()
    rates = np.array([0.51, 1.05, 0.11])
    u = np.array([*rates, G_CONST])

    t = np.linspace(0, 4.0, 500)
    trj_nl = np.empty((len(t) - 1, len(state)))
    trj_lin = np.empty((len(t) - 1, len(state)))
    for i, (t0, t1) in enumerate(zip(t[:-1], t[1:])):
        dt = t1 - t0
        state = state + dynamics_model.dynamics(state, u) * dt

        A = dynamics_model.xdot_by_dx(state_lin, u)
        B = dynamics_model.xdot_by_du(state_lin)
        state_lin = state_lin + (A @ state_lin + B @ u) * dt
        print(state_lin[:4])
        print(np.linalg.norm(state_lin[:4]))
        print("Nonlinear vs Linear")
        print("State:")
        for x, x_lin in zip(state, state_lin):
            print(f"{x:.3f} vs {x_lin:.3f}")

        trj_nl[i] = state
        trj_lin[i] = state_lin
        print("==" * 10)
    import matplotlib.pyplot as plt

    plt.subplots(1, 1)
    plt.plot(t[:-1], trj_lin[:, 0:4], label="Linear")
    plt.plot(t[:-1], trj_nl[:, 0:4], label="Nonlinear quat", linestyle="--")
    plt.legend()
    plt.xlabel("Time (s)")
    plt.title("Quaternion Dynamics")

    plt.subplots(1, 1)
    plt.plot(t[:-1], trj_lin[:, 4:7], label="Linear")
    plt.plot(t[:-1], trj_nl[:, 4:7], label="Nonlinear vel", linestyle="--")
    plt.legend()
    plt.xlabel("Time (s)")
    plt.title("Velocity Dynamics")
    plt.show()
