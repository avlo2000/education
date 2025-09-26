import random
import math
import matplotlib.pyplot as plt
import numpy as np
from tqdm import tqdm
from numpy.random import choice, shuffle


def experiment(n: int, k: int) -> bool:
    bag = list()
    for i in range(n):
        bag.extend([i] * k)
    bag = np.array(bag)
    shuffle(bag)
    taken = bag[:n]
    return len(set(taken)) == n


# def experiment(n: int, k: int) -> bool:
#     bag = dict()
#     for i in range(n):
#         bag[i] = k
#     taken = set()
#     for i in range(n):
#         sum_cnt = sum(bag.values())
#         probs = [bag[ch] / sum_cnt for ch in bag]
#         ch = choice(list(bag.keys()), p=probs)
#         if ch in taken:
#             return False
#         taken.add(ch)
#         bag[ch] -= 1
#         if bag[ch] == 0:
#             del bag[ch]
#     return True


def monte_carlo(n: int, k: int, trials: int) -> float:
    return sum(experiment(n, k) for _ in range(trials)) / trials


def perms_k(n: int, k: int) -> int:
    return math.factorial(n * k) // math.factorial(k) ** n


def analytical(n: int, k: int) -> float:
    denom = perms_k(n, k)
    nom = math.factorial(n) * perms_k(n, k - 1)
    return nom / denom


def main():
    print(f"Analytical solution: {analytical(2, 2)}")
    for n in range(2, 10):
        res = []
        for k in range(1, 20):
            mc = analytical(n, k)
            res.append(mc)
            # print(f"Monte Carlo: {mc}")
            print(f"Analytical solution: {analytical(n, k)}")
        plt.plot(np.arange(1, len(res) + 1), res, label=f"n={n}")
    plt.xlabel("k")
    plt.ylabel("Probability")
    plt.legend()
    plt.show()


if __name__ == '__main__':
    main()
