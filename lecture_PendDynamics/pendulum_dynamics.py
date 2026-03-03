import numpy as np
import pygame
import sys

from sim_scene import SimScene

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


def main():
    pygame.init()

    dt = 0.1
    pendulum = PendulumDynamics(m=1.0, M=5.0, r=2.0, g=9.81, c=0.1, b=10.5)

    width, height = 1000, 600
    screen = pygame.display.set_mode((width, height))
    pygame.display.set_caption("Pendulum Dynamics Simulation (Pygame)")
    clock = pygame.time.Clock()

    scene = SimScene(screen)

    running = True
    paused = False
    force_mag = 20.0
    state = np.array([0.0, 0.0, 0.0, 0.0])  # [theta, theta_dot, x, x_dot]
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_SPACE:
                    paused = not paused
                elif event.key == pygame.K_r:
                    scene.reset()

        keys = pygame.key.get_pressed()
        u = 0.0
        if keys[pygame.K_LEFT]:
            u = -force_mag
        if keys[pygame.K_RIGHT]:
            u = force_mag

        if not paused:
            state_dot = pendulum.dynamics(state, u)
            # k1 = pendulum.dynamics(state, u)
            # k2 = pendulum.dynamics(state + 0.5 * dt * k1, u)
            # k3 = pendulum.dynamics(state + 0.5 * dt * k2, u)
            # k4 = pendulum.dynamics(state + dt * k3, u)
            # state_dot = (dt / 6.0) * (k1 + 2 * k2 + 2 * k3 + k4)
            state = state + state_dot
            scene.set_state(state)

        scene.render(u, paused)
        pygame.display.flip()
        clock.tick(60)

    pygame.quit()
    sys.exit()

if __name__ == "__main__":
    main()
