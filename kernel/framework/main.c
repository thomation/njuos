#include <klib.h>
#include <common.h>
#include <os.h>

static int locked = 0;
static void lock()   { while (atomic_xchg(&locked, 1)); }
static void unlock() { atomic_xchg(&locked, 0); }
static inline task_t *task_alloc() {
  return pmm->alloc(sizeof(task_t));
}
void func(void *arg) {
  while (1) {
    lock();
    printf("Thread-%s on CPU #%d\n", arg, cpu_current());
    unlock();
    for (int volatile i = 0; i < 100000; i++) ;
  }
}
void test_tasks() {
  task_t *t1 = task_alloc();
  task_t *t2 = task_alloc();
  task_t *t3 = task_alloc();
  kmt->create(t1, "test-thread-1", func, "1");
  kmt->create(t2, "test-thread-2", func, "2");
  kmt->create(t3, "test-thread-3", func, "3");
  // kmt->teardown(t1);
}
int main() {
  ioe_init();
  cte_init(os->trap);
  os->init();
  test_tasks();
  mpe_init(os->run);
  return 1;
}
