import numpy as np
import pygame
import sys

from sim_scene import SimScene
from pendulum_dynamics import PendulumDynamics


INITIAL_STATE = np.array([0.02, 0.0, 0.0, 0.2])

ANGLE_KP = 1.0
ANGLE_KI = 0.0
ANGLE_KD = 80.0
ANGLE_LIMITS = (-500.0, 500.0)

POS_KP = 0.8
POS_KI = 0.0
POS_KD = 2.0
POS_LIMITS = (-0.35, 0.35)

DT = 0.05


class PIDController:
    def __init__(self, kp: float, ki: float, kd: float,
                 setpoint: float = 0.0,
                 output_limits: tuple[float, float] = (-100.0, 100.0)):
        self.kp = kp
        self.ki = ki
        self.kd = kd
        self.setpoint = setpoint
        self.output_limits = output_limits
        self._integral = 0.0
        self._prev_error: float | None = None

    def reset(self):
        self._integral = 0.0
        self._prev_error = None

    def compute(self, measurement: float, dt: float) -> float:
        error = self.setpoint - measurement

        self._integral += error * dt
        ki_safe = max(abs(self.ki), 1e-6)
        self._integral = np.clip(
            self._integral,
            self.output_limits[0] / ki_safe,
            self.output_limits[1] / ki_safe,
        )

        derivative = 0.0 if self._prev_error is None else (error - self._prev_error) / dt
        self._prev_error = error

        output = self.kp * error + self.ki * self._integral + self.kd * derivative
        return float(np.clip(output, *self.output_limits))


def main():
    pygame.init()

    pendulum = PendulumDynamics(m=0.1, M=0.4, r=2.0, g=9.81, c=0.1, b=10.5)

    angle_pid = PIDController(kp=ANGLE_KP, ki=ANGLE_KI, kd=ANGLE_KD,
                              setpoint=0.0, output_limits=ANGLE_LIMITS)
    pos_pid = PIDController(kp=POS_KP, ki=POS_KI, kd=POS_KD,
                            setpoint=0.0, output_limits=POS_LIMITS)

    width, height = 1000, 600
    screen = pygame.display.set_mode((width, height))
    pygame.display.set_caption("Pendulum PID Controller")
    clock = pygame.time.Clock()

    scene = SimScene(screen)
    state = INITIAL_STATE.copy()
    scene.set_state(state)

    running = True
    u_ctrl = 0.0

    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

        angle_setpoint = -pos_pid.compute(state[2], DT)
        angle_pid.setpoint = angle_setpoint
        u_ctrl = angle_pid.compute(state[0], DT)

        k1 = pendulum.dynamics(state, u_ctrl)
        k2 = pendulum.dynamics(state + 0.5 * DT * k1, u_ctrl)
        k3 = pendulum.dynamics(state + 0.5 * DT * k2, u_ctrl)
        k4 = pendulum.dynamics(state + DT * k3, u_ctrl)
        state_dot = (DT / 6.0) * (k1 + 2 * k2 + 2 * k3 + k4)
        state = state + DT * state_dot

        scene.set_state(state)
        scene.render(u_ctrl, False)
        pygame.display.flip()
        clock.tick(60)

    pygame.quit()
    sys.exit()


if __name__ == "__main__":
    main()
