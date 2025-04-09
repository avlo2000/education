import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider


def signal_x(freq: float, phase: float, damping: float, t: np.ndarray) -> np.ndarray:
    return np.exp(-damping * t + 1j*(2*np.pi*freq*t + phase))


init_freq = 5.0
init_phase = 0.0
init_damping = 1.0

t = np.linspace(0, 5, 1000)
x0 = signal_x(init_freq, init_phase, init_damping, t)

fig = plt.figure(figsize=(12, 8))
ax_signal = fig.add_subplot(211)

line_signal, = ax_signal.plot(t, x0, lw=2)
ax_signal.set_xlabel("Time (s)")
ax_signal.set_ylabel("Amplitude")
ax_signal.set_title("Real-valued Signal")
text_signal = ax_signal.text(0.05, 0.95, f"{-init_damping:.2f} + i{2*np.pi*init_freq:.2f}", transform=ax_signal.transAxes, verticalalignment='top')

sigmas = np.linspace(-50, 50, 50)
omegas = np.linspace(-50, 50, 50)

axcolor = 'lightgoldenrodyellow'
axfreq = plt.axes([0.15, 0.02, 0.65, 0.03], facecolor=axcolor)
axphase = plt.axes([0.15, 0.06, 0.65, 0.03], facecolor=axcolor)
axdamp = plt.axes([0.15, 0.10, 0.65, 0.03], facecolor=axcolor)

freq_slider = Slider(axfreq, 'Freq', 0.1, 10.0, valinit=init_freq)
phase_slider = Slider(axphase, 'Phase', -np.pi, np.pi, valinit=init_phase)
damping_slider = Slider(axdamp, 'Damping', -5.0, 5.0, valinit=init_damping)

def update_plots(val):
    freq = freq_slider.val
    phase = phase_slider.val
    damping = damping_slider.val
    x = signal_x(freq, phase, damping, t)
    line_signal.set_ydata(x)
    text_signal.set_text(f"{-damping:.2f} + i{2*np.pi*freq:.2f}")
    ax_signal.relim()
    ax_signal.autoscale_view()
    fig.canvas.draw_idle()

freq_slider.on_changed(update_plots)
phase_slider.on_changed(update_plots)
damping_slider.on_changed(update_plots)

plt.tight_layout()
plt.show()
