import numpy as np
import matplotlib.pyplot as plt


ns = np.array([i for i in range(1, 3)])
probs = (2 ** ns - 1) / (2 ** ns)
print(np.prod(probs))