from matplotlib import pyplot as plt
import numpy as np

def s_plane_plot(sfunc, limits = [3,3,10], nsamp = 500):
    fig = plt.figure()
    ax = fig.add_subplot(projection = '3d')

    sigma = np.linspace(-limits[0], limits[0], nsamp)
    omega = sigma.copy()

    sigma, omega = np.meshgrid(sigma, omega)

    s = sigma + 1j*omega 
    
    ax.plot_surface(sigma, omega, np.abs(sfunc(s)))
    ax.set_zlim(0, limits[2])
    plt.xlabel('$\sigma$')
    plt.ylabel('$j\omega$')
    fig.tight_layout()
    plt.show()

def X(s): return 1/((s + .2+.5j)*(s + .2-.5j))

s_plane_plot(X, limits = [1,1,4], nsamp =40)