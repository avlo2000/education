import numpy as np
import matplotlib.pyplot as plt


B = 125e3
T = 1 / B
SF = 7
T_s = (2**SF) * T

w = [1, 1, 0, 1, 0, 1, 1]

symbol = 0

for h in range(SF):
    symbol += w[h] * (2**h)

t = np.linspace(start=0, stop=T_s, num=int(T_s / T))

k = np.arange(start=0, stop=len(t), step=0.01)

chirp = np.exp(1j * 2 * np.pi * ((symbol + k) % (2**SF)) / (2**SF) * k)

plt.figure(figsize=(10, 6))

plt.subplot(2, 1, 1)
plt.plot(k, chirp.real, label='Real Part', color='blue')
plt.title('LoRa Signal - Real Part of s(t)')
plt.xlabel('Time Index [k]')
plt.ylabel('Amplitude [mW]')
plt.grid(True)
plt.legend()

plt.subplot(2, 1, 2)
plt.plot(k, chirp.imag, label='Imaginary Part', color='orange')
plt.title('LoRa Signal - Imaginary Part of s(t)')
plt.xlabel('Time Index [k]')
plt.ylabel('Amplitude [mW]')
plt.grid(True)
plt.legend()

plt.tight_layout()
plt.show()
