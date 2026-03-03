import numpy as np
import scipy.stats as stats


a = 579
b = 237
c = 124
d = 41
p1 = a / (a + b)
p2 = c / (c + d)
p = (a + c) / (a + b + c + d)
print(f"p1: {p1:.4f}, p2: {p2:.4f}, p: {p:.4f}")

z = (p2 - p1) / np.sqrt(p * (1 - p) * (1 / (a + b) + 1 / (c + d)))
print(f"z: {z:.4f}")

table = np.array([[a, b],
                  [c, d]])

chi2, p, dof, expected = stats.chi2_contingency(table, correction=False)

print("Chi-square statistic:", chi2)
print("p-value:", p)
print("Degrees of freedom:", dof)
print("Expected frequencies:\n", expected)
