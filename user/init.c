#include "ulib.h"

int main() {
  // Example:
  // printf("pid = %d\n", getpid());
  for(const char * s = "Hello User!"; *s; s ++)
    kputc(*s);
  return 0;
}
