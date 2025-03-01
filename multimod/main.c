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
uint64_t bits_to_uint64(uint8_t * bits, int len); 
void print_bits(uint8_t * bits, int len);
int bits_left_1(uint8_t *bits, int len); 
void bits_sub(uint8_t *l_bits, int l_size, uint8_t *r_bits, int r_size); 
void bits_mod(uint8_t *bits, uint8_t * m_bits);
void bits_add(uint8_t *l, uint8_t *r); 
void reset_bits(uint8_t * bits, int len); 

void test(uint64_t a, uint64_t b, uint64_t m, uint64_t r) {
  printf(U64 " * " U64 " mod " U64 " = " U64 " \n", a, b, m, r);
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
  uint64_t n2 = bits_to_uint64(bits, 64);
  printf(X64 "\n", n2);
  assert(n == n2);
}
// left must be large than right
void test_bits_sub(uint64_t l, uint64_t r) {
  assert(l >= r);
  printf(U64 " - " U64 " = " U64 "\n", l, r, l - r);
  uint8_t l_bits[64];
  uint64_to_bits(l, l_bits);
  // print_bits(l_bits, 64);
  uint8_t r_bits[64];
  uint64_to_bits(r, r_bits);
  // print_bits(r_bits, 64);
  int l1 = bits_left_1(l_bits, 64);
  int r1 = bits_left_1(r_bits, 64);
  bits_sub(l_bits + l1, 64 - l1, r_bits + r1, 64 - r1);
  // print_bits(l_bits, 64);
  uint64_t v = bits_to_uint64(l_bits, 64);
  printf(U64 " - " U64 " = " U64 "\n", l, r, v);
  assert(v == l - r);
}
void test_bits_mod(uint64_t n, uint64_t m) {
  printf(U64 " mod " U64 " = " U64 "\n", n, m, n % m);
  uint8_t n_bits[128];
  reset_bits(n_bits, 128);
  uint64_to_bits(n, n_bits + 64);
  uint8_t m_bits[64];
  uint64_to_bits(m, m_bits);
  bits_mod(n_bits, m_bits);
  print_bits(n_bits, 128);
  uint64_t v = bits_to_uint64(n_bits + 64, 64);
  printf(U64 " mod " U64 " = " U64 "\n", n, m, v);
  assert(v == n % m);
}
void test_bits_add(uint64_t l, uint64_t r) {
  printf(U64 " + " U64 " = " U64 "\n", l, r, l + r);
  uint8_t l_bits[128];
  reset_bits(l_bits, 128);
  uint64_to_bits(l, l_bits + 64);
  uint8_t r_bits[128];
  reset_bits(r_bits, 128);
  uint64_to_bits(r, r_bits + 64);

  bits_add(l_bits, r_bits);
  uint64_t v = bits_to_uint64(l_bits + 64, 64);
  printf(U64 " + " U64 " = " U64 "\n", l, r, v);
  assert(l + r == v);
}
void test_components() {
  printf("================ test bits convert ================================\n");
  test_bits_convert(123);
  test_bits_convert(456);
  test_bits_convert(-1ULL);
  test_bits_convert(-2ULL);

  printf("================ test bits sub ================================\n");
  test_bits_sub(1, 1);
  test_bits_sub(456, 123);
  test_bits_sub(-1ULL, 123);
  test_bits_sub(-1ULL, -2ULL);
  test_bits_sub(-1ULL, -1ULL);

  printf("================ test bits mod ================================\n");
  test_bits_mod(1, 1);
  test_bits_mod(123, 456);
  test_bits_mod(456, 123);
  test_bits_mod(-2ULL, -1ULL);
  test_bits_mod(-1ULL, -1ULL);
  test_bits_mod(123*456, 789);

  printf("================ test bits add ================================\n");
  test_bits_add(0, 0);
  test_bits_add(0, 1);
  test_bits_add(1, 0);
  test_bits_add(123, 456);
  test_bits_add(-1ULL, 1);
}
int main(int argc, char* argv[]) {
  uint64_t a, b, m, r;
  sscanf(argv[1], U64, &a);
  sscanf(argv[2], U64, &b);
  sscanf(argv[3], U64, &m);
  sscanf(argv[4], U64, &r);
  test(a, b, m, r);
  //  test_components();
  // int64_t a, b, m, r;
  // sscanf(argv[1], D64, &a);
  // sscanf(argv[2], D64, &b);
  // sscanf(argv[3], D64, &m);
  // sscanf(argv[4], D64, &r);
  // test2(a, b, m, r);
}
