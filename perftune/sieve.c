#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#define N 10000000
#define WORD_BIT 4
#define WORD_SIZE 16 
#define WORD_MASK 0xf
#define PRIME_ARRAY_TYPE uint16_t
static PRIME_ARRAY_TYPE is_not_prime[N >> WORD_BIT];
static int  primes[N];
#define GET_NOT_PRIME(x) (is_not_prime + (x >> WORD_BIT)) 
#define PRIME_BITS(x) (0x1 << (x&WORD_MASK))
#define IS_NOT_PRIME(x) (*GET_NOT_PRIME(x) & PRIME_BITS(x))
#define SET_NOT_PRIME(x) (*GET_NOT_PRIME(x) |= PRIME_BITS(x))

#define SELECT_RESULT(p, t, i) { \
  int j = 1 - ((t >> i) & 0x1); \
  *p = index + i; \
  p += j; \
}
int *sieve(int n) {
  assert(n + 1 < N);
  *is_not_prime = 0x5550;
  int max = n >> WORD_BIT;
  if (max << WORD_BIT < n) max ++;
  PRIME_ARRAY_TYPE *prime_end = is_not_prime + max;
  for(PRIME_ARRAY_TYPE * q = is_not_prime + 1; q < prime_end; q++) {
    *q = 0x5555;
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
  *p++ = 11;
  *p++ = 13;
  int index = WORD_SIZE;
  for(PRIME_ARRAY_TYPE * q = is_not_prime + 1; q < prime_end; q++, index += WORD_SIZE) {
    PRIME_ARRAY_TYPE t = *q;
    SELECT_RESULT(p, t, 1);
    SELECT_RESULT(p, t, 3);
    SELECT_RESULT(p, t, 5);
    SELECT_RESULT(p, t, 7);
    SELECT_RESULT(p, t, 9);
    SELECT_RESULT(p, t, 11);
    SELECT_RESULT(p, t, 13);
    SELECT_RESULT(p, t, 15);
  }
  *p = 0;
  return primes;
}
