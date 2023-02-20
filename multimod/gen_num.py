def print_num(a, b, m):
    r = a * b % m
    print(a, b, m, r)

print_num(123, 456, 789)
print_num(456, 789, 123)
print_num(123, 456, 2**64-1)
print_num(2**64-2, 2**64-2, 2**64-1)