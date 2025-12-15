import numpy as np
from scipy.spatial.transform import Rotation
from scipy.integrate import solve_ivp

from spatial import quat_mul

from dataclasses import dataclass


@dataclass
class ThrustDynamicsParams:
    mass: float
    Cl: float = 1.1  # prop thrust coefficient. Typically between 0.9 and 1.2
    Cd: float = 0.1  # prop drag coefficient (reactive torque). Typically between 0.05 and 0.2
    d: float = 0.25  # distance from quadcopter center to each motor
    I_xx: float = 4.2e-2
    I_yy: float = 4.2e-2
    I_zz: float = 5.2e-2
    I_xy: float = 0.0
    I_xz: float = 0.0
    I_yz: float = 0.0
    g: float = 9.81  # gravity constant


class ThrustDynamicsState:
    p: np.ndarray
    q: np.ndarray
    v: np.ndarray
    w: np.ndarray

    def __init__(self, p: np.ndarray, q: np.ndarray, v: np.ndarray, w: np.ndarray):
        self.p = p
        self.q = q
        self.v = v
        self.w = w

    @classmethod
    def zero_state(cls):
        return cls(
            p=np.array([0.0, 0.0, 0.0]),
            q=np.array([1.0, 0.0, 0.0, 0.0]),
            v=np.array([0.0, 0.0, 0.0]),
            w=np.array([0.0, 0.0, 0.0]),
        )

    def __mul__(self, scalar: float):
        return ThrustDynamicsState(
            p=self.p * scalar,
            q=self.q * scalar,
            v=self.v * scalar,
            w=self.w * scalar,
        )

    def __add__(self, other: 'ThrustDynamicsState'):
        return ThrustDynamicsState(
            p=self.p + other.p,
            q=self.q + other.q,
            v=self.v + other.v,
            w=self.w + other.w,
        )

    @classmethod
    def from_vector(cls, y: np.ndarray):
        return cls(
            p=y[0:3],
            q=y[3:7],
            v=y[7:10],
            w=y[10:13],
        )

    def to_vector(self) -> np.ndarray:
        return np.concatenate([self.p, self.q, self.v, self.w])


class ThrustDynamics:
    def __init__(self, params: ThrustDynamicsParams):
        I = np.array([[params.I_xx, -params.I_xy, -params.I_xz],
                      [-params.I_xy, params.I_yy, -params.I_yz],
                      [-params.I_xz, -params.I_yz, params.I_zz]])
        self.I = I
        d = params.d
        self.r0 = np.array([-d, -d, 0.0]) / np.sqrt(2)
        self.r1 = np.array([d, -d, 0.0]) / np.sqrt(2)
        self.r2 = np.array([d, d, 0.0]) / np.sqrt(2)
        self.r3 = np.array([-d, d, 0.0]) / np.sqrt(2)
        self.params = params

    def _dpos_dt(self, state: ThrustDynamicsState) -> np.ndarray:
        return state.v

    def _dquat_dt(self, state: ThrustDynamicsState) -> np.ndarray:
        w = state.w
        q = state.q
        q_w = np.array([0.0, *w])
        dq = 0.5 * quat_mul(q, q_w, w_first=True)
        return dq

    def _dvel_dt(self, omega: np.ndarray, state: ThrustDynamicsState) -> np.ndarray:
        F_g = np.array([0.0, 0.0, -self.params.mass * self.params.g])
        F_bf = self.params.Cl * omega**2
        F_total_bf = np.array([0.0, 0.0, np.sum(F_bf)])
        R = Rotation.from_quat(state.q, scalar_first=True)
        F_total_wf = R.apply(F_total_bf) + F_g
        acc = F_total_wf / self.params.mass

        tau_prop = self.params.Cd * omega**2
        tau_prop[1] = -tau_prop[1]
        tau_prop[3] = -tau_prop[3]
        tau_bw = (np.cross(self.r0, np.array([0.0, 0.0, F_bf[0]])) +
                  np.cross(self.r1, np.array([0.0, 0.0, F_bf[1]])) +
                  np.cross(self.r2, np.array([0.0, 0.0, F_bf[2]])) +
                  np.cross(self.r3, np.array([0.0, 0.0, F_bf[3]])))
        tau_bw[2] += tau_prop[0] + tau_prop[1] + tau_prop[2] + tau_prop[3]
        dw = np.linalg.inv(self.I) @ (tau_bw - np.cross(state.w, self.I @ state.w))
        return acc, dw

    def dx_dt(self, omega: np.ndarray, state: ThrustDynamicsState) -> ThrustDynamicsState:
        dp = self._dpos_dt(state)
        dq = self._dquat_dt(state)
        dv, dw = self._dvel_dt(omega, state)
        return ThrustDynamicsState(p=dp, q=dq, v=dv, w=dw)


def main():
    from drone_traj import DroneTraj
    from imgui_bundle import imgui, hello_imgui, implot3d, implot

    params = ThrustDynamicsParams(mass=1.0)
    dyn = ThrustDynamics(params)

    dt = 0.01
    T = 15.0
    steps = int(T / dt)

    ts = np.linspace(0, T, steps)
    ps = np.zeros((steps, 3))
    qs = np.zeros((steps, 4))

    w_hover = np.sqrt(params.mass * params.g / (4 * params.Cl))
    w_cmd = w_hover * 1.0
    omega = np.array([w_cmd * 0.1, w_cmd, w_cmd * 0.9, w_cmd])
    

    
    def ode(_, y):
        dstate = dyn.dx_dt(omega, ThrustDynamicsState.from_vector(y))
        return dstate.to_vector()

    state = ThrustDynamicsState.zero_state()
    sol = solve_ivp(ode, [0, T], state.to_vector(), t_eval=ts, method='RK45')
    for i in range(steps):
        s = ThrustDynamicsState.from_vector(sol.y[:, i])
        ps[i] = s.p
        qs[i] = s.q
        print(f"Step {i}, Time {ts[i]:.2f}s, Position: {s.p}, Quaternion: {s.q}")

    traj = DroneTraj()
    traj.set_data(ps, qs, ts)
    
    def gui():
        imgui.text("Drone Simulation: Fly Up")
        traj.draw()

    runner_params = hello_imgui.RunnerParams()
    runner_params.callbacks.show_gui = gui
    runner_params.app_window_params.window_title = "Drone Simulation"

    def post_init():
        implot3d.create_context()
        implot.create_context()
        
    def before_exit():
        implot3d.destroy_context()
        implot.destroy_context()
        
    runner_params.callbacks.post_init = post_init
    runner_params.callbacks.before_exit = before_exit

    hello_imgui.run(runner_params)

if __name__ == "__main__":
    main()
