#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#define N 10000000
//These macro must be updated when you change the type of prim array. 
#define WORD_BIT 5
#define WORD_SIZE 32 
#define WORD_MASK 0x1f
#define HARD_CODE_PRIME_NUM 11
#define FIRST_PRIME 37
#define PRIME_ARRAY_TYPE uint32_t
#define FIRST_PRIME_SLOT 0x55555550
#define PRIME_SLOT 0x55555555
//-----------------------------------------
static PRIME_ARRAY_TYPE is_not_prime[N >> WORD_BIT];
static int  primes[N] = {2,3,5,7,11,13,17,19,23,29,31};

#define GET_NOT_PRIME(x) (is_not_prime + (x >> WORD_BIT)) 
#define PRIME_BITS(x) (0x1 << (x&WORD_MASK))
#define IS_NOT_PRIME(x) (*GET_NOT_PRIME(x) & PRIME_BITS(x))
#define SET_NOT_PRIME(x) (*GET_NOT_PRIME(x) |= PRIME_BITS(x))
#define COMPUTE_PRIME(i, n) { \
  int step = i + i; \
  for (int j = i + i + i; j <= n; j += step) { \
    SET_NOT_PRIME(j); \
  } \
}
#define SELECT_RESULT(p, q, i) { \
  int j = 1 - ((*q >> i) & 0x1); \
  *p = index + i; \
  p += j; \
}
int *sieve(int n) {
  assert(n + 1 < N);
  *is_not_prime = FIRST_PRIME_SLOT;
  int max = n >> WORD_BIT;
  if (max << WORD_BIT < n) max ++;
  PRIME_ARRAY_TYPE *prime_end = is_not_prime + max;
  for(PRIME_ARRAY_TYPE * q = is_not_prime + 1; q < prime_end; q++) {
    *q = PRIME_SLOT;
  }
  for(int i = 1; i < HARD_CODE_PRIME_NUM; i ++)
    COMPUTE_PRIME(primes[i], n);

  for (int i = FIRST_PRIME; i * i <= n; i+= 2) {
    if(IS_NOT_PRIME(i))
      continue;
    COMPUTE_PRIME(i, n);
  }

  int *p = primes + HARD_CODE_PRIME_NUM;
  int index = WORD_SIZE;
  for(PRIME_ARRAY_TYPE * q = is_not_prime + 1; q < prime_end; q++, index += WORD_SIZE) {
    for(int i = 1; i < WORD_SIZE; i += 2)
      SELECT_RESULT(p, q, i);
  }
  *p = 0;
  return primes;
}
