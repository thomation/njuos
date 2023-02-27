#include <stdint.h>
#include <stdio.h>
#include <assert.h>

void print_bits(uint8_t * bits, int len) {
  for(int i = 0; i < len; i ++)
    printf("%c", bits[i] + '0');
  printf("\n");
}
// 123 == 0x7b == 0000000000000000000000000000000000000000000000000000000001111011
void uint64_to_bits(uint64_t n, uint8_t *bits) {
  for(int i = 0; i < 64; i ++) {
    bits[64 - i - 1]  = (n >> i) & 0x1;
  }
}
uint64_t bits_to_uint64(uint8_t * bits, int len) {
  assert(len <= 64);
  uint64_t n = 0;
  for(int i = 0; i < len; i ++) {
    n += bits[i] ? (uint64_t) 0x1 << (len - 1 - i) : 0;
  }
  return n;
}
void bit_multi_bits(uint8_t bit, uint8_t *bits, uint8_t * ret)
{
  for(int i = 0; i < 64; i ++) {
    ret[i] = bit == 0x1 ? bits[i] : 0x0;
  }
}
int bits_left_1(uint8_t *bits, int len) {
  for(int i = 0; i < len; i ++) {
    if(bits[i] == 1) 
      return i;
  }
  assert(0);
}
void bits_sub(uint8_t *l_bits, int l_size, uint8_t *r_bits, int r_size) {
  printf("bits_sub: %d, %d\n", l_size, r_size);
  // print_bits(l_bits, l_size);
  // print_bits(r_bits, r_size);
  uint64_t l = bits_to_uint64(l_bits, l_size);
  uint64_t r = bits_to_uint64(r_bits, r_size);
  uint64_t t = l - r;
  uint8_t t_bits[64];
  uint64_to_bits(t, t_bits);
  // printf("bits_sub result:\n");
  // print_bits(t_bits, 64);
  for(int i = 0; i < l_size; i ++) {
    // printf("%d: %d -> %d\n",i, l_bits[i], t_bits[i + 64 - l_size]);
    l_bits[i] = t_bits[i + 64 - l_size];
  }
}
uint64_t bits_mod(uint8_t *bits, uint8_t * m_bits) {
  int left = bits_left_1(bits, 128);
  int m_left = bits_left_1(m_bits, 64);
  // printf("left:%d, m_left:%d\n", left, m_left);
  // print_bits(bits, 128);
  // print_bits(m_bits, 64);
  for(int i = left; i < 128; i ++) {
    if(128 - i < 64 - m_left)
      break;
    if(bits[i] == 0) {
      continue;
    }
    // bits[i] == 1
    int m_is_small = 1;
    for(int j = m_left; j < 64; j ++) {
      if(bits[i + j - m_left] < m_bits[j]) {
        m_is_small = 0;
        break;
      }
    }
    // printf("mod before sub: %d\n", i);
    // print_bits(bits, 128);
    // print_bits(bits + i, 128 - i);
    if(m_is_small) {
      bits_sub(bits + i, 64 - m_left, m_bits + m_left, 64 - m_left);
    } else if(64 - m_left < 64 && (128 - i > 64 - m_left)) {
      bits_sub(bits + i, 64 - m_left + 1, m_bits + m_left, 64 - m_left);
    }
    // printf("mod after sub: %d\n", i);
    // print_bits(bits, 128);
    // print_bits(bits + i, 128 - i);
  }
  print_bits(bits + 64, 64);
  uint64_t ret = bits_to_uint64(bits + 64, 64);
  return ret;
}
uint64_t multimod(uint64_t a, uint64_t b, uint64_t m) {
  assert(m != 0);
  if(a == 0 || b == 0)
    return 0;
  uint8_t a_bits[64];
  uint8_t b_bits[64];
  uint8_t m_bits[64];
  uint8_t tmp_bits[128];
  uint64_to_bits(a, a_bits);
  uint64_to_bits(b, b_bits);
  uint64_to_bits(m, m_bits);
  uint64_t ret = 0;
  for(int i = 0; i < 64; i ++) {
    if(a_bits[i] == 0)
      continue;
    for(int j = 0; j < 128; j ++)
      tmp_bits[j] = 0x0;
    // tmp_bits == 00--0[multi resutl][i bits 0]
    bit_multi_bits(a_bits[i], b_bits, tmp_bits + i + 1);
    // printf("a index: %d\n", i);
    // print_bits(b_bits, 64);
    // print_bits(tmp_bits, 128);
    ret += bits_mod(tmp_bits, m_bits);
    ret %= m;
  }
  return ret;
}

int64_t multimod_fast(int64_t a, int64_t b, int64_t m) {
  int64_t t = (a * b - (int64_t)((double)a * b / m) * m) % m;
  return t < 0 ? t + m : t;
}