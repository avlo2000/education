import numpy as np
from scipy.spatial.transform import Rotation

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
        self.state = ThrustDynamicsState.zero_state()

    def _dpos_dt(self) -> np.ndarray:
        return self.state.v

    def _dquat_dt(self) -> np.ndarray:
        w = self.state.w
        q = self.state.q
        q_w = np.array([0.0, *w])
        dq = 0.5 * quat_mul(q, q_w, w_first=True)
        return dq

    def _dvel_dt(self, omega: np.ndarray) -> np.ndarray:
        F_g = np.array([0.0, 0.0, -self.params.mass * self.params.g])
        F_bf = self.params.Cl * omega**2
        F_total_bf = np.array([0.0, 0.0, np.sum(F_bf)])
        R = Rotation.from_quat(self.state.q, scalar_first=True)
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
        dw = np.linalg.inv(self.I) @ (tau_bw - np.cross(self.state.w, self.I @ self.state.w))
        return acc, dw

    def dx_dt(self, omega: np.ndarray) -> ThrustDynamicsState:
        dp = self._dpos_dt()
        dq = self._dquat_dt()
        dv, dw = self._dvel_dt(omega)
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
    w_cmd = w_hover * 1.0 # 5% more thrust to fly up
    omega = np.array([w_cmd, w_cmd, w_cmd + 0.01, w_cmd])
    
    state = dyn.state
    
    for i in range(steps):
        ps[i] = state.p
        qs[i] = state.q

        dstate = dyn.dx_dt(omega)
        state = state + dstate * dt
        state.q = state.q / np.linalg.norm(state.q)
        dyn.state = state

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
