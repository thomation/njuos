#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>

#define U64 "%" PRIu64
#define D64 "%" PRId64
#define X64 "%" PRIx64

uint64_t multimod(uint64_t, uint64_t, uint64_t);
int64_t multimod_fast(int64_t a, int64_t b, int64_t m);
void uint64_to_bits(uint64_t n, uint8_t *bits); 
uint64_t bits_to_uint64(uint8_t * bits); 

void print_bits(uint8_t * bits, int len) {
  for(int i = 0; i < len; i ++)
    printf("%c", bits[i] + '0');
  printf("\n");
}
void test(uint64_t a, uint64_t b, uint64_t m, uint64_t r) {
  uint64_t v = multimod(a, b, m);
  printf(U64 " * " U64 " mod " U64 " = " U64 " ?= " U64 " --> %s \n", a, b, m, v, r,  v == r ? "pass" : "fail");
}
void test2(int64_t a, int64_t b, int64_t m, int64_t r) {
  int64_t v = multimod_fast(a, b, m);
  printf(D64 " * " D64 " mod " D64 " = " D64 " ?= " D64 " --> %s \n", a, b, m, v, r,  v == r ? "pass" : "fail");
}
void test_bits_convert(uint64_t n) {
  printf(X64 "\n", n);
  uint8_t bits[64];
  uint64_to_bits(n, bits);
  print_bits(bits, 64);
  uint64_t n2 = bits_to_uint64(bits);
  printf(X64 "\n", n2);
  assert(n == n2);
}

int main(int argc, char* argv[]) {
  uint64_t a, b, m, r;
  sscanf(argv[1], U64, &a);
  sscanf(argv[2], U64, &b);
  sscanf(argv[3], U64, &m);
  sscanf(argv[4], U64, &r);
  test_bits_convert(a);
  test_bits_convert(b);
  test_bits_convert(m);
  test_bits_convert(r);
  test(a, b, m, r);
  // int64_t a, b, m, r;
  // sscanf(argv[1], D64, &a);
  // sscanf(argv[2], D64, &b);
  // sscanf(argv[3], D64, &m);
  // sscanf(argv[4], D64, &r);
  // test2(a, b, m, r);
}
