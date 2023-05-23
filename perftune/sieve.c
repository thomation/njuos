#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#define N 10000000
#define WORD_BIT 3
#define WORD_SIZE 8
#define WORD_MASK 0x7
static uint8_t is_not_prime[N >> WORD_BIT];
static int  primes[N];
#define GET_NOT_PRIME(x) (is_not_prime + (x >> 3)) 
#define PRIME_BITS(x) (0x1 << (x&WORD_MASK))
#define IS_NOT_PRIME(x) (*GET_NOT_PRIME(x) & PRIME_BITS(x))
#define SET_NOT_PRIME(x) (*GET_NOT_PRIME(x) |= PRIME_BITS(x))
#define RUN(t, g, index) do { \
    int test = (t >> g) & 0x1; \
    if(test == 0) { \
      int i = index + g; \
      for (int j = i + i + i; j <= n; j += i + i) { \
        SET_NOT_PRIME(j); \
      } \
    } \
}while(0)

#define SELECT_RESULT(p, t, i) { \
  int j = 1 - ((t >> i) & 0x1); \
  *p = index + i; \
  p += j; \
}
int *sieve(int n) {
  assert(n + 1 < N);
  *is_not_prime = 0x50;
  int max = n >> WORD_BIT;
  if (max << WORD_BIT < n) max ++;
  uint8_t *prime_end = is_not_prime + max;
  for(uint8_t * q = is_not_prime + 1; q < prime_end; q++) {
    *q = 0x55;
  }

  for (int j = 9; j <= n; j += 6) {
    SET_NOT_PRIME(j);
  }
  for (int j = 25; j <= n; j += 10) {
    SET_NOT_PRIME(j);
  }
  for (int j = 21; j <= n; j += 14) {
    SET_NOT_PRIME(j);
  }
  // int index = 8;
  // for(uint8_t * q = is_not_prime + 1; q < prime_end; q ++, index += 8) {
  //   if(index * index > n)
  //     break;
  //   uint8_t t = *q;
  //   RUN(t, 1, index);
  //   RUN(t, 3, index);
  //   RUN(t, 5, index);
  //   RUN(t, 7, index);
  // }
  for (int i = 11; i * i <= n; i+= 2) {
    if(IS_NOT_PRIME(i))
      continue;
    int step = i + i;
    for (int j = i + i + i; j <= n; j += step) {
      SET_NOT_PRIME(j);
    }
  }

  int *p = primes;
  *p++ = 2;
  *p++ = 3;
  *p++ = 5;
  *p++ = 7;
  int index = WORD_SIZE;
  for(uint8_t * q = is_not_prime + 1; q < prime_end; q++, index += WORD_SIZE) {
    uint8_t t = *q;
    SELECT_RESULT(p, t, 1);
    SELECT_RESULT(p, t, 3);
    SELECT_RESULT(p, t, 5);
    SELECT_RESULT(p, t, 7);
  }
  *p = 0;
  return primes;
}
