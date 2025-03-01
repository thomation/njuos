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
  return -1;
}
void bits_sub(uint8_t *l_bits, int l_size, uint8_t *r_bits, int r_size) {
  // printf("bits_sub: %d, %d\n", l_size, r_size);
  // print_bits(l_bits, l_size);
  // print_bits(r_bits, r_size);
  assert(l_size >= r_size);
  uint8_t c = 0;
  for(int i = 0; i < l_size; i ++) {
    uint8_t lb = l_bits[l_size - i - 1];
    uint8_t rb = i >= r_size ? 0 : r_bits[r_size - i - 1];
    int sub = lb - rb - c;
    if(sub >= 0) {
      c = 0;
      l_bits[l_size - i - 1] = sub;
    } else {
      c = 1;
      l_bits[l_size - i - 1] = (- sub) % 2;
    }
  }
  // printf("bits_sub result:\n");
  // print_bits(l_bits, l_size);
}
void bits_mod(uint8_t *bits, uint8_t * m_bits) {
  int left = bits_left_1(bits, 128);
  if(left < 0) // bits is zero
    return;
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
      int nb = bits[i + j - m_left];
      int mb = m_bits[j];
      if(nb == mb) {
        continue;
      } 
      m_is_small = mb <= nb;
      break;
    }
    // printf("mod before sub: %d\n", i);
    // print_bits(bits, 128);
    // print_bits(bits + i, 128 - i);
    if(m_is_small) {
      bits_sub(bits + i, 64 - m_left, m_bits + m_left, 64 - m_left);
    } else if(128 - i > 64 - m_left) {
      bits_sub(bits + i, 64 - m_left + 1, m_bits + m_left, 64 - m_left);
    }
    // printf("mod after sub: %d\n", i);
    // print_bits(bits, 128);
    // print_bits(bits + i, 128 - i);
  }
  // printf("mod result\n");
  // print_bits(bits, 128);
}
void bits_add(uint8_t *l, uint8_t *r) {
  int c = 0;
  // print_bits(l, 128);
  // print_bits(r, 128);
  for(int i = 127; i >= 0; i --) {
    uint8_t ret = (l[i] + r[i] + c) % 2;
    c = l[i] + r[i] + c >= 2;
    l[i] = ret;
  }
  // print_bits(l, 128);
}
void reset_bits(uint8_t * bits, int len) {
  for(int i = 0; i < len; i ++)
    bits[i] = 0x0;
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
  // print_bits(a_bits, 64);
  // print_bits(b_bits, 64);
  // print_bits(m_bits, 64);
  uint8_t ret_bits[128];
  reset_bits(ret_bits, 128);
  for(int i = 0; i < 64; i ++) {
    if(a_bits[i] == 0)
      continue;
    reset_bits(tmp_bits, 128);
    // tmp_bits == 0i*0[multi resutl]0..0
    bit_multi_bits(a_bits[i], b_bits, tmp_bits + i + 1);
    // printf("a index: %d\n", i);
    // print_bits(b_bits, 64);
    // print_bits(tmp_bits, 128);
    bits_mod(tmp_bits, m_bits);
    bits_add(ret_bits, tmp_bits);
  }
  // print_bits(ret_bits, 128);
  bits_mod(ret_bits, m_bits);
  // print_bits(ret_bits, 128);
  uint64_t ret = bits_to_uint64(ret_bits + 64, 64);
  return ret;
}

int64_t multimod_fast(int64_t a, int64_t b, int64_t m) {
  int64_t t = (a * b - (int64_t)((double)a * b / m) * m) % m;
  return t < 0 ? t + m : t;
}