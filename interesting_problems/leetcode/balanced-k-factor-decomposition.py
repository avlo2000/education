from functools import lru_cache


def trial_division(n):
    factorization = []
    d = 2
    while d * d <= n:
        while n % d == 0:
            factorization.append(d)
            n //= d
        d += 1
    if n > 1:
        factorization.append(n)
    return factorization

def diff(res):
    return max(res) - min(res)

@lru_cache(None)
def dp(res, factors, idx, n, k):
    if idx == len(factors):
        return res[:]
    cand0 = [1] * k
    cand0[-1] = n
    for i in range(len(res)):
        res[i] *= factors[idx]
        cand1 = dp(res, factors, idx + 1, n, k)
        res[i] //= factors[idx]
        if diff(cand0) > diff(cand1):
            cand0 = cand1
    return cand0

def minDifference(n, k):
    res = [1] * k
    factors = trial_division(n)
    return dp(res, factors, 0, n, k)

if __name__ == "__main__":
    res = minDifference(72072, 5)
    print(*res)