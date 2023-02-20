#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>

uint64_t multimod(uint64_t, uint64_t, uint64_t);

void test(uint64_t a, uint64_t b, uint64_t m, uint64_t r) {
  uint64_t v = multimod(a, b, m);
  #define U64 "%" PRIu64
  printf(U64 " * " U64 " mod " U64 " = " U64 " ?= " U64 " --> %s \n", a, b, m, v, r,  v == r ? "pass" : "fail");
}

int main(int argc, char* argv[]) {
  // test(123, 456, 789, 69);
  // test(123, 456, -1ULL, 56088);
  // test(-2ULL, -2ULL, -1ULL, 1); // should be 1
  uint64_t a, b, m, r;
  sscanf(argv[1], U64, &a);
  sscanf(argv[2], U64, &b);
  sscanf(argv[3], U64, &m);
  sscanf(argv[4], U64, &r);
  test(a, b, m, r);
}
