import numpy as np
import pygame
import sys

from sim_scene import SimScene
from generated_dynamics import A_state_nl, B_state_nl

# Permutation from [theta, theta_dot, x, x_dot] to [x, xdot, theta, thetadot]
_P = np.array([[0, 0, 1, 0],
               [0, 0, 0, 1],
               [1, 0, 0, 0],
               [0, 1, 0, 0]], dtype=float)
_PT = _P.T

class PendulumDynamics:
    def __init__(self, m=1.0, M=1.0, r=1.0, g=9.81, c=0.1, b=0.1):
        self.m = m
        self.M = M
        self.r = r
        self.g = g
        self.c = c
        self.b = b

    # state = [theta, theta_dot, x, x_dot]
    def dynamics(self, state: np.ndarray, u: float) -> np.ndarray:
        theta, theta_dot, _, x_dot = state

        lhs = np.array([
            [self.m + self.M, self.m * self.r * np.cos(theta)],
            [self.m * self.r * np.cos(theta), self.m * self.r**2]
        ])

        rhs = np.array([
            u - self.b * x_dot + self.m * self.r * (theta_dot**2) * np.sin(theta),
            -self.c * theta_dot + self.m * self.g * self.r * np.sin(theta)
        ])

        try:
            res = np.linalg.solve(lhs, rhs)
            x_ddot, theta_ddot = res[0], res[1]
        except np.linalg.LinAlgError:
            x_ddot, theta_ddot = 0.0, 0.0

        return np.array([theta_dot, theta_ddot, x_dot, x_ddot])

    def jac_A(self, state: np.ndarray, u: float = 0.0) -> np.ndarray:
        theta, theta_dot, _, x_dot = state
        A_nb = A_state_nl(theta, theta_dot, x_dot, u,
                          self.m, self.M, self.r, self.g, self.b, self.c)
        return _PT @ A_nb @ _P

    def jac_B(self, state: np.ndarray) -> np.ndarray:
        theta = state[0]
        B_nb = B_state_nl(theta, 0.0, 0.0, 0.0,
                          self.m, self.M, self.r, self.g, self.b, self.c)
        return _PT @ B_nb


def main():
    pygame.init()

    dt = 0.05
    pendulum = PendulumDynamics(m=1.0, M=5.0, r=2.0, g=9.81, c=1.1, b=10.5)

    width, height = 1000, 600
    screen = pygame.display.set_mode((width, height))
    pygame.display.set_caption("Pendulum Dynamics Simulation (Pygame)")
    clock = pygame.time.Clock()

    scene = SimScene(screen)

    running = True
    paused = False
    force_mag = 20.0
    state = np.array([0.0, 0.0, 0.0, 0.0])  # [theta, theta_dot, x, x_dot]
    state_lin = state.copy()

    A_lin = pendulum.jac_A(state)
    B_lin = pendulum.jac_B(state).flatten()

    def lin_dynamics(s, ctrl):
        return A_lin @ s + B_lin * ctrl

    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_SPACE:
                    paused = not paused
                elif event.key == pygame.K_r:
                    scene.reset()
                    state_lin = state.copy()

        keys = pygame.key.get_pressed()
        u = 0.0
        if keys[pygame.K_LEFT]:
            u = -force_mag
        if keys[pygame.K_RIGHT]:
            u = force_mag

        if not paused:
            state = state + dt * pendulum.dynamics(state, u)

            lk1 = lin_dynamics(state_lin, u)
            lk2 = lin_dynamics(state_lin + 0.5 * dt * lk1, u)
            lk3 = lin_dynamics(state_lin + 0.5 * dt * lk2, u)
            lk4 = lin_dynamics(state_lin + dt * lk3, u)
            state_lin = state_lin + (dt / 6.0) * (lk1 + 2 * lk2 + 2 * lk3 + lk4)

            scene.set_state(state)
            scene.set_linearized_state(state_lin)

        scene.render(u, paused)
        pygame.display.flip()
        clock.tick(60)

    pygame.quit()
    sys.exit()

if __name__ == "__main__":
    main()
