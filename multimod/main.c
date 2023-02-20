#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>

uint64_t multimod(uint64_t, uint64_t, uint64_t);

void test(uint64_t a, uint64_t b, uint64_t m, uint64_t r) {
  uint64_t v = multimod(a, b, m);
  #define U64 "%" PRIu64
  printf(U64 " * " U64 " mod " U64 " = " U64 " ?= " U64 "\n", a, b, m, v, r);
  assert(v == r);
}

int main() {
  test(123, 456, 789, 69);
  test(123, 456, -1ULL, 56088);
  test(-2ULL, -2ULL, -1ULL, 1); // should be 1
}
