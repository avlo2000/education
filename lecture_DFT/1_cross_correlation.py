import matplotlib.pyplot as plt
import numpy as np

from matplotlib.widgets import Button, Slider


def f(t, phase, frequency):
    return np.sin(2 * np.pi * frequency * t + phase)


ts = np.linspace(0, 10, 1000)

init_phase = 5
init_frequency = 3

fig = plt.figure(figsize=(10, 7))
gs = fig.add_gridspec(2, 1, height_ratios=[3, 2])
ax1 = fig.add_subplot(gs[0])
ax_prod = fig.add_subplot(gs[1])

signal, = ax1.plot(ts, f(ts, init_phase, init_frequency), lw=2)
trg_data = f(ts, init_phase, init_frequency)
ax1.plot(ts, trg_data, lw=2)

complex_trg = np.exp(1j * (2 * np.pi * init_frequency * ts + init_phase))
complex_sig = complex_trg.copy()

corr_text = ax1.text(0.9, 1.05, '', transform=ax1.transAxes)
complex_corr_text = ax1.text(0.04, 1.05, '', transform=ax1.transAxes)

ax1.set_xlabel('Time [s]')

ax_prod.set_xlabel('Time [s]')
ax_prod.set_ylabel('Product')
ax_prod.set_ylim(-2.1, 2.1)

init_phase_rad = np.pi * init_phase / 180
prod = f(ts, init_phase_rad, init_frequency) * f(ts, init_phase_rad, init_frequency)
prod_line, = ax_prod.plot(ts, prod, lw=2, color='green')

fig.subplots_adjust(left=0.1, right=0.95, bottom=0.25)

axfreq = fig.add_axes([0.25, 0.1, 0.65, 0.03])
freq_slider = Slider(
    ax=axfreq,
    label='Frequency [Hz]',
    valmin=0.1,
    valmax=8.0,
    valinit=init_frequency,
)

axamp = fig.add_axes([0.1, 0.25, 0.0225, 0.63])
phase_slider = Slider(
    ax=axamp,
    label="Phase",
    valmin=0,
    valmax=360,
    valinit=init_phase,
    orientation="vertical"
)


def update(_):
    phase = np.pi * phase_slider.val / 180
    freq = freq_slider.val

    sig_data = f(ts, phase, freq)
    signal.set_ydata(sig_data)

    trj_signal = f(ts, init_phase, init_frequency)
    prod_vals = sig_data * trj_signal
    cov = np.sum(prod_vals)
    corr_text.set_text(f'Correlation : {cov:.3f}')
    prod_line.set_ydata(prod_vals)


freq_slider.on_changed(update)
phase_slider.on_changed(update)

resetax = fig.add_axes([0.8, 0.025, 0.1, 0.04])
button = Button(resetax, 'Reset', hovercolor='0.975')


def reset(_):
    freq_slider.reset()
    phase_slider.reset()


button.on_clicked(reset)

plt.show()
