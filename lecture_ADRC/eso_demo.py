from eso import LinearESO2

from matplotlib import pyplot as plt
from matplotlib.widgets import Slider
import numpy as np

# Pendulum parameters (plant the ESO does NOT know about)
G = 9.81      # gravity [m/s^2]
L = 1.0       # length [m]
C = 0.2       # viscous damping coefficient


def simulate(b0, wo, t, u, d_ext, dt):
    """
    Simulate a damped pendulum driven by control u and external disturbance d_ext:

        theta_ddot = -(g/L) sin(theta) - c * theta_dot + b0 * u + d_ext

    The ESO sees only the output theta and the control u. Everything other than
    b0 * u (gravity, damping, external disturbance) is the "total disturbance" f
    that z3 should estimate:

        f = -(g/L) sin(theta) - c * theta_dot + d_ext
    """
    eso = LinearESO2(b0, wo)

    theta = np.zeros_like(t)
    theta_dot = np.zeros_like(t)
    f_true = np.zeros_like(t)

    z1_est = np.zeros_like(t)
    z2_est = np.zeros_like(t)
    z3_est = np.zeros_like(t)

    th = 0.0
    th_dot = 0.0

    for i in range(len(t)):
        f = -(G / L) * np.sin(th) - C * th_dot + d_ext[i]
        th_ddot = f + b0 * u[i]

        theta[i] = th
        theta_dot[i] = th_dot
        f_true[i] = f

        # ESO observes only the output and the control input
        z1_est[i], z2_est[i], z3_est[i] = eso.update(th, u[i], dt)

        # Integrate the pendulum (explicit Euler)
        th += dt * th_dot
        th_dot += dt * th_ddot

    return theta, theta_dot, f_true, z1_est, z2_est, z3_est


def main():
    b0_init = 1.0
    wo_init = 10.0
    dt = 0.01
    t_final = 10.0

    t = np.arange(0, t_final, dt)

    u = np.zeros_like(t)
    u[t > 1.0] = 5.0
    u[t > 5.0] = 0.0
    d_ext = 0.5 * np.sin(2 * np.pi * 0.5 * t)

    theta, theta_dot, f_true, z1_est, z2_est, z3_est = simulate(
        b0_init, wo_init, t, u, d_ext, dt
    )

    fig, axes = plt.subplots(3, 1, figsize=(12, 9))
    plt.subplots_adjust(bottom=0.18, hspace=0.5)

    ax1, ax2, ax3 = axes

    line_theta, = ax1.plot(t, theta, label='True Output theta')
    line_z1, = ax1.plot(t, z1_est, label='Estimated Output z1', linestyle='--')
    ax1.set_title('Output Estimation')
    ax1.set_xlabel('Time [s]')
    ax1.set_ylabel('Angle [rad]')
    ax1.legend()

    line_thetadot, = ax2.plot(t, theta_dot, label='True Derivative theta_dot')
    line_z2, = ax2.plot(t, z2_est, label='Estimated Derivative z2', linestyle='--')
    ax2.set_title('Derivative Estimation')
    ax2.set_xlabel('Time [s]')
    ax2.set_ylabel('Rate [rad/s]')
    ax2.legend()

    line_z3, = ax3.plot(t, z3_est, label='Estimated Disturbance z3', linestyle='--')
    line_ftrue, = ax3.plot(t, f_true, label='True Total Disturbance f', linestyle=':')
    ax3.set_title('Total Disturbance Estimation')
    ax3.set_xlabel('Time [s]')
    ax3.set_ylabel('Disturbance')
    ax3.legend()

    ax_b0 = plt.axes([0.15, 0.07, 0.7, 0.03])
    ax_wo = plt.axes([0.15, 0.02, 0.7, 0.03])
    slider_b0 = Slider(ax_b0, 'b0', 0.1, 5.0, valinit=b0_init)
    slider_wo = Slider(ax_wo, 'wo', 1.0, 500.0, valinit=wo_init)

    def update(_):
        b0 = slider_b0.val
        wo = slider_wo.val
        theta, theta_dot, f_true, z1_est, z2_est, z3_est = simulate(
            b0, wo, t, u, d_ext, dt
        )

        line_theta.set_ydata(theta)
        line_z1.set_ydata(z1_est)
        line_thetadot.set_ydata(theta_dot)
        line_z2.set_ydata(z2_est)
        line_z3.set_ydata(z3_est)
        line_ftrue.set_ydata(f_true)

        for ax in axes:
            ax.relim()
            ax.autoscale_view()

        fig.canvas.draw_idle()

    slider_b0.on_changed(update)
    slider_wo.on_changed(update)

    plt.show()


if __name__ == "__main__":
    main()
