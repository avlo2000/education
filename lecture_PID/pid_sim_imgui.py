import dearpygui.dearpygui as dpg
import numpy as np
import control as ct


class PIDSimulator:
    def __init__(self, simulation_time=40.0, num_points=500):
        self.simulation_time = simulation_time
        self.num_points = num_points
        self.time = np.linspace(0, simulation_time, num_points)

        self.mass = 10.0
        self.spring_k = 1.0
        self.damping_b = 0.02

        self.create_plant()
        
    def create_plant(self):
        num = [1.0]
        den = [self.mass, self.damping_b, self.spring_k]
        self.plant_tf = ct.tf(num, den)
        
    def create_pid_controller(self, kp, ki, kd):
        num = [kd, kp, ki]
        den = [1, 0]
        return ct.tf(num, den)
    
    def simulate_step_response(self, kp=1.0, ki=0.1, kd=0.01, latency=0.0, kff=0.0, kdf=0.0):
        """Simulate closed-loop step response with latency"""
        try:
            # Use time-domain simulation for all cases to support FF terms and latency consistently
            return self._simulate_time_domain(kp, ki, kd, latency, kff, kdf)
                
        except Exception as e:
            print(f"Simulation error: {e}")
            # Return zeros if simulation fails
            zeros = np.zeros_like(self.time)
            return self.time, zeros, zeros, zeros, zeros, zeros, zeros, zeros, zeros
    
    def _simulate_time_domain(self, kp, ki, kd, latency, kff, kdf):
        dt = self.time[1] - self.time[0] if len(self.time) > 1 else 0.01

        output = np.zeros_like(self.time)
        control = np.zeros_like(self.time)
        error = np.zeros_like(self.time)
        p_term = np.zeros_like(self.time)
        i_term = np.zeros_like(self.time)
        d_term = np.zeros_like(self.time)
        ff_term = np.zeros_like(self.time)
        df_term = np.zeros_like(self.time)

        delay_samples = int(latency / dt)
        if delay_samples == 0:
            delay_samples = 1

        integral = 0.0
        prev_error = 0.0
        prev_setpoint = 0.0

        plant_state = np.array([0.0, 0.0])
        
        for i, t in enumerate(self.time):
            setpoint = 1.0

            if i == 0:
                measured_output = 0.0
            else:
                if i < delay_samples:
                    measured_output = 0.0
                else:
                    measured_output = output[i - delay_samples]

            current_error = setpoint - measured_output
            error[i] = current_error

            p_term[i] = kp * current_error
            integral += current_error * dt
            i_term[i] = ki * integral

            if i > 0:
                derivative = (current_error - prev_error) / dt
                d_term[i] = kd * derivative

            ff_term[i] = kff * setpoint
            
            if i > 0:
                setpoint_derivative = (setpoint - prev_setpoint) / dt
                df_term[i] = kdf * setpoint_derivative
            else:
                df_term[i] = kdf * (setpoint - 0.0) / dt

            control[i] = p_term[i] + i_term[i] + d_term[i] + ff_term[i] + df_term[i]

            position, velocity = plant_state
            acceleration = (control[i] - self.damping_b * velocity - self.spring_k * position) / self.mass

            new_velocity = velocity + acceleration * dt
            new_position = position + new_velocity * dt
            
            plant_state = np.array([new_position, new_velocity])
            output[i] = new_position
            
            prev_error = current_error
            prev_setpoint = setpoint
        
        return self.time, output, control, error, p_term, i_term, d_term, ff_term, df_term

    def get_system_info(self, kp, ki, kd, latency):
        pid = self.create_pid_controller(kp, ki, kd)
        forward = ct.series(pid, self.plant_tf)
        closed_loop = ct.feedback(forward, 1)

        poles = ct.poles(closed_loop)
        
        if latency > 0:
            is_stable = all(np.real(poles) < 0)
        else:
            is_stable = all(np.real(poles) < 0)
        
        try:
            step_info = ct.step_info(closed_loop)
        except:
            step_info = {'SettlingTime': 0, 'Overshoot': 0}
        
        return {
            'poles': poles,
            'stable': is_stable,
            'step_info': step_info,
            'latency_effect': latency > 0
        }


sim = PIDSimulator()

def update_plots(sender, app_data, user_data):
    kp = dpg.get_value("kp_input")
    ki = dpg.get_value("ki_input")
    kd = dpg.get_value("kd_input")
    kff = dpg.get_value("kff_input")
    kdf = dpg.get_value("kdf_input")
    latency = dpg.get_value("latency_input")
    
    time, output, control_signal, error, p_term, i_term, d_term, ff_term, df_term = sim.simulate_step_response(kp, ki, kd, latency, kff, kdf)

    dpg.set_value("series_output", [time, output])
    dpg.set_value("series_setpoint", [time, np.ones_like(time)])
    dpg.fit_axis_data("axis_x_step")
    dpg.fit_axis_data("axis_y_step")

    dpg.set_value("series_control", [time, control_signal])
    dpg.fit_axis_data("axis_x_control")
    dpg.fit_axis_data("axis_y_control")

    dpg.set_value("series_error", [time, error])
    dpg.fit_axis_data("axis_x_error")
    dpg.fit_axis_data("axis_y_error")

    dpg.set_value("series_p", [time, p_term])
    dpg.set_value("series_i", [time, i_term])
    dpg.set_value("series_d", [time, d_term])
    dpg.set_value("series_ff", [time, ff_term])
    dpg.set_value("series_df", [time, df_term])
    dpg.fit_axis_data("axis_x_pid")
    dpg.fit_axis_data("axis_y_pid")

    info = sim.get_system_info(kp, ki, kd, latency)
    dpg.set_value("info_stable", f"Stable: {info['stable']}")
    
    step_info = info['step_info']
    if step_info and 'SettlingTime' in step_info:
            dpg.set_value("info_settling", f"Settling Time: {step_info['SettlingTime']:.2f} s")
    else:
            dpg.set_value("info_settling", "Settling Time: N/A")
            
    if step_info and 'Overshoot' in step_info:
            dpg.set_value("info_overshoot", f"Overshoot: {step_info['Overshoot']:.2f} %")
    else:
            dpg.set_value("info_overshoot", "Overshoot: N/A")

def main():
    dpg.create_context()
    dpg.create_viewport(title='PID Simulator', width=1200, height=800)
    dpg.setup_dearpygui()

    with dpg.window(label="Controls", width=300, height=800, pos=(0, 0)):
        dpg.add_text("PID Parameters")
        dpg.add_slider_float(label="Kp", tag="kp_input", default_value=1.0, min_value=0.0, max_value=50.0, callback=update_plots)
        dpg.add_slider_float(label="Ki", tag="ki_input", default_value=0.1, min_value=0.0, max_value=20.0, callback=update_plots)
        dpg.add_slider_float(label="Kd", tag="kd_input", default_value=0.01, min_value=0.0, max_value=20.0, callback=update_plots)
        dpg.add_slider_float(label="Kff", tag="kff_input", default_value=0.0, min_value=0.0, max_value=20.0, callback=update_plots)
        dpg.add_slider_float(label="Kdf", tag="kdf_input", default_value=0.0, min_value=0.0, max_value=5.0, callback=update_plots)
        dpg.add_slider_float(label="Latency (s)", tag="latency_input", default_value=0.0, min_value=0.0, max_value=2.0, callback=update_plots)
        
        dpg.add_separator()
        dpg.add_text("System Info")
        dpg.add_text("Stable: ?", tag="info_stable")
        dpg.add_text("Settling Time: ?", tag="info_settling")
        dpg.add_text("Overshoot: ?", tag="info_overshoot")

        dpg.add_separator()
        dpg.add_text("System Parameters")
        dpg.add_text(f"Mass: {sim.mass}")
        dpg.add_text(f"Spring K: {sim.spring_k}")
        dpg.add_text(f"Damping B: {sim.damping_b}")

    with dpg.window(label="Plots", width=900, height=800, pos=(300, 0)):
        with dpg.plot(label="Step Response", height=200, width=-1):
            dpg.add_plot_legend()
            dpg.add_plot_axis(dpg.mvXAxis, label="Time (s)", tag="axis_x_step")
            with dpg.plot_axis(dpg.mvYAxis, label="Amplitude", tag="axis_y_step"):
                dpg.add_line_series([], [], label="Output", tag="series_output")
                dpg.add_line_series([], [], label="Setpoint", tag="series_setpoint")

        with dpg.plot(label="Control Signal", height=200, width=-1):
            dpg.add_plot_legend()
            dpg.add_plot_axis(dpg.mvXAxis, label="Time (s)", tag="axis_x_control")
            with dpg.plot_axis(dpg.mvYAxis, label="Control Output", tag="axis_y_control"):
                dpg.add_line_series([], [], label="Control Signal", tag="series_control")

        with dpg.plot(label="PID Terms", height=200, width=-1):
            dpg.add_plot_legend()
            dpg.add_plot_axis(dpg.mvXAxis, label="Time (s)", tag="axis_x_pid")
            with dpg.plot_axis(dpg.mvYAxis, label="Value", tag="axis_y_pid"):
                dpg.add_line_series([], [], label="P Term", tag="series_p")
                dpg.add_line_series([], [], label="I Term", tag="series_i")
                dpg.add_line_series([], [], label="D Term", tag="series_d")
                dpg.add_line_series([], [], label="FF Term", tag="series_ff")
                dpg.add_line_series([], [], label="DF Term", tag="series_df")

        with dpg.plot(label="Error", height=200, width=-1):
            dpg.add_plot_legend()
            dpg.add_plot_axis(dpg.mvXAxis, label="Time (s)", tag="axis_x_error")
            with dpg.plot_axis(dpg.mvYAxis, label="Error", tag="axis_y_error"):
                dpg.add_line_series([], [], label="Error", tag="series_error")

    update_plots(None, None, None)

    dpg.show_viewport()
    dpg.start_dearpygui()
    dpg.destroy_context()

if __name__ == "__main__":
    main()
