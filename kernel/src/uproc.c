#include <os.h>
#include <syscall.h>
#include <task.h>

#include "initcode.inc"

#define __DEBUG
#ifdef __DEBUG
#define DEBUG(format,...) printf(""format"", ##__VA_ARGS__)  
#else
#define DEBUG(...)
#endif
static int next_uproc_id;
static void * vme_alloc(int size) {
  DEBUG("vme_alloc size %d\n", size);
  uintptr_t ret = (uintptr_t)pmm->alloc(size * 2);
  DEBUG("vme_alloc raw addr %p\n", ret);
  ret = ret & ~(0xfff);
  DEBUG("vme_alloc new addr %p\n", ret);
  return (void*)ret;
}
static void vme_free(void * content) {
  DEBUG("vm_free:%p\n", content);
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
  DEBUG("uproc stack %p, %p\n", stack.start, stack.end);
  DEBUG("uproc create as pgsize:%d, start:%p, ptr:%p\n", task->as.pgsize, task->as.area.start, task->as.ptr);
  task->context = *ucontext(&task->as, stack, entry);
  DEBUG("uproc task context:%p, cr3 %p, rsp0:%p\n", &task->context, task->context.cr3, task->context.rsp0);
  append_task(task);
  print_tasks();
  return task->id;
}
static void pgmap(task_t * task, void * va, void * pa) {
  DEBUG("uproc_pgmap %p -> %p\n", va, pa);
  task->va[task->np] = va;
  task->pa[task->np] = pa;
  task->np ++;
  assert(task->np < 64);
  map(&task->as, va, pa, MMAP_WRITE | MMAP_READ);
}
static Context *uproc_pagefault(Event ev, Context *context) {
  DEBUG("uproc_pagefalut %p by %p\n", ev.ref, context->rip);
  task_t * task = get_current_running_task();
  DEBUG("uproc_pagefalut current task:%s\n", task->name);
  void * pa = vme_alloc(task->as.pgsize);
  void * va = (void*)(ev.ref & ~(task->as.pgsize - 1));
  if(va == task->as.area.start) {
    DEBUG("uproc_pagefalut load code\n");
    memcpy(pa, _init, _init_len);
  }
  pgmap(task, va, pa);
  return NULL;
}
static Context * uproc_syscall(Event ev, Context * context) {
  // iset(true);
  int no = context->GPRx;
  task_t * task = get_current_running_task();
  DEBUG("uproc syscall: %d\n", no);
  switch(no) {
    case SYS_kputc:   
      putch(context->GPR1);
      break;
    case SYS_exit:
      task->status = TASK_STATUS_DEATH;
      break;
  }
  // iset(false);
  return NULL; 
}
static void uproc_init() {
  printf("uproc_init \n");
  vme_init(vme_alloc, vme_free);
  task_t * task = pmm->alloc(sizeof(task_t));
  create(task, "init", 0, TASK_STATUS_READY);
  os->on_irq(10, EVENT_PAGEFAULT, uproc_pagefault);
  os->on_irq(20, EVENT_SYSCALL, uproc_syscall);
}
int kputc(task_t *task, char ch) {
  putch(ch); // safe for qemu even if not lock-protected
  return 0;
}

MODULE_DEF(uproc) = {
  .init = uproc_init,
  .kputc = kputc,
};
