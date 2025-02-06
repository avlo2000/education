import matplotlib.pyplot as plt
import numpy as np

from matplotlib.widgets import Button, Slider


def f(t, phase, frequency):
    return np.sin(2 * np.pi * frequency * t + phase)


ts = np.linspace(0, 1, 1000)

init_phase = 5
init_frequency = 3

# Fix subplot creation
fig = plt.figure(figsize=(10, 5))
gs = fig.add_gridspec(1, 2, width_ratios=[2, 1])
ax1 = fig.add_subplot(gs[0])
ax_polar = fig.add_subplot(gs[1], projection='polar')

signal, = ax1.plot(ts, f(ts, init_phase, init_frequency), lw=2)
trg_data = f(ts, init_phase, init_frequency)
ax1.plot(ts, trg_data, lw=2)

# Create complex signals for polar plot
complex_trg = np.exp(1j * (2 * np.pi * init_frequency * ts + init_phase))
complex_sig = complex_trg.copy()

# Initial polar plots
norm = plt.Normalize(0, len(ts))
polar_sig = ax_polar.scatter(np.angle(complex_sig), np.abs(complex_sig), 
                           c=np.arange(len(ts)), cmap='viridis', norm=norm)
ax_polar.set_ylim([0, 1.5])
ax_polar.legend(['Signal'])

# Add correlation text box
corr_text = ax1.text(0.02, 0.95, '', transform=ax1.transAxes)

ax1.set_xlabel('Time [s]')

# Adjust layout
fig.subplots_adjust(left=0.1, right=0.95, bottom=0.25)

axfreq = fig.add_axes([0.25, 0.1, 0.65, 0.03])
freq_slider = Slider(
    ax=axfreq,
    label='Frequency [Hz]',
    valmin=0.1,
    valmax=4.0,
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


def update(val):
    phase = np.pi * phase_slider.val / 180
    freq = freq_slider.val

    sig_data = f(ts, phase, freq)
    signal.set_ydata(sig_data)

    complex_corr = sig_data * np.exp(1j * (2 * np.pi * freq * ts + phase))
    polar_sig.set_offsets(np.c_[np.angle(complex_corr), np.abs(complex_corr)])
    
    correlation = np.corrcoef(sig_data,  np.exp(1j * (2 * np.pi * freq * ts + phase)))[0,1]
    corr_text.set_text(f'Correlation: {correlation:.3f}')


freq_slider.on_changed(update)
phase_slider.on_changed(update)

resetax = fig.add_axes([0.8, 0.025, 0.1, 0.04])
button = Button(resetax, 'Reset', hovercolor='0.975')


def reset(event):
    freq_slider.reset()
    phase_slider.reset()


button.on_clicked(reset)

plt.show()
