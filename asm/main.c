#include "asm.h"
#include <assert.h>
#include <stdio.h>

int main() {
  printf("Hello longjmp\n");
  asm_jmp_buf buf;
  int r = asm_setjmp(buf);
  if (r == 0) {
    printf("longjmp 0\n");
    // assert(asm_add(1234, 5678) == 6912);
    // assert(asm_popcnt(0x0123456789abcdefULL) == 32);
    // TODO: add more tests here.
    asm_longjmp(buf, 123);
    printf("after call longjmp \n");
  } else {
    printf("longjmp %d\n", r);
    assert(r == 123);
    printf("PASSED.\n");
  }
}
