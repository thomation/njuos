#include <os.h>
#define INT_MIN	(-INT_MAX - 1)
#define INT_MAX	32767
static int next_thread_id;
task_t * task_list_head;
task_t * task_list_tail;
static Context *kmt_context_save(Event ev, Context *context) {
  int cpu = cpu_current();
  printf("kmt_context_save cpu %d context %p >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", cpu, context);
  for(task_t * p = task_list_head; p; p = p->next) {
    if(p->cpu != cpu) continue;
    if(p->status == TASK_STATUS_RUNNING) {
      p->context = context;
      printf("kmt_context_save %d current is %s\n", cpu, p->name);
      break;
    }
  }
  printf("kmt_context_save %d <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n", cpu);
  return NULL;
}
static Context *kmt_schedule(Event ev, Context *context) {
  int cpu = cpu_current();
  printf("kmt_schedule cpu %d, context %p >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", cpu, context);
  task_t * current = NULL;
  for(task_t * p = task_list_head; p; p = p->next) {
    if(p->status == TASK_STATUS_RUNNING && p->cpu == cpu) {
      current = p;
      printf("kmt_schedule %d current is %s\n", cpu, p->name);
      break;
    }
  }
  task_t * next = NULL;
  for(task_t * p = task_list_head; p; p = p->next) {
    if(p->status == TASK_STATUS_READY) {
      next = p;
      printf("kmt_schedule %d next is %s\n", cpu, p->name);
      break;
    }
  }
  Context * new_context = context;
  if(current && next) {
    current->status = TASK_STATUS_READY;
    next->status = TASK_STATUS_RUNNING;
    next->cpu = cpu;
    new_context = next->context;
  } else if(current && !next) {
    // schedule current again
  } else if(!current && next) {
    next->status = TASK_STATUS_RUNNING;
    next->cpu = cpu;
    new_context = next->context;
  } else {
    // no task
  }
  printf("kmt_schedule cpu %d context %p <<<<<<<<<<<<<<<<<<<<<<<\n", cpu, new_context);
  return new_context;
}
static void kmt_init() {
  printf("kmt init\n");
  task_list_head = pmm->alloc(sizeof(task_t));
  task_list_head->name = "Idle";
  task_list_head->id = next_thread_id ++;
  task_list_head->status = TASK_STATUS_NONE;
  task_list_head->cpu = -1;
  task_list_head->next = NULL;
  task_list_head->context = NULL;
  task_list_tail = task_list_head;
  os->on_irq(INT_MIN, EVENT_NULL, kmt_context_save);
  os->on_irq(INT_MAX, EVENT_NULL, kmt_schedule);
}
static void print_tasks() {
  for(task_t * p = task_list_head; p; p = p->next) {
    printf("task: id=%d, name=%s, status=%d, cpu=%d, context=%p\n", p->id, p->name, p->status, p->cpu, p->context);
  }
}
int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg) {
  printf("kmt_create task:%s\n", name);
  task->name = name;
  task->id = next_thread_id ++;
  task->status = TASK_STATUS_READY;
  task->cpu = -1;
  task->next = NULL;
  task->entry = entry;
  Area stack  = (Area) { task->stack, task->stack + THREAD_STACK_SIZE};
  task->context = kcontext(stack, task->entry, (void *)task->name);
  task_list_tail->next = task;
  task_list_tail = task_list_tail->next;
  printf("task %d, context %p, stack (%p, %p)\n", task->id, task->context, task->stack, task->stack + THREAD_STACK_SIZE);
  print_tasks();
  return task->id;
}

void kmt_spin_init(spinlock_t *lk, const char *name) {
  lk->locked = 0;
  lk->cpu = NULL;
  lk->name = name;
}
void kmt_spin_lock(spinlock_t *lk) {
  bool enable = ienabled();
  iset(false);
  if(mycpu()->ncli == 0)
    mycpu()->enable = enable;
  mycpu()->ncli ++;
  while (atomic_xchg(&(lk->locked), 1));
  lk->cpu = mycpu();
}
void kmt_spin_unlock(spinlock_t *lk) {
  atomic_xchg(&lk->locked, 0);
  lk->cpu = NULL;
  mycpu()->ncli --;
  if(mycpu()->ncli == 0 && mycpu()->enable)
    iset(mycpu()->enable);
}

MODULE_DEF(kmt) = {
  .init  = kmt_init,
  .create = kmt_create,
  .spin_init = kmt_spin_init,
  .spin_lock = kmt_spin_lock,
  .spin_unlock = kmt_spin_unlock,
};
