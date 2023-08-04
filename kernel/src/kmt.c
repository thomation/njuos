#include <os.h>
#define INT_MIN	(-INT_MAX - 1)
#define INT_MAX	32767
static int next_thread_id;
task_t * task_list_head;
task_t * task_list_tail;
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
  task_list_head = pmm->alloc(sizeof(task_t));
  task_list_head->name = "Idle";
  task_list_head->id = next_thread_id ++;
  task_list_head->next = NULL;
  task_list_tail = task_list_head;
  os->on_irq(INT_MIN, EVENT_NULL, kmt_context_save);
  os->on_irq(INT_MAX, EVENT_NULL, kmt_schedule);
}
int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg) {
  Area stack    = (Area) { task->stack, task->stack + THREAD_STACK_SIZE};
  task->context = kcontext(stack, task->entry, (void *)task->name);
  task->id = next_thread_id ++;
  task_list_tail->next = task;
  task_list_tail = task_list_tail->next;
  return task->id;
}


MODULE_DEF(kmt) = {
  .init  = kmt_init,
  .create = kmt_create,
};
