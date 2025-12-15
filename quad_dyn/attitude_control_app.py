
import numpy as np
from imgui_bundle import imgui, hello_imgui, implot3d, implot
from scipy.spatial.transform import Rotation
from scipy.integrate import solve_ivp
from thrust_dynamics import ThrustDynamics, ThrustDynamicsParams, ThrustDynamicsState
from attitude_control_pid import AttitudeControlPID, AttitudeControlPIDParams
from drone_traj import DroneTraj
from pid import PIDParams

class SimulationState:
    def __init__(self):
        self.needs_update = True
        self.traj = DroneTraj()

        self.target_roll = 0.0
        self.target_pitch = 0.0
        self.target_yaw = 0.0
        self.target_climb_rate = 0.0

        self.att_ctrl_params = AttitudeControlPIDParams()

sim_state = SimulationState()

def pid_ui(label: str, params: PIDParams) -> bool:
    changed = False
    if imgui.tree_node(label):
        if imgui.is_item_deactivated_after_edit(): params.P = imgui.get_item_rect_min()[0] # This is wrong usage of imgui python bindings for simple types
        
        c, params.P = imgui.slider_float(f"P##{label}", params.P, 0.0, 50.0)
        changed |= c
        c, params.I = imgui.slider_float(f"I##{label}", params.I, 0.0, 10.0)
        changed |= c
        c, params.D = imgui.slider_float(f"D##{label}", params.D, 0.0, 1.0)
        changed |= c
        c, params.FF = imgui.slider_float(f"FF##{label}", params.FF, 0.0, 20.0)
        changed |= c
        c, params.D_FF = imgui.slider_float(f"D_FF##{label}", params.D_FF, 0.0, 1.0)
        changed |= c
        c, params.max_integral = imgui.slider_float(f"Max I##{label}", params.max_integral, 0.0, 100.0)
        changed |= c
        c, params.min_output = imgui.input_float(f"Min Out##{label}", params.min_output)
        changed |= c
        c, params.max_output = imgui.input_float(f"Max Out##{label}", params.max_output)
        changed |= c
        
        imgui.tree_pop()
    return changed

def run_simulation():
    params = ThrustDynamicsParams(
        mass=1.0,
        Cl=3.0,
        Cd=1.1
    )
    dyn = ThrustDynamics(params)

    att_ctrl = AttitudeControlPID(sim_state.att_ctrl_params)
    att_ctrl.reset()

    dt = 0.1
    T = 10.0
    steps = int(T / dt)

    ts = np.linspace(0, T, steps)
    ps = np.zeros((steps, 3))
    qs = np.zeros((steps, 4))
    vs = np.zeros((steps, 3))
    qs_des = np.zeros((steps, 4))
    
    target_euler = [sim_state.target_roll, sim_state.target_pitch, sim_state.target_yaw]
    target_quat = Rotation.from_euler('xyz', target_euler, degrees=True).as_quat(scalar_first=True)


    def ode(t, y):
        state = ThrustDynamicsState.from_vector(y)
        current_quat = state.q
        current_quat /= np.linalg.norm(current_quat)
        current_rates = state.w
        omega = att_ctrl.control(
            target_quat,
            current_quat,
            current_rates,
            target_climb_rate=sim_state.target_climb_rate,
            current_climb_rate=state.v[2],
            dt=dt
        )
        dstate = dyn.dx_dt(omega, ThrustDynamicsState.from_vector(y))
        return dstate.to_vector()

    state = ThrustDynamicsState.zero_state()
    sol = solve_ivp(ode, [0, T], state.to_vector(), t_eval=ts, method='RK45')
    for i in range(steps):
        s = ThrustDynamicsState.from_vector(sol.y[:, i])
        ps[i] = s.p
        qs[i] = s.q
        vs[i] = s.v
        qs_des[i] = target_quat

    sim_state.traj = DroneTraj()
    sim_state.traj.set_data(ps, qs, ts)

    r_act = Rotation.from_quat(qs, scalar_first=True)
    euler_act = r_act.as_euler('xyz', degrees=True)
    
    r_des = Rotation.from_quat(qs_des, scalar_first=True)
    euler_des = r_des.as_euler('xyz', degrees=True)

    sim_state.traj.add_value_plot("Roll").add_data("act", euler_act[:, 0]).add_data("des", euler_des[:, 0])
    sim_state.traj.add_value_plot("Pitch").add_data("act", euler_act[:, 1]).add_data("des", euler_des[:, 1])
    sim_state.traj.add_value_plot("Yaw").add_data("act", euler_act[:, 2]).add_data("des", euler_des[:, 2])
    sim_state.traj.add_value_plot("Climb Rate").add_data("act", vs[:, 2]).add_data(
        "des", np.full_like(ts, sim_state.target_climb_rate))

def gui():
    if imgui.begin("Setpoints"):
        changed = False
        c, sim_state.target_roll = imgui.slider_float("Roll (deg)", sim_state.target_roll, -45.0, 45.0)
        changed |= c
        c, sim_state.target_pitch = imgui.slider_float("Pitch (deg)", sim_state.target_pitch, -45.0, 45.0)
        changed |= c
        c, sim_state.target_yaw = imgui.slider_float("Yaw (deg)", sim_state.target_yaw, -180.0, 180.0)
        changed |= c
        c, sim_state.target_climb_rate = imgui.slider_float("Climb Rate (m/s)", sim_state.target_climb_rate, -5.0, 5.0)
        changed |= c
        
        if changed:
            sim_state.needs_update = True
    imgui.end()

    if imgui.begin("PID Tuning"):
        changed = False
        changed |= pid_ui("Roll Rate PID", sim_state.att_ctrl_params.roll_rate_pid)
        changed |= pid_ui("Pitch Rate PID", sim_state.att_ctrl_params.pitch_rate_pid)
        changed |= pid_ui("Yaw Rate PID", sim_state.att_ctrl_params.yaw_rate_pid)
        changed |= pid_ui("Thrust PID", sim_state.att_ctrl_params.thrust_pid)
        
        if changed:
            sim_state.needs_update = True
    imgui.end()

    if sim_state.needs_update:
        run_simulation()
        sim_state.needs_update = False

    imgui.text("Drone Simulation")
    sim_state.traj.draw()

def main():
    runner_params = hello_imgui.RunnerParams()
    runner_params.callbacks.show_gui = gui
    runner_params.app_window_params.window_title = "Attitude Control Tuning"
    runner_params.app_window_params.window_geometry.size = (1600, 900)

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
