from __future__ import annotations

import numpy as np
import matplotlib.pyplot as plt
from filterpy.kalman import ExtendedKalmanFilter

G = 9.81
MASS = 1.0


def quat_to_R_bw(q: np.ndarray) -> np.ndarray:
    qw, qx, qy, qz = q
    return np.array([
        [1 - 2 * (qy * qy + qz * qz), 2 * (qx * qy - qw * qz),     2 * (qx * qz + qw * qy)],
        [2 * (qx * qy + qw * qz),     1 - 2 * (qx * qx + qz * qz), 2 * (qy * qz - qw * qx)],
        [2 * (qx * qz - qw * qy),     2 * (qy * qz + qw * qx),     1 - 2 * (qx * qx + qy * qy)],
    ])


def quat_normalize(q: np.ndarray) -> np.ndarray:
    return q / np.linalg.norm(q)


def partial_q_by_q(q: np.ndarray) -> np.ndarray:
    qn2 = float(q @ q)
    qn = np.sqrt(qn2)
    # return np.eye(4)
    return (np.eye(4) - np.outer(q, q) / qn2) / qn


def f_continuous(x: np.ndarray, T: float, mass: float = MASS, g: float = G) -> np.ndarray:
    v = x[3:6]
    q = x[6:10]
    w = x[10:13]

    R = quat_to_R_bw(q)

    dp = v
    dv = np.array([0.0, 0.0, -g]) + (R @ np.array([0.0, 0.0, T])) / mass

    qw, qx, qy, qz = q
    wx, wy, wz = w

    dq = 0.5 * np.array([
        -wx * qx - wy * qy - wz * qz,
         wx * qw + wy * qz - wz * qy,
        -wx * qz + wy * qw + wz * qx,
         wx * qy - wy * qx + wz * qw,
    ])
    dw = np.zeros(3)

    return np.concatenate([dp, dv, dq, dw])


def df_dx(x: np.ndarray, T: float, mass: float = MASS) -> np.ndarray:
    q = x[6:10]
    qw, qx, qy, qz = q
    wx, wy, wz = x[10:13]

    Pq = partial_q_by_q(q)

    A = np.zeros((13, 13))

    A[0:3, 3:6] = np.eye(3)

    s = T / mass
    dvdot_dq_raw = 2.0 * s * np.array([
        [ qy,  qz,  qw,  qx],
        [-qx, -qw,  qz,  qy],
        [0.0, -2.0 * qx, -2.0 * qy, 0.0],
    ])
    A[3:6, 6:10] = dvdot_dq_raw @ Pq

    dqdot_dq_raw = 0.5 * np.array([
        [0.0, -wx, -wy, -wz],
        [ wx, 0.0, -wz,  wy],
        [ wy,  wz, 0.0, -wx],
        [ wz, -wy,  wx, 0.0],
    ])
    A[6:10, 6:10] = dqdot_dq_raw @ Pq

    A[6:10, 10:13] = 0.5 * np.array([
        [-qx, -qy, -qz],
        [ qw,  qz, -qy],
        [-qz,  qw,  qx],
        [ qy, -qx,  qw],
    ])

    return A


def h_meas(x: np.ndarray, T: float, mass: float = MASS, g: float = G) -> np.ndarray:
    p = x[0:3]
    q = x[6:10]
    w = x[10:13]
    R_wb = quat_to_R_bw(q).T
    a_b = R_wb @ np.array([0.0, 0.0, T / mass - g])
    return np.concatenate([p, w, a_b])


def H_jacobian(x: np.ndarray, T: float, mass: float = MASS, g: float = G) -> np.ndarray:
    q = x[6:10]
    qw, qx, qy, qz = q
    H = np.zeros((9, 13))
    H[0:3, 0:3] = np.eye(3)
    H[3:6, 10:13] = np.eye(3)

    s = T / mass - g
    da_dq_raw = 2.0 * s * np.array([
        [-qy,  qz, -qw,  qx],
        [ qx,  qw,  qz,  qy],
        [0.0, -2.0 * qx, -2.0 * qy, 0.0],
    ])
    H[6:9, 6:10] = da_dq_raw @ partial_q_by_q(q)
    return H


def simulate_truth(t: np.ndarray, x0: np.ndarray, w_truth_fn, T_fn, mass: float = MASS):
    xs = np.empty((len(t), 13))
    xs[0] = x0
    for i in range(len(t) - 1):
        dt = t[i + 1] - t[i]
        x = xs[i].copy()

        def rhs(xi, ti):
            xi = xi.copy()
            xi[10:13] = w_truth_fn(ti)
            return f_continuous(xi, T_fn(ti), mass=mass)

        k1 = rhs(x,                 t[i])
        k2 = rhs(x + 0.5 * dt * k1, t[i] + 0.5 * dt)
        k3 = rhs(x + 0.5 * dt * k2, t[i] + 0.5 * dt)
        k4 = rhs(x + dt * k3,       t[i] + dt)
        x = x + (dt / 6.0) * (k1 + 2 * k2 + 2 * k3 + k4)

        x[6:10] = quat_normalize(x[6:10])
        x[10:13] = w_truth_fn(t[i + 1])
        xs[i + 1] = x
    return xs


############### REST IS GENERATED CODE
class QuadEKF:
    """Thin wrapper that drives ``filterpy.kalman.ExtendedKalmanFilter``."""

    def __init__(self, mass: float = MASS, g: float = G):
        self.mass = mass
        self.g = g

        self.ekf = ExtendedKalmanFilter(dim_x=13, dim_z=9, dim_u=1)

        x0 = np.zeros(13)
        x0[6] = 1.0  # qw = 1
        self.ekf.x = x0

        self.ekf.P = np.diag([
            1.0, 1.0, 1.0,                   # position
            0.5, 0.5, 0.5,                   # velocity
            1e-2, 1e-2, 1e-2, 1e-2,          # quaternion (rough att. is known)
            0.05, 0.05, 0.05,                # body rates
        ])

        self._q_pos = 1e-4
        self._q_vel = 5e-2
        self._q_quat = 1e-7
        self._q_w = 1e-1   # treat body rates as a random walk

        self.ekf.R = np.diag([
            0.05 ** 2, 0.05 ** 2, 0.05 ** 2,    # position sensor (m^2)
            0.01 ** 2, 0.01 ** 2, 0.01 ** 2,    # gyro (rad/s)^2
            0.1 ** 2,  0.1 ** 2,  0.1 ** 2,     # accel (m/s^2)^2
        ])

    # ------------------------------------------------------------------ predict
    def predict(self, T: float, dt: float):
        x = self.ekf.x
        # nonlinear propagation (Euler is enough at small dt)
        x_new = x + f_continuous(x, T, mass=self.mass, g=self.g) * dt
        x_new[6:10] = quat_normalize(x_new[6:10])
        self.ekf.x = x_new

        A = df_dx(x, T, mass=self.mass)
        F = np.eye(13) + A * dt

        Q = np.diag([
            self._q_pos, self._q_pos, self._q_pos,
            self._q_vel, self._q_vel, self._q_vel,
            self._q_quat, self._q_quat, self._q_quat, self._q_quat,
            self._q_w, self._q_w, self._q_w,
        ]) * dt

        with np.errstate(all='ignore'):
            self.ekf.P = F @ self.ekf.P @ F.T + Q

    def update(self, z: np.ndarray, T: float):
        self.ekf.update(
            z,
            HJacobian=lambda x: H_jacobian(x, T, mass=self.mass, g=self.g),
            Hx=lambda x: h_meas(x, T, mass=self.mass, g=self.g),
        )
        self.ekf.x[6:10] = quat_normalize(self.ekf.x[6:10])

    @property
    def x(self) -> np.ndarray:
        return self.ekf.x


def main():
    rng = np.random.default_rng(0)

    dt = 0.01
    t_end = 10.0
    t = np.arange(0.0, t_end + dt, dt)

    def w_truth_fn(ti):
        return np.array([
            0.5 * np.sin(0.5 * ti),
            0.3 * np.cos(0.7 * ti),
            0.2,
        ])

    def T_truth_fn(ti):
        # Hover thrust + sizeable modulation so the body-frame accel signal
        # |T/m - g| is non-trivial (otherwise pitch/roll become unobservable).
        return MASS * G * (1.0 + 0.4 * np.sin(0.4 * ti))

    x0 = np.zeros(13)
    x0[6] = 1.0  # identity quaternion

    truth = simulate_truth(t, x0, w_truth_fn, T_truth_fn)

    # Generate noisy measurements
    sigma_p = 0.05
    sigma_w = 0.01
    sigma_a = 0.1

    z_all = np.empty((len(t), 9))
    for i, ti in enumerate(t):
        x = truth[i]
        T_i = T_truth_fn(ti)
        z = h_meas(x, T_i)
        z[0:3] += rng.normal(0.0, sigma_p, 3)
        z[3:6] += rng.normal(0.0, sigma_w, 3)
        z[6:9] += rng.normal(0.0, sigma_a, 3)
        z_all[i] = z

    ekf = QuadEKF(mass=MASS, g=G)
    ekf.ekf.x = np.array([
        0.5, -0.3, 0.2,
        0.0, 0.0, 0.0,
        1.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0,
    ])

    est = np.empty_like(truth)
    P_diag = np.empty((len(t), 13))
    est[0] = ekf.x
    P_diag[0] = np.diag(ekf.ekf.P)
    for i in range(1, len(t)):
        T_i = T_truth_fn(t[i - 1])
        ekf.predict(T_i, dt)
        ekf.update(z_all[i], T_truth_fn(t[i]))
        est[i] = ekf.x
        P_diag[i] = np.diag(ekf.ekf.P)

    # ------------------------------------------------------------------ plotting
    fig, axes = plt.subplots(4, 1, figsize=(11, 11), sharex=True)

    labels = ['x', 'y', 'z']
    for k in range(3):
        axes[0].plot(t, truth[:, k],   color=f'C{k}', label=f'p{labels[k]} truth')
        axes[0].plot(t, est[:, k], '--', color=f'C{k}', label=f'p{labels[k]} est')
        axes[0].plot(t, z_all[:, k], '.', color=f'C{k}', alpha=0.15, markersize=2)
    axes[0].set_ylabel('Position [m]')
    axes[0].legend(ncol=3, fontsize=8)
    axes[0].grid(True, alpha=0.3)

    for k in range(3):
        axes[1].plot(t, truth[:, 3 + k],   color=f'C{k}', label=f'v{labels[k]} truth')
        axes[1].plot(t, est[:, 3 + k], '--', color=f'C{k}', label=f'v{labels[k]} est')
    axes[1].set_ylabel('Velocity [m/s]')
    axes[1].legend(ncol=3, fontsize=8)
    axes[1].grid(True, alpha=0.3)

    quat_labels = ['qw', 'qx', 'qy', 'qz']
    for k in range(4):
        axes[2].plot(t, truth[:, 6 + k],   color=f'C{k}', label=f'{quat_labels[k]} truth')
        axes[2].plot(t, est[:, 6 + k], '--', color=f'C{k}', label=f'{quat_labels[k]} est')
    axes[2].set_ylabel('Quaternion')
    axes[2].legend(ncol=4, fontsize=8)
    axes[2].grid(True, alpha=0.3)

    for k in range(3):
        axes[3].plot(t, truth[:, 10 + k],   color=f'C{k}', label=f'w{labels[k]} truth')
        axes[3].plot(t, est[:, 10 + k], '--', color=f'C{k}', label=f'w{labels[k]} est')
        axes[3].plot(t, z_all[:, 3 + k], '.', color=f'C{k}', alpha=0.15, markersize=2)
    axes[3].set_ylabel('Body rates [rad/s]')
    axes[3].set_xlabel('Time [s]')
    axes[3].legend(ncol=3, fontsize=8)
    axes[3].grid(True, alpha=0.3)

    fig.suptitle('Quadrotor EKF demo (FilterPy) - solid: truth, dashed: estimate, dots: measurements')
    fig.tight_layout()

    err_pos = np.sqrt(np.mean(np.sum((est[:, 0:3] - truth[:, 0:3]) ** 2, axis=1)))
    err_vel = np.sqrt(np.mean(np.sum((est[:, 3:6] - truth[:, 3:6]) ** 2, axis=1)))
    err_q   = np.sqrt(np.mean(np.sum((est[:, 6:10] - truth[:, 6:10]) ** 2, axis=1)))
    err_w   = np.sqrt(np.mean(np.sum((est[:, 10:13] - truth[:, 10:13]) ** 2, axis=1)))
    print(f'RMSE  position : {err_pos:.4f} m')
    print(f'RMSE  velocity : {err_vel:.4f} m/s')
    print(f'RMSE  quat     : {err_q:.4f}')
    print(f'RMSE  rates    : {err_w:.4f} rad/s')

    plt.show()


if __name__ == '__main__':
    main()
