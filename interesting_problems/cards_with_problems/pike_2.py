tests = 100
pos = 0
mean = 0
for n in range(tests):
    p =  n * (n - 1) / (2 ** n)
    mean += p
print(mean)

mean = 0
for n in range(tests):
    p = 0
    for i in range(n):
        p += (n + 1) / 2**(n + 1)
    mean += p
print(mean)
