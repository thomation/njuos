#include <klib.h>
#include <common.h>
#include <os.h>

static int locked = 0;
static void lock()   { while (atomic_xchg(&locked, 1)); }
static void unlock() { atomic_xchg(&locked, 0); }

void func(void *arg) {
  while (1) {
    lock();
    printf("Thread-%s on CPU #%d\n", arg, cpu_current());
    unlock();
    for (int volatile i = 0; i < 100000; i++) ;
  }
}
void test_tasks() {
  kmt->create(pmm->alloc(sizeof(struct task)), "test-thread-1", func, "1");
  kmt->create(pmm->alloc(sizeof(task_t)), "test-thread-2", func, "2");
  kmt->create(pmm->alloc(sizeof(task_t)), "test-thread-3", func, "3");
}
int main() {
  ioe_init();
  cte_init(os->trap);
  os->init();
  test_tasks();
  mpe_init(os->run);
  return 1;
}
