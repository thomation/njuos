#include <os.h>
#define INT_MIN	(-INT_MAX - 1)
#define INT_MAX	32767
static Context *kmt_context_save(Event ev, Context *context) {
  // printf("kmt_context_save %d\n", cpu_current());
  return NULL;
}
static Context *kmt_schedule(Event ev, Context *context) {
  // printf("kmt_schedule %d\n", cpu_current());
  return context;
}
static void kmt_init() {
  printf("kmt init\n");
  os->on_irq(INT_MIN, EVENT_NULL, kmt_context_save);
  os->on_irq(INT_MAX, EVENT_NULL, kmt_schedule);
}
MODULE_DEF(kmt) = {
  .init  = kmt_init,
};
