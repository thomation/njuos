import random

def print_num(a, b, m):
    r = a * b % m
    print(a, b, m, r)

# special case
print_num(123, 456, 789)
print_num(123, 456, 2**64-1)
print_num(2**64-2, 2**64-2, 2**64-1)
print_num(2**63, 2**63, 2**64-1)

for _ in range(1, 10):
    a = random.randint(0, 2**64 - 1)
    b = random.randint(0, 2**64 - 1)
    m = random.randint(1, 2**64 - 1)
    print_num(a, b, m)
for _ in range(1, 10):
    a = random.randint(2**60 - 1, 2**62-1)
    b = random.randint(2**60 - 1, 2**62-1)
    m = random.randint(2**60 - 1, 2**62-1)
    print_num(a, b, m)