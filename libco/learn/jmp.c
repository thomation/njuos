#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <setjmp.h>

// Only save callee saved register
//https://github.com/lattera/glibc/blob/master/sysdeps/x86_64/setjmp.S

static jmp_buf buf;
static inline void save()
{
  if(setjmp(buf) != 0)
    printf("Comeback\n");
}
void restore()
{
  longjmp(buf, 1);
}
int main()
{
  int n = 0;
  // setjmp(buf);
  save();
  n++;
  //the value on stack will be the newest.
  printf("Hello %d\n", n);
  restore();
}