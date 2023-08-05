#include <klib.h>
#include <common.h>
#include <os.h>

static void producer(void *arg) {
  printf("producer\n");
}
static void consumer(void *arg) {
  printf("consumer\n");
}
void test_tasks() {
  kmt->create(pmm->alloc(sizeof(struct task)), "test-thread-1", producer, "1");
  kmt->create(pmm->alloc(sizeof(task_t)), "test-thread-2", consumer, "2");
}
int main() {
  ioe_init();
  cte_init(os->trap);
  os->init();
  test_tasks();
  mpe_init(os->run);
  return 1;
}
