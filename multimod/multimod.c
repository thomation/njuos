#include <stdint.h>
#include <stdio.h>

// 123 == 0x7b == 0000000000000000000000000000000000000000000000000000000001111011
static uint8_t a_bits[64];
static uint8_t b_bits[64];
static uint8_t m_bits[64];
void uint64_to_bits(uint64_t n, uint8_t *bits) {
  for(int i = 0; i < 64; i ++) {
    bits[64 - i - 1]  = (n >> i) & 0x1;
  }
}
uint64_t bits_to_uint64(uint8_t * bits) {
  uint64_t n = 0;
  for(int i = 0; i < 64; i ++) {
    n += bits[i] ? (uint64_t) 0x1 << (63 - i) : 0;
  }
  return n;
}

uint64_t multimod(uint64_t a, uint64_t b, uint64_t m) {
  uint64_to_bits(a, a_bits);
  uint64_to_bits(b, b_bits);
  uint64_to_bits(m, m_bits);

  return (a * b) % m; // buggy
}

int64_t multimod_fast(int64_t a, int64_t b, int64_t m) {
  int64_t t = (a * b - (int64_t)((double)a * b / m) * m) % m;
  return t < 0 ? t + m : t;
}