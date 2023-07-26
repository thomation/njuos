#include <common.h>

static void os_init() {
  pmm->init();
}
#define TEST_SIZE 10
static void simple_test() {
  void * addrs[TEST_SIZE];
  for(int i = 0; i < TEST_SIZE; i ++) {
    addrs[i] = pmm->alloc(i + 1);
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
  printf("cpu count:%d\n", cpu_count());
  for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
    putch(*s == '*' ? '0' + cpu_current() : *s);
  }
  printf("test start %d\n", cpu_current());
  simple_test();
  simple_test();
  printf("test end %d\n", cpu_current());
  while (1) ;
}

MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
};
