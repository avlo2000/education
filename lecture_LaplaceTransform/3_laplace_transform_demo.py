import numpy as np
import matplotlib.pyplot as plt


def signal(z: complex, t: np.ndarray) -> np.ndarray:
    return np.exp(-z * t).real


def signal_z(z: complex, t: np.ndarray) -> np.ndarray:
    return np.exp(-z * t)


def cov(z0: np.ndarray, z1: np.ndarray) -> float:
    return np.sum(z0 * z1.conj())


def compute_z_transform(x: np.ndarray, t: np.ndarray, min: complex, max: complex, n: int, m: int) -> np.ndarray:
    real_linspace = np.linspace(min.real, max.real, n)
    imag_linspace = 1.0j * np.linspace(min.imag, max.imag, m)
    xx, yy = np.meshgrid(real_linspace, imag_linspace)
    z_vals = xx + yy
    plt.imshow(np.abs(z_vals))
    plt.show()
    Z_transform = np.sum(x * np.exp(-z_vals[..., None] * t), axis=-1)
    return Z_transform


if __name__ == '__main__':
    N = 500
    t = np.linspace(0.0, 15.0, N)
    x = signal(0.0 + 30.3j, t)
    plt.subplot(211)
    plt.plot(t, x)
    mn = -5.0 - 5.0j
    mx = +5.0 + 5.0j
    n = 500
    m = 500
    X_vals = compute_z_transform(x, t, mn, mx, n, m)
    plt.subplot(212)
    plt.imshow(np.abs(X_vals), extent=[mn.real, mx.real, mn.imag, mx.imag], origin='lower', aspect='auto')
    plt.colorbar()
    plt.title('Z-Transform Magnitude')
    plt.xlabel('Real Part')
    plt.ylabel('Imaginary Part')
    plt.show()
