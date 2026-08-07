"""
Standalone demo: ESO vs. a classical (Luenberger) observer under disturbance.

Both observers watch the SAME plant

    y_ddot = b0 * u + f(t)

and are tuned to the SAME bandwidth wo. The only difference is their model:

  * The classical observer assumes the nominal model y_ddot = b0 * u.
    It has no way to account for f(t), so a disturbance produces a persistent
    estimation bias:
        steady-state position error  = f / wo^2
        steady-state velocity error  = 2 * f / wo

  * The ESO augments the state with an estimate of the total disturbance f,
    which lets it drive the state-estimation error back to zero even while a
    disturbance is acting.

Drag the sliders to change the observer bandwidth wo and the disturbance
amplitude and watch the classical estimate drift while the ESO tracks the truth.
"""

from eso import LinearESO2, ClassicalObserver2

from matplotlib import pyplot as plt
from matplotlib.widgets import Slider
import numpy as np


def simulate(b0, wo, dist_amp, t, u, dt):
    """
    Integrate the true plant and run both observers on the same measurement.

    Disturbance: piecewise-constant steps. A constant disturbance is the clearest
    case: the classical observer settles to a fixed bias (f/wo^2 in position,
    2*f/wo in velocity), while the ESO drives the error back to zero.
    """
    f_true = dist_amp * ((t > 2.0).astype(float) + 0.6 * (t > 7.0).astype(float))

    eso = LinearESO2(b0, wo)
    lobs = ClassicalObserver2(b0, wo)

    x1 = np.zeros_like(t)  # true position (output)
    x2 = np.zeros_like(t)  # true velocity

    c1 = np.zeros_like(t)  # classical position estimate
    c2 = np.zeros_like(t)  # classical velocity estimate

    z1 = np.zeros_like(t)  # ESO position estimate
    z2 = np.zeros_like(t)  # ESO velocity estimate
    z3 = np.zeros_like(t)  # ESO disturbance estimate

    pos = 0.0
    vel = 0.0

    for i in range(len(t)):
        x1[i] = pos
        x2[i] = vel

        # Record each observer's current estimate (aligned to time t[i]) before
        # advancing it, so estimates and true states share the same time index.
        c1[i], c2[i] = lobs.x1, lobs.x2
        z1[i], z2[i], z3[i] = eso.z1, eso.z2, eso.z3

        # Both observers see only the output (pos) and the control input
        lobs.update(pos, u[i], dt)
        eso.update(pos, u[i], dt)

        # Integrate true plant: y_ddot = b0 * u + f  (explicit Euler)
        acc = b0 * u[i] + f_true[i]
        pos += dt * vel
        vel += dt * acc

    return x1, x2, f_true, c1, c2, z1, z2, z3


def main():
    b0 = 1.0
    wo_init = 8.0
    dist_amp_init = 1.0
    dt = 0.005
    t_final = 12.0

    t = np.arange(0, t_final, dt)

    # Mild control input so the states move; the disturbance is the star here.
    u = np.zeros_like(t)
    u[t > 1.0] = 1.0
    u[t > 6.0] = -1.0

    x1, x2, f_true, c1, c2, z1, z2, z3 = simulate(
        b0, wo_init, dist_amp_init, t, u, dt
    )

    fig, axes = plt.subplots(3, 1, figsize=(12, 9), sharex=True)
    plt.subplots_adjust(bottom=0.2, hspace=0.35)
    ax1, ax2, ax3 = axes

    l_x1, = ax1.plot(t, x1, 'k', label='True position')
    l_c1, = ax1.plot(t, c1, 'r--', label='Classical estimate')
    l_z1, = ax1.plot(t, z1, 'g--', label='ESO estimate')
    ax1.set_title('Position Estimation')
    ax1.set_ylabel('Position')
    ax1.legend(loc='upper right')

    l_x2, = ax2.plot(t, x2, 'k', label='True velocity')
    l_c2, = ax2.plot(t, c2, 'r--', label='Classical estimate')
    l_z2, = ax2.plot(t, z2, 'g--', label='ESO estimate')
    ax2.set_title('Velocity Estimation')
    ax2.set_ylabel('Velocity')
    ax2.legend(loc='upper right')

    l_f, = ax3.plot(t, f_true, 'k', label='True disturbance f')
    l_z3, = ax3.plot(t, z3, 'g--', label='ESO disturbance estimate z3')
    ax3.axhline(0.0, color='r', linestyle=':', label='Classical (no disturbance model)')
    ax3.set_title('Disturbance Estimation')
    ax3.set_xlabel('Time [s]')
    ax3.set_ylabel('Disturbance')
    ax3.legend(loc='upper right')

    ax_wo = plt.axes([0.15, 0.08, 0.7, 0.03])
    ax_amp = plt.axes([0.15, 0.03, 0.7, 0.03])
    slider_wo = Slider(ax_wo, 'wo', 1.0, 30.0, valinit=wo_init)
    slider_amp = Slider(ax_amp, 'disturbance', 0.0, 3.0, valinit=dist_amp_init)

    def update(_):
        wo = slider_wo.val
        amp = slider_amp.val
        x1, x2, f_true, c1, c2, z1, z2, z3 = simulate(b0, wo, amp, t, u, dt)

        l_x1.set_ydata(x1)
        l_c1.set_ydata(c1)
        l_z1.set_ydata(z1)
        l_x2.set_ydata(x2)
        l_c2.set_ydata(c2)
        l_z2.set_ydata(z2)
        l_f.set_ydata(f_true)
        l_z3.set_ydata(z3)

        for ax in axes:
            ax.relim()
            ax.autoscale_view()

        fig.canvas.draw_idle()

    slider_wo.on_changed(update)
    slider_amp.on_changed(update)

    plt.show()


if __name__ == "__main__":
    main()
