import matplotlib.pyplot as plt
import numpy as np

from matplotlib.widgets import Button, Slider


def f(t, phase, frequency):
    return np.sin(2 * np.pi * frequency * t + phase)


ts = np.linspace(0, 1, 1000)

init_phase = 5
init_frequency = 3

# Create subplots: top for signal, bottom for product signal
fig = plt.figure(figsize=(10, 7))
gs = fig.add_gridspec(2, 1, height_ratios=[3, 2])
ax1 = fig.add_subplot(gs[0])
ax_prod = fig.add_subplot(gs[1])

# Plot signal on top subplot
signal, = ax1.plot(ts, f(ts, init_phase, init_frequency), lw=2, label='Signal')
trg_data = f(ts, init_phase, init_frequency)
ax1.plot(ts, trg_data, lw=2, label='Target Signal')
ax1.legend()  # add legend for top subplot

# Add correlation text box
corr_text = ax1.text(0.14, 1.1, '', transform=ax1.transAxes)
complex_corr_text = ax1.text(0.14, 1.2, '', transform=ax1.transAxes)

ax1.set_xlabel('Time [s]')

# Create initial product signal (plot real and imag parts) on bottom subplot
trj_signal = np.exp(1j * (2 * np.pi * init_frequency * ts + init_phase))
init_sig = f(ts, init_phase, init_frequency)
prod_signal = init_sig * trj_signal
prod_line, = ax_prod.plot(ts, np.real(prod_signal), lw=2, color='green', label='Real Part')
imag_line, = ax_prod.plot(ts, np.imag(prod_signal), lw=2, color='red', label='Imag Part')  # new imag_line
ax_prod.legend()  # add legend for bottom subplot
ax_prod.set_xlabel('Time [s]')
ax_prod.set_ylabel('Signal')
ax_prod.set_ylim(-1, 1)  # set y-axis range

# Adjust layout
fig.subplots_adjust(left=0.1, right=0.95, bottom=0.25)

axfreq = fig.add_axes([0.25, 0.1, 0.65, 0.03])
freq_slider = Slider(
    ax=axfreq,
    label='Frequency [Hz]',
    valmin=0.1,
    valmax=8.0,
    valinit=init_frequency,
)

axamp = fig.add_axes([0.05, 0.25, 0.0225, 0.63])
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

    trj_signal = np.exp(1j * (2 * np.pi * init_frequency * ts + init_phase))
    prod_signal = sig_data * trj_signal
    complex_corr = np.sum(prod_signal)
    corr_text.set_text(f'Correlation : {abs(complex_corr):.3f}')
    complex_corr_text.set_text(f'Complex corr : {complex_corr:.3f}')
    
    # Update the product subplot with the real and imag parts of prod_signal
    prod_line.set_ydata(np.real(prod_signal))
    imag_line.set_ydata(np.imag(prod_signal))


freq_slider.on_changed(update)
phase_slider.on_changed(update)

resetax = fig.add_axes([0.8, 0.025, 0.1, 0.04])
button = Button(resetax, 'Reset', hovercolor='0.975')


def reset(event):
    freq_slider.reset()
    phase_slider.reset()


button.on_clicked(reset)

plt.show()
