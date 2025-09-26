from fractions import Fraction
import numpy as np


def period_estimation(periods: np.ndarray) -> float:
    fraction = [Fraction(period) for period in periods]
    nums = [frac.numerator for frac in fraction]
    dens = [frac.denominator for frac in fraction]
    return np.lcm.reduce(nums) / np.gcd.reduce(dens)

print(period_estimation(np.array([3, 2.2, 5.1])))
