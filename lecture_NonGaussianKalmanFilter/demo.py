import matplotlib.pyplot as plt
import numpy as np
import rnd_dyn
from kalman_filter import KalmanFilter
from state_transition import StateTransition


def create_st(nx, nu, ny) -> StateTransition:
    mat_a = np.eye(nx)
    mat_a[0, 1] = 0.02
    mat_b = np.eye(nx, nu)
    mat_c = np.eye(ny, nx)
    return StateTransition(mat_a, mat_b, mat_c)


def main():
    n_iters = 1000

    nx, nu, ny = 3, 3, 2
    st = create_st(nx, nu, ny)

    process_cov = rnd_dyn.rand_positive_semidefinite(nx, 400)
    measurement_cov = rnd_dyn.rand_positive_semidefinite(ny, 6)

    x = np.zeros(nx)
    u = np.ones(nu)

    x_trj = [x]
    x_noisy_trj = [x]
    x_kalman_trj = [x]
    x_kalman_cv_trj = [x]

    kf = KalmanFilter(
        st, process_cov, measurement_cov
    )
    kf.reset(x)

    for _ in range(n_iters):
        x_next, y, x_next_noisy, y_noisy = rnd_dyn.random_dynamics(
            st,
            x_trj[-1],
            u,
            process_cov,
            measurement_cov
        )
        kf.time_update(u)
        kf.measurement_update(y_noisy)

        x_kalman_trj.append(kf.get_prediction())
        x_trj.append(x_next)
        x_noisy_trj.append(x_next_noisy)

    plt.plot(x_trj, c='green', label='real')
    plt.plot(x_noisy_trj, c='red', label='measured')
    plt.plot(x_kalman_trj, c='orange', label='kalman')
    plt.plot(x_kalman_cv_trj, c='pink', label='kalman')
    plt.show()


if __name__ == '__main__':
    main()
