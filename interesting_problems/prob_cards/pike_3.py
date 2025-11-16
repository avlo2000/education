import numpy as np


n = 70
P = np.zeros((n, n))
P[0, 0] = 1.0
P[-1, -1] = 0.0
for i in range(1, n - 1):
    P[i, i - 1] = 0.5
    P[i, i + 1] = 0.5
probs = np.zeros(n)
probs[1] = 1.0

for p in range(1, 30):
    probs = probs @ np.linalg.matrix_power(P, p)
    print(probs)
    print(np.sum(probs))
