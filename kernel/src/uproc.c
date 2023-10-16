#include <os.h>
#include <syscall.h>
#include <task.h>

#include "initcode.inc"
static int next_uproc_id;
static void * vme_alloc(int size) {
  printf("vme_alloc size %d\n", size);
  void * ret = pmm->alloc(size);
  printf("vme_alloc addr %p\n", ret);
  return ret;
}
static void vme_free(void * content) {
    pmm->free(content);
}
static int create(task_t * task, char* name, int cpu, enum task_status status) {
  protect(&task->as);
  task->name = name;
  task->id = next_uproc_id ++;
  task->status = status;
  task->cpu = cpu;
  task->next = NULL;
  void * entry = task->as.area.start;
  task->entry = entry;
  Area stack  = (Area) { task->stack, task->stack + THREAD_STACK_SIZE};
  printf("uproc create as pgsize:%d, start:%p\n", task->as.pgsize, task->as.area.start);
  task->context = *ucontext(&task->as, stack, entry);
  append_task(task);
  printf("uproc task %d, context %p, stack (%p, %p)\n", task->id, task->context, task->stack, task->stack + THREAD_STACK_SIZE);
  print_tasks();
  return task->id;
}
static void uproc_init() {
  printf("uproc_init \n");
  vme_init(vme_alloc, vme_free);
  task_t * task = pmm->alloc(sizeof(task_t));
  create(task, "init", 0, TASK_STATUS_READY);
}
int kputc(task_t *task, char ch) {
  putch(ch); // safe for qemu even if not lock-protected
  return 0;
}

MODULE_DEF(uproc) = {
  .init = uproc_init,
  .kputc = kputc,
};
