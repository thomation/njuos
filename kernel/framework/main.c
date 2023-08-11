#include <klib.h>
#include <common.h>
#include <os.h>

static inline task_t *task_alloc() {
  return pmm->alloc(sizeof(task_t));
}
// #define TEST_SEM
#ifdef TEST_TASK
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
  task_t *t1 = task_alloc();
  task_t *t2 = task_alloc();
  task_t *t3 = task_alloc();
  kmt->create(t1, "test-thread-1", func, "1");
  kmt->create(t2, "test-thread-2", func, "2");
  kmt->create(t3, "test-thread-3", func, "3");
  // kmt->teardown(t1);
}
#endif
#ifdef TEST_SEM
sem_t empty, fill;
#define P kmt->sem_wait
#define V kmt->sem_signal
void Tproduce(void *arg) {
  uintptr_t t = (uintptr_t)arg; 
  while (1) {
    P(&empty);
    putch(t + '0');
    V(&fill);
  }
}
void Tconsume(void *arg) {
  uintptr_t t = (uintptr_t)arg; 
  while (1) {
    P(&fill);
    putch('a' + t);
    V(&empty);
  }
}
void test_sem() {
  kmt->sem_init(&empty, "empty", 5);
  kmt->sem_init(&fill,  "fill",  0);
  for (uintptr_t i = 0; i < 2; i++) {
    kmt->create(task_alloc(), "producer", Tproduce, (void*)i);
  }
  for (uintptr_t i = 0; i < 2; i++) {
    kmt->create(task_alloc(), "consumer", Tconsume, (void*)i);
  }
}
#endif
int main() {
  ioe_init();
  cte_init(os->trap);
  os->init();
#ifdef TEST_TASK
  test_tasks();
#endif
#ifdef TEST_SEM
  test_sem();
#endif
  mpe_init(os->run);
  return 1;
}
