"""
Standalone demo: ADRC vs. classical PID on a disturbed pendulum.

Plant (same for both controllers, driven independently):

    theta_ddot = b0 * u + f
    f = -(g/L) * sin(theta) - c * theta_dot + d_ext(t)

The controllers only measure theta and command u. Everything else -- the
nonlinear gravity term, the damping, and an external torque disturbance d_ext
that switches on partway through -- is unknown to them.

  * PID reacts only to the tracking error, so it must integrate the error away
    after each disturbance/setpoint change, which costs overshoot and settling
    time.

  * ADRC estimates the total disturbance f with its ESO and cancels it directly,
    so it rejects the same disturbance with far less error.

Drag the sliders to retune each controller and change the disturbance amplitude.
"""

"""
Betaflight demo
https://www.youtube.com/watch?v=fxuEPyDjAdE
"""


import numpy as np
from matplotlib import pyplot as plt
from matplotlib.widgets import Slider

from adrc import ADRC

# Pendulum parameters (unknown to the controllers)
G = 9.81
L = 1.0
C = 0.3
B0 = 1.0


class PID:
    """Discrete PID with derivative-on-measurement and anti-windup clamping."""

    def __init__(self, kp, ki, kd, dt, u_min=None, u_max=None, tau_d=0.02):
        self.kp = kp
        self.ki = ki
        self.kd = kd
        self.dt = dt
        self.u_min = u_min
        self.u_max = u_max
        # Derivative low-pass coefficient (first-order filter on the derivative).
        self.alpha = dt / (dt + tau_d)

        self.integral = 0.0
        self.prev_meas = 0.0
        self.d_filt = 0.0
        self.initialized = False

    def update(self, setpoint, y):
        e = setpoint - y

        if not self.initialized:
            self.prev_meas = y
            self.initialized = True

        # Derivative on measurement (avoids derivative kick on setpoint steps).
        d_meas = (y - self.prev_meas) / self.dt
        self.d_filt += self.alpha * (d_meas - self.d_filt)
        self.prev_meas = y

        u_unsat = self.kp * e + self.ki * self.integral - self.kd * self.d_filt

        u = u_unsat
        saturated = False
        if self.u_max is not None and u > self.u_max:
            u = self.u_max
            saturated = True
        if self.u_min is not None and u < self.u_min:
            u = self.u_min
            saturated = True

        # Conditional integration: stop winding up while saturated and pushing
        # further into the limit.
        if not (saturated and np.sign(e) == np.sign(u_unsat)):
            self.integral += e * self.dt

        return u


def simulate(t, setpoint, dt, dist_amp, wo, wc, kp, ki, kd, u_lim=30.0):
    d_ext = dist_amp * (t > 3.0).astype(float)

    pid = PID(kp, ki, kd, dt, u_min=-u_lim, u_max=u_lim)
    adrc = ADRC(b0=B0, wo=wo, wc=wc, dt=dt, u_min=-u_lim, u_max=u_lim)

    y_pid = np.zeros_like(t)
    u_pid = np.zeros_like(t)
    y_adrc = np.zeros_like(t)
    u_adrc = np.zeros_like(t)
    z3 = np.zeros_like(t)

    th_p, thd_p = 0.0, 0.0
    th_a, thd_a = 0.0, 0.0

    for i in range(len(t)):
        y_pid[i] = th_p
        y_adrc[i] = th_a

        up = pid.update(setpoint[i], th_p)
        ua = adrc.update(setpoint[i], th_a)
        u_pid[i] = up
        u_adrc[i] = ua
        z3[i] = adrc.z3

        # Advance both pendulums (explicit Euler).
        fp = -(G / L) * np.sin(th_p) - C * thd_p + d_ext[i]
        fa = -(G / L) * np.sin(th_a) - C * thd_a + d_ext[i]
        acc_p = B0 * up + fp
        acc_a = B0 * ua + fa
        th_p += dt * thd_p
        thd_p += dt * acc_p
        th_a += dt * thd_a
        thd_a += dt * acc_a

    return y_pid, u_pid, y_adrc, u_adrc, z3


def main():
    dt = 0.005
    t_final = 8.0
    t = np.arange(0, t_final, dt)

    setpoint = np.zeros_like(t)
    setpoint[t > 0.5] = 0.5
    setpoint[t > 5.0] = -0.3

    init = dict(dist_amp=3.0, wo=25.0, wc=5.0, kp=30.0, ki=20.0, kd=8.0)

    y_pid, u_pid, y_adrc, u_adrc, z3 = simulate(t, setpoint, dt, **init)

    fig, (ax_y, ax_u) = plt.subplots(2, 1, figsize=(12, 8), sharex=True)
    plt.subplots_adjust(bottom=0.32, hspace=0.25)

    ax_y.plot(t, setpoint, 'k:', label='Setpoint')
    l_ypid, = ax_y.plot(t, y_pid, 'r', label='PID')
    l_yadrc, = ax_y.plot(t, y_adrc, 'g', label='ADRC')
    l_dist = ax_y.axvline(3.0, color='gray', ls='--', alpha=0.6)
    ax_y.set_ylabel('Angle [rad]')
    ax_y.set_title('Output tracking (disturbance switches on at t = 3 s)')
    ax_y.legend(loc='upper right')
    ax_y.grid(True, alpha=0.3)

    l_upid, = ax_u.plot(t, u_pid, 'r', label='PID')
    l_uadrc, = ax_u.plot(t, u_adrc, 'g', label='ADRC')
    ax_u.set_xlabel('Time [s]')
    ax_u.set_ylabel('Control u')
    ax_u.set_title('Control effort')
    ax_u.legend(loc='upper right')
    ax_u.grid(True, alpha=0.3)

    # Sliders: PID gains on the left, ADRC + disturbance on the right.
    def slider_axes(x, y):
        return plt.axes([x, y, 0.32, 0.03])

    sl_kp = Slider(slider_axes(0.08, 0.20), 'PID kp', 0.0, 100.0, valinit=init['kp'])
    sl_ki = Slider(slider_axes(0.08, 0.15), 'PID ki', 0.0, 100.0, valinit=init['ki'])
    sl_kd = Slider(slider_axes(0.08, 0.10), 'PID kd', 0.0, 40.0, valinit=init['kd'])

    sl_wc = Slider(slider_axes(0.58, 0.20), 'ADRC wc', 1.0, 20.0, valinit=init['wc'])
    sl_wo = Slider(slider_axes(0.58, 0.15), 'ADRC wo', 5.0, 80.0, valinit=init['wo'])
    sl_amp = Slider(slider_axes(0.58, 0.10), 'disturbance', 0.0, 8.0, valinit=init['dist_amp'])

    def update(_):
        y_pid, u_pid, y_adrc, u_adrc, z3 = simulate(
            t, setpoint, dt,
            dist_amp=sl_amp.val, wo=sl_wo.val, wc=sl_wc.val,
            kp=sl_kp.val, ki=sl_ki.val, kd=sl_kd.val,
        )
        l_ypid.set_ydata(y_pid)
        l_yadrc.set_ydata(y_adrc)
        l_upid.set_ydata(u_pid)
        l_uadrc.set_ydata(u_adrc)

        for ax in (ax_y, ax_u):
            ax.relim()
            ax.autoscale_view()
        fig.canvas.draw_idle()

    for s in (sl_kp, sl_ki, sl_kd, sl_wc, sl_wo, sl_amp):
        s.on_changed(update)

    plt.show()


if __name__ == "__main__":
    main()
