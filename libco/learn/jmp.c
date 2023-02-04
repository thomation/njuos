#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

static int method(void *param)
{
    printf("Hello jmp start\n");
    int i = (int)param;
    printf("Hello jmp param\n");
    printf("Hello Jmp %d\n", i);
    return 1;
}
static inline void stack_switch_call(void *sp, void *entry, uintptr_t arg)
{
  asm volatile(
// #if __x86_64__
//       "movq %0, %%rsp; movq %2, %%rdi; jmp *%1"
//       :
//       : "b"((uintptr_t)sp), "d"(entry), "a"(arg)
// #else
//       "movl %0, %%esp; movl %2, 4(%0); jmp *%1"
//       :
//       : "b"((uintptr_t)sp - 8), "d"(entry), "a"(arg)
// #endif
#if __x86_64__
      "movq %2, %%rdi; call *%1"
      :
      : "b"((uintptr_t)sp), "d"(entry), "a"(arg)
#endif
  );
}
int main()
{
    // method(1);
    // stack_switch_call(0, method, 7);
    uintptr_t p = 2;
    int size = sizeof(u_int8_t) * 8192;
    u_int8_t *stack = malloc(size);
    u_int8_t *top = stack + 1024;
    printf("stack top:%x\n", top);
    asm volatile(
#if __x86_64__
    //   "movq %2, %%rsp; movq %1, %%rdi; jmp *%0" ::  "d"(method), "a"(p), "b"((uintptr_t)top)
    //   "movq %2, %%rsp; movq %1, %%rdi; callq *%0" ::  "d"(method), "a"(p), "b"((uintptr_t)top)
      "movq %1, %%rdi; callq *%0" ::  "d"(method), "a"(p), "b"((uintptr_t)top)
#endif
    );
    printf("Jmp end\n");
    return 0;
}