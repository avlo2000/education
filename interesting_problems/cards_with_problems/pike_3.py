import random


# def sim():
#     # 1/3 subtract, 2/3 add
#     curr = 0
#     n_tests = 9999
#     for _ in range(n_tests):
#         if random.random() < 1/3:
#             curr -= 1
#         else:
#             curr += 1
#         if curr == 0:
#             return True
#     return False


# tests = 90000
# wins = 0
# for _ in range(tests):
#     if sim():
#         wins += 1
# print(wins / tests)

print(1 / 3 / (1 - 2 / 3))