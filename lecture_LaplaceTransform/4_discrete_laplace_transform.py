import numpy as np
import matplotlib.pyplot as plt


def signal_x(freq: float, damping: float, t: np.ndarray) -> np.ndarray:
    return np.exp(-damping * t + 1j*(2*np.pi*freq*t))


t = np.linspace(0, 2, 500)
init_freq = 10
init_demp = 5.0
x = signal_x(init_freq, init_demp, t).real
plt.plot(t, x, label='Real')
plt.show()
damps = np.linspace(0, 20.0, 2000)

laplace = np.zeros((len(damps), len(t)), dtype=complex)
for i, damp in enumerate(damps):
    sig = x * np.exp(-damp * t)
    sig /= np.sum(sig)
    laplace[i, :] = np.fft.fft(sig)

freq = len(t) / 2
laplace_abs = np.abs(laplace)
plt.imshow(laplace_abs.T, aspect='auto', extent=[damps[0], damps[-1], -freq, freq], origin='lower')
plt.colorbar(label='Magnitude')
plt.ylabel('Freq [Hz]')
plt.xlabel('Damping')
plt.title('Laplace Transform Magnitude')
plt.show()
