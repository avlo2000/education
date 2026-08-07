import numpy as np


class LPF:
    def __init__(self, dt, cutoff_freq):
        self.dt = dt
        self.cutoff_freq = cutoff_freq
        self.alpha = self.compute_alpha(dt, cutoff_freq)
        self.prev_output = 0.0

    def compute_alpha(self, dt, cutoff_freq):
        rc = 1.0 / (2 * np.pi * cutoff_freq)
        return dt / (dt + rc)

    def filter(self, input_signal):
        output = self.alpha * input_signal + (1 - self.alpha) * self.prev_output
        self.prev_output = output
        return output


class TrackingDiff:
    def __init__(self, dt, omega, zeta):
        self.dt = dt
        self.omega = omega
        self.zeta = zeta
        self.x1 = 0.0
        self.x2 = 0.0

    def filter(self, v):
        x1_dot = self.x2
        x2_dot = -2 * self.zeta * self.omega * self.x2 + self.omega**2 * (v - self.x1)

        self.x1 += self.dt * x1_dot
        self.x2 += self.dt * x2_dot

        return self.x1, self.x2
