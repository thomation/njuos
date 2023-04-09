#include "asm.h"
#include <string.h>
#include <stdio.h>

int64_t asm_add(int64_t a, int64_t b) {
  int64_t c = 0;
  asm volatile("movq %1, %%rax; movq %2, %%rdx; addq %%rdx, %%rax; movq %%rax, %0"
    : "=r"(c)
    : "a"(a), "b"(b)
  );
  return c;
}

int asm_popcnt(uint64_t x) {
  int s = 0;
  for (int i = 0; i < 64; i++) {
    if ((x >> i) & 1) s++;
  }
  return s;
}

void *asm_memcpy(void *dest, const void *src, size_t n) {
  return memcpy(dest, src, n);
}

int asm_setjmp(asm_jmp_buf env) {
  long int *buf = env[0].__jmpbuf;
  // printf("before setjmp asm\n");
  // rbx, rbp, r12, r13, r14, r15, rsp, pc
  asm volatile("movq %%rbx, %0;"
               "movq %%rbp, %1;"
               "movq %%r12, %2;"
               "movq %%r13, %3;"
               "movq %%r14, %4;"
               "movq %%r15, %5;"
               "lea 8(%%rsp), %%rdx;"
               "movq %%rdx, %6;"
               "movq (%%rsp), %%rax;"
               "movq %%rax, %7"
    : "=r"(buf[0]), "=r"(buf[1]), "=r"(buf[2]), "=r"(buf[3]), "=r"(buf[4]), "=r"(buf[5]), "=r"(buf[6]), "=r"(buf[7])
  );
  // printf("after setjmp asm\n");
  return 0;
}

void asm_longjmp(asm_jmp_buf env, int val) {
  // printf("before longjmp asm\n");
  long int *buf = env[0].__jmpbuf;
  // rbx, rbp, r12, r13, r14, r15, rsp, pc
  asm volatile("movq %0, %%rbx;"
               "movq %1, %%r9;"
               "movq %2, %%r12;"
               "movq %3, %%r13;"
               "movq %4, %%r14;"
               "movq %5, %%r15;"
               "movq %6, %%r8;"
               "movq %7, %%rdx;"
               "movq %%r8, %%rsp;"
               "movq %%r9, %%rbp;"
               "mov %8, %%eax;"
               "jmpq *%%rdx"
    :: "r"(buf[0]), "r"(buf[1]), "r"(buf[2]), "r"(buf[3]), "r"(buf[4]), "r"(buf[5]), "r"(buf[6]), "r"(buf[7]), "r"(val)
  );
  // printf("after longjmp asm\n");
}
