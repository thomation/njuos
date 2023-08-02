#include <common.h>

static void os_init() {
  printf("cpu count:%d\n", cpu_count());
  pmm->init();
  kmt->init();
}
#if TEST_ALLOC
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
#endif
static void os_run() {
  for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
    putch(*s == '*' ? '0' + cpu_current() : *s);
  }
  #if TEST_ALLOC
  simple_test(cpu_current());
  simple_test(cpu_current());
  #endif
  iset(true);
  while (1) ;
}
static Context *os_trap(Event ev, Context *ctx) {
  printf("on_interrupt event %d\n", ev.event);
  return ctx;
}
static void os_on_irq(int seq, int event, handler_t handler) {

}
MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
  .trap = os_trap,
  .on_irq = os_on_irq,
};
