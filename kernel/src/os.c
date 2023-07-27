#include <common.h>

static void os_init() {
  printf("cpu count:%d\n", cpu_count());
  pmm->init();
}
#define TEST_SIZE 100
static void simple_test(int cpu) {
  void * addrs[TEST_SIZE];
  for(int i = 0; i < TEST_SIZE; i ++) {
    addrs[i] = pmm->alloc(i + 1 + cpu * 1000);
  }
  for(int i = 2; i >= 0; i --) {
    if(addrs[i])
      pmm->free(addrs[i]);
  }
  for(int i = 3; i < TEST_SIZE; i ++) {
    if(addrs[i])
      pmm->free(addrs[i]);
  }
}
static void os_run() {
  for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
    putch(*s == '*' ? '0' + cpu_current() : *s);
  }
  simple_test(cpu_current());
  simple_test(cpu_current());
  while (1) ;
}

MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
};
