#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define N 10000000

static __uint8_t is_not_prime[N >> 3];
static int  primes[N];
#define IS_NOT_PRIME(x)  (is_not_prime[x >> 3] & (0x1 << (x&0x7)))
#define SET_NOT_PRIME(x) (is_not_prime[x >> 3] |= (0x1 << (x&0x7)))
int *sieve(int n) {
  assert(n + 1 < N);

  for(int i = 4; i <=n; i +=2 )
    SET_NOT_PRIME(i);
  for (int i = 3; i <= n; i+= 2) {
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
