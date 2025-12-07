from dataclasses import dataclass


@dataclass
class PIDParams:
    P: float = 1.0
    D: float = 0.0
    I: float = 0.0
    FF: float = 0.0 # Feedforward term
    D_FF: float = 0.0 # Derivative feedforward term
    max_integral: float = float('inf')
    min_output: float = -float('inf')
    max_output: float = float('inf')


class PID:
    def __init__(self, params: PIDParams):
        self.params = params
        self.integral = 0.0
        self.prev_error = 0.0

    def reset(self):
        self.integral = 0.0
        self.prev_error = 0.0

    def update(self, target: float, current: float, dt: float, target_derivative: float = 0.0) -> float:
        error = target - current

        p_term = self.params.P * error
        self.integral += error * dt

        if self.params.max_integral > 0:
            self.integral = max(min(self.integral, self.params.max_integral), -self.params.max_integral)
        i_term = self.params.I * self.integral

        derivative = (error - self.prev_error) / dt if dt > 0 else 0.0
        d_term = self.params.D * derivative
        
        self.prev_error = error
        ff_term = self.params.FF * target + self.params.D_FF * target_derivative
        
        output = p_term + i_term + d_term + ff_term

        output = max(min(output, self.params.max_output), self.params.min_output)
        
        return output
