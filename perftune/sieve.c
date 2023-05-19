#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define N 10000000

static bool is_not_prime[N];
static int  primes[N];

int *sieve(int n) {
  assert(n + 1 < N);
  for (int i = 2; i <= n; i++) {
    if(is_not_prime[i])
      continue;
    for (int j = i + i; j <= n; j += i) {
      is_not_prime[j] = true;
    }
  }

  int *p = primes;
  *p++ = 2;
  for (int i = 3; i <= n; i+=2)
    if (!is_not_prime[i]) {
      *p++ = i;
    }
  *p = 0;
  return primes;
}
