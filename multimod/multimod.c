#include <stdint.h>

uint64_t multimod(uint64_t a, uint64_t b, uint64_t m) {
  return (a * b) % m; // buggy
}

int64_t multimod_fast(int64_t a, int64_t b, int64_t m) {
  int64_t t = (a * b - (int64_t)((double)a * b / m) * m) % m;
  return t < 0 ? t + m : t;
}