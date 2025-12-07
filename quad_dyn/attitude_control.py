from dataclasses import dataclass
import numpy as np
from pid import PID, PIDParams
from spatial import quat_conj, quat_mul
from scipy.spatial.transform import Rotation


@dataclass
class AttitudeControlParams:
    pitch_rate_pid: PIDParams
    roll_rate_pid: PIDParams
    yaw_rate_pid: PIDParams
    thrust_pid: PIDParams

    def __init__(self):
        self.pitch_rate_pid = PIDParams(P=0.05, D=0.0001, I=0.0, FF=0.0, D_FF=0.0, max_integral=10.0, min_output=-10.0, max_output=10.0)
        self.roll_rate_pid = PIDParams(P=0.05, D=0.0001, I=0.0, FF=0.0, D_FF=0.0, max_integral=10.0, min_output=-10.0, max_output=10.0)
        self.yaw_rate_pid = PIDParams(P=0.3, D=0.01, I=0.0, FF=0.0, D_FF=0.0, max_integral=10.0, min_output=-10.0, max_output=10.0)
        self.thrust_pid = PIDParams(P=10.0, D=0.01, I=0.01, FF=10.0, D_FF=0.0, max_integral=50.0, min_output=0.0, max_output=100.0)


class AttitudeControl:
    def __init__(self, params: AttitudeControlParams):
        self.pitch_rate_pid = PID(params.pitch_rate_pid)
        self.roll_rate_pid = PID(params.roll_rate_pid)
        self.yaw_rate_pid = PID(params.yaw_rate_pid)
        self.climb_rate_pid = PID(params.thrust_pid)

    def reset(self):
        self.pitch_rate_pid.reset()
        self.roll_rate_pid.reset()
        self.yaw_rate_pid.reset()
        self.climb_rate_pid.reset()

    def _rates2torques(self, 
                current_quat: np.ndarray,
                target_ang_vel: np.ndarray, 
                current_ang_vel: np.ndarray, 
                target_climb_rate: float,
                current_climb_rate: float, 
                dt: float
                ) -> tuple[float, float, float, float]:
        roll_torque = self.roll_rate_pid.update(
            target_ang_vel[0], current_ang_vel[0], dt
        )
        pitch_torque = self.pitch_rate_pid.update(
            target_ang_vel[1], current_ang_vel[1], dt
        )
        yaw_torque = self.yaw_rate_pid.update(
            target_ang_vel[2], current_ang_vel[2], dt
        )
        # Climb rate control (in world frame, z direction), thrust is in body frame

        climb_rate = self.climb_rate_pid.update(
            target_climb_rate, current_climb_rate, dt
        )
        rot = Rotation.from_quat(current_quat, scalar_first=True).as_matrix()
        z_body = rot[:, 2]
        thrust = climb_rate / z_body[2]  # Approximate thrust needed in body frame
        # print(f"thrust: {thrust}, climb_rate: {climb_rate}, z_body[2]: {z_body[2]}")
        return roll_torque, pitch_torque, yaw_torque, thrust

    def _torques2motor(self, roll_torque: float, pitch_torque: float, yaw_torque: float, thrust: float):
        C_d = 1.0  # Drag coefficient
        C_l = 1.0  # Lift coefficient
        d = 1.0   # Distance from center to motor
        
        F_des = thrust
        tau_x_des = roll_torque
        tau_y_des = pitch_torque
        tau_z_des = yaw_torque
        
        w0 = np.sqrt(max(0, (C_d*F_des*d - C_d*tau_x_des + C_d*tau_y_des + C_l*d*tau_z_des)/(C_d*C_l*d)))/2
        w1 = np.sqrt(max(0, (C_d*F_des*d - C_d*tau_x_des - C_d*tau_y_des - C_l*d*tau_z_des)/(C_d*C_l*d)))/2
        w2 = np.sqrt(max(0, (C_d*F_des*d + C_d*tau_x_des - C_d*tau_y_des + C_l*d*tau_z_des)/(C_d*C_l*d)))/2
        w3 = np.sqrt(max(0, (C_d*F_des*d + C_d*tau_x_des + C_d*tau_y_des - C_l*d*tau_z_des)/(C_d*C_l*d)))/2
        
        return np.array([w0, w1, w2, w3])

    def _attitude2rates(self, trg_quat: np.ndarray, cur_quat: np.ndarray) -> np.ndarray:
        cur_quat_conj = quat_conj(cur_quat, w_first=True)
        q_err = quat_mul(trg_quat, cur_quat_conj, w_first=True)
        angular_rates = 2.0 * q_err[1:4]
        return angular_rates

    def control(
        self,
        target_quat:  np.ndarray,
        current_quat: np.ndarray,
        current_ang_vel: np.ndarray,
        target_climb_rate: float, # in fact climb rate is velocity in z direction in WORLD FRAME
        current_climb_rate: float,
        dt: float
    ) -> np.ndarray:
        target_ang_vel = self._attitude2rates(target_quat, current_quat)
        roll_torque, pitch_torque, yaw_torque, thrust = self._rates2torques(
            current_quat,
            target_ang_vel,
            current_ang_vel,
            target_climb_rate,
            current_climb_rate,
            dt
        )
        motor_commands = self._torques2motor(roll_torque, pitch_torque, yaw_torque, thrust)
        return motor_commands
