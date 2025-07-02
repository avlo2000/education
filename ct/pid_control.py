import control
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.widgets import Slider

from second_oreder_system import simulate_oreder2


time = np.linspace(0, 10, 300)
setpoint = 1.0


def update(_):
    controller = control.tf([s_p.val, s_d.val, s_i.val], [1, 0])
    yout, _ = simulate_oreder2(time, controller)
    plot.set_ydata(yout)
    ax.set_ylim([min(yout) - 0.1, max(yout) + 1])
    setpoint_line.set_ydata([1.0] * len(time))
    fig.canvas.draw_idle()


fig, ax = plt.subplots()
plt.subplots_adjust(left=0.1, bottom=0.35)
(plot,) = plt.plot(time, 0.0 * time, lw=2)
(setpoint_line,) = plt.plot(time, [setpoint] * len(time), "r--", lw=2)

ax_p = plt.axes((0.1, 0.25, 0.65, 0.03), facecolor="lightgoldenrodyellow")
ax_i = plt.axes((0.1, 0.2, 0.65, 0.03), facecolor="lightgoldenrodyellow")
ax_d = plt.axes((0.1, 0.15, 0.65, 0.03), facecolor="lightgoldenrodyellow")

s_p = Slider(ax_p, "P", 0.0, 1000.0, valinit=1.0)
s_i = Slider(ax_i, "I", 0.0, 100.0, valinit=0.1)
s_d = Slider(ax_d, "D", 0.0, 100.0, valinit=0.01)

s_p.on_changed(update)
s_i.on_changed(update)
s_d.on_changed(update)

plt.show()
