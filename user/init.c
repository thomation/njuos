#include "ulib.h"

static char * text = "Hello User!";
int main() {
  // Example:
  // printf("pid = %d\n", getpid());
  for(int i = 0; i < 12; i ++)
    kputc(text[i]);
  return 0;
}
