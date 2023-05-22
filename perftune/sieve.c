#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#define N 10000000

static uint8_t is_not_prime[N >> 3];
static int  primes[N];
#define IS_NOT_PRIME(x)  (is_not_prime[x >> 3] & (0x1 << (x&0x7)))
#define SET_NOT_PRIME(x) (is_not_prime[x >> 3] |= (0x1 << (x&0x7)))
int *sieve(int n) {
  assert(n + 1 < N);
  *is_not_prime = 0x50;
  uint8_t *prime_end = is_not_prime + (N >> 3);
  int index = 8;
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
  for (int i = 11; i <= n; i+= 2) {
    if(IS_NOT_PRIME(i))
      continue;
    for (int j = i + i + i; j <= n; j += i + i) {
      SET_NOT_PRIME(j);
    }
  }

  int *p = primes;
  *p++ = 2;
  for (int i = 3; i <= n; i+=2)
    if (!IS_NOT_PRIME(i)) {
      *p++ = i;
    }
  *p = 0;
  return primes;
}
