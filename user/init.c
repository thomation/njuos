#include "ulib.h"

int main() {
  // Example:
  // printf("pid = %d\n", getpid());
  int pid = fork();
  if(pid == 0) {
    while(1) {
      kputc('c');
    }
  }
  else {
    while(1) {
      kputc('p');
    }
  }
  return 0;
}
