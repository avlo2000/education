import control as ct
import numpy as np


def simulate_oreder2(time: np.ndarray, controller: ct.TransferFunction):
    m = 10.0
    k = 1
    b = 0.02

    A = [[0, 1.0], [-k / m, -b / m]]
    B = [[0], [1 / m]]
    C = [[1.0, 0]]
    sys = ct.StateSpace(A, B, C, 0)
    plant = ct.ss2tf(sys)
    plant = ct.series(plant)
    sys = ct.feedback(plant * controller, 1)

    T, yout = ct.forced_response(sys, T=time, U=1)
    return yout, T
