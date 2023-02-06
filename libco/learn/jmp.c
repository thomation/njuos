#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <setjmp.h>

static jmp_buf context;
static int method(void *param)
{
  printf("Hello jmp start\n");
  uintptr_t i = (uintptr_t)param;
  printf("Hello jmp param\n");
#if __x86_64__
  printf("Hello Jmp %ld\n", i);
#else
  printf("Hello Jmp %d\n", i);
#endif
  // longjmp(context, 1);
  printf("Hello jmp return\n");
  return 1;
}
static void method_return()
{
  printf("method_return\n");
  longjmp(context, 1);
}
static inline void stack_switch_call(void *sp, void *entry, uintptr_t arg, void *exit)
{
  asm volatile(
#if __x86_64__
      "movq %0, %%rsp; movq %3, (%%rsp); movq %2, %%rdi; jmp *%1"
      :
      : "b"((uintptr_t)sp), "d"(entry), "a"(arg), "r"((uintptr_t)exit)
#else
      "movl %0, %%esp; movl %2, 4(%0); jmp *%1"
      :
      : "b"((uintptr_t)sp - 8), "d"(entry), "a"(arg)
#endif
  );
  printf("stack_switch_call end\n");
}
int main()
{
  int size = sizeof(u_int8_t) * 8192;
  u_int8_t *stack = malloc(size);
  u_int8_t *top = stack + size - 8;

  int val = setjmp(context);
  if (val == 0)
  {
    stack_switch_call(top, method, 7, method_return);
  }
  else
  {
  }
  printf("Jmp end\n");
  return 0;
}