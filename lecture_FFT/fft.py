import numpy as np


def fft(x: np.ndarray) -> np.ndarray:
    if len(x) == 1:
        return x
    x_even = x[::2]
    x_odd = x[1::2]
    fft_even = fft(x_even)
    fft_odd = fft(x_odd)
    freq = np.empty_like(x, dtype=complex)
    for i in range(len(x) // 2):
        phasor = np.exp(-1j * i * 2 * np.pi / len(x))
        freq[i] = fft_even[i] + phasor * fft_odd[i]
        freq[i + len(x) // 2] = fft_even[i] - phasor * fft_odd[i]
    return freq


def ifft(w: np.ndarray) -> np.ndarray:
    return fft(w.conj()).conj() / len(w)


x = np.arange(16)
print(x)
res = fft(x)
res = ifft(res)
print(res)
