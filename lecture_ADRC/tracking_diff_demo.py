import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider
from tracking_diff import LPF, TrackingDiff

# --- Signal setup ---
dt = 0.01
T = 5.0
t = np.arange(0, T, dt)
signal = np.sin(2 * np.pi * 0.5 * t) + 0.5 * np.sin(2 * np.pi * 1.5 * t)
noise_std = 0.2
np.random.seed(42)
noisy_signal = signal + np.random.normal(0, noise_std, len(t))

# --- Initial parameters ---
init_cutoff = 2.0   # Hz
init_omega = 10.0
init_zeta = 0.707


def run_filters(cutoff, omega, zeta):
    lpf = LPF(dt, cutoff)
    td = TrackingDiff(dt, omega, zeta)
    lpf_out = []
    td_out = []
    for v in noisy_signal:
        lpf_out.append(lpf.filter(v))
        x1, _ = td.filter(v)
        td_out.append(x1)
    return np.array(lpf_out), np.array(td_out)


lpf_out, td_out = run_filters(init_cutoff, init_omega, init_zeta)

# --- Plot ---
fig, ax = plt.subplots(figsize=(10, 5))
plt.subplots_adjust(bottom=0.30)

(line_noisy,) = ax.plot(t, noisy_signal, color="lightgray", lw=0.8, label="Noisy signal")
(line_true,) = ax.plot(t, signal, color="black", lw=1.2, ls="--", label="True signal")
(line_lpf,) = ax.plot(t, lpf_out, color="steelblue", lw=1.5, label="LPF")
(line_td,) = ax.plot(t, td_out, color="tomato", lw=1.5, label="TrackingDiff (x₁)")

ax.set_xlabel("Time [s]")
ax.set_ylabel("Amplitude")
ax.set_title("LPF vs Tracking Differentiator")
ax.legend(loc="upper right")
ax.grid(True, alpha=0.3)

# --- Sliders ---
ax_cutoff = plt.axes([0.15, 0.20, 0.70, 0.03])
ax_omega  = plt.axes([0.15, 0.14, 0.70, 0.03])
ax_zeta   = plt.axes([0.15, 0.08, 0.70, 0.03])

sl_cutoff = Slider(ax_cutoff, "LPF cutoff [Hz]", 0.1, 20.0, valinit=init_cutoff, valstep=0.1)
sl_omega  = Slider(ax_omega,  "TD ω",            1.0, 50.0, valinit=init_omega,  valstep=0.5)
sl_zeta   = Slider(ax_zeta,   "TD ζ",            0.1,  2.0, valinit=init_zeta,   valstep=0.05)


def update(_):
    lpf_out, td_out = run_filters(sl_cutoff.val, sl_omega.val, sl_zeta.val)
    line_lpf.set_ydata(lpf_out)
    line_td.set_ydata(td_out)
    fig.canvas.draw_idle()


sl_cutoff.on_changed(update)
sl_omega.on_changed(update)
sl_zeta.on_changed(update)

plt.show()


