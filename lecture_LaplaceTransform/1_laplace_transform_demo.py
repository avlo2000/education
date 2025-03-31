import numpy as np
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider
from mpl_toolkits.mplot3d import Axes3D


def signal_x(freq: float, phase: float, damping: float, t: np.ndarray) -> np.ndarray:
    t = np.linspace(0, 1, 1000)
    return np.exp(-damping * t) * np.sin(2 * np.pi * freq * t + phase)


def compute_laplace(signal, t, sigmas, omegas):
    s = sigmas[None, :] + 1j * omegas[:, None]
    exp_term = np.exp(-s[..., None] * t[None, None, :])
    integrand = signal[None, None, :] * exp_term
    return np.trapz(integrand, t, axis=2)


init_freq = 5.0
init_phase = 0.0
init_damping = 1.0

t = np.linspace(0, 5, 1000)
x0 = signal_x(init_freq, init_phase, init_damping, t)

fig = plt.figure(figsize=(12, 8))
ax_signal = fig.add_subplot(211)
ax_3d = fig.add_subplot(212, projection='3d')

line_signal, = ax_signal.plot(t, x0, lw=2)
ax_signal.set_xlabel("Time (s)")
ax_signal.set_ylabel("Amplitude")
ax_signal.set_title("Real-valued Signal")

sigmas = np.linspace(-50, 50, 50)
omegas = np.linspace(-50, 50, 50)
L0 = compute_laplace(x0, t, sigmas, omegas)
X, Y = np.meshgrid(sigmas, omegas)
surface = ax_3d.plot_surface(X, Y, np.abs(L0), cmap='viridis')
ax_3d.set_xlabel("Sigma")
ax_3d.set_ylabel("Omega")
ax_3d.set_zlabel("|Laplace|")
ax_3d.set_title("Laplace Transform Magnitude")

# Create sliders for freq, phase, and damping
axcolor = 'lightgoldenrodyellow'
axfreq = plt.axes([0.15, 0.02, 0.65, 0.03], facecolor=axcolor)
axphase = plt.axes([0.15, 0.06, 0.65, 0.03], facecolor=axcolor)
axdamp = plt.axes([0.15, 0.10, 0.65, 0.03], facecolor=axcolor)

freq_slider = Slider(axfreq, 'Freq', 0.1, 10.0, valinit=init_freq)
phase_slider = Slider(axphase, 'Phase', -np.pi, np.pi, valinit=init_phase)
damping_slider = Slider(axdamp, 'Damping', 0.0, 5.0, valinit=init_damping)

def update_plots(val):
    freq = freq_slider.val
    phase = phase_slider.val
    damping = damping_slider.val
    
    x = signal_x(freq, phase, damping, t)
    line_signal.set_ydata(x)
    
    L = compute_laplace(x, t, sigmas, omegas)
    ax_3d.clear()
    X, Y = np.meshgrid(sigmas, omegas)
    ax_3d.plot_surface(X, Y, np.abs(L), cmap='viridis')
    ax_3d.set_xlabel("Sigma")
    ax_3d.set_ylabel("Omega")
    ax_3d.set_zlabel("|Laplace|")
    ax_3d.set_title("Laplace Transform Magnitude")
    
    fig.canvas.draw_idle()

freq_slider.on_changed(update_plots)
phase_slider.on_changed(update_plots)
damping_slider.on_changed(update_plots)

plt.tight_layout()
plt.show()
