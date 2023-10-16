#include <os.h>
#include <task.h>

#define __DEBUG
#ifdef __DEBUG
#define DEBUG(format,...) printf(""format"", ##__VA_ARGS__)  
#else
#define DEBUG(...)
#endif
#define INT_MIN	(-INT_MAX - 1)
#define INT_MAX	32767
static int next_thread_id;


static Context *kmt_context_save(Event ev, Context *context) {
  DEBUG("kmt_context_save cpu %d context %p >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", cpu_current(), context);
  task_t * p = get_current_task();
  assert(p); // There is idle task that cannot be blocked
  p->context = *context;
  DEBUG("kmt_context_save %d current is %s, status %d\n", cpu_current(), p->name, p->status);
  DEBUG("kmt_context_save %d <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n", cpu_current());
  return NULL;
}
static bool can_schedule(task_t *task)
{
  return task->status == TASK_STATUS_READY && (task->cpu == -1 || task->cpu == cpu_current());
}
static Context *kmt_schedule(Event ev, Context *context) {
  DEBUG("kmt_schedule cpu %d, context %p >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", cpu_current(), context);
  task_t * current = get_current_task();
  assert(current);
  task_t * next = NULL;
  // search from current
  for(task_t * p = current; p; p = p->next) {
    if(can_schedule(p)) {
      next = p;
      DEBUG("kmt_schedule %d next is %s\n", cpu_current(), p->name);
      break;
    }
  }
  // search from head
  if(!next) {
    for(task_t * p = get_task_list_head(); p; p = p->next) {
      if(can_schedule(p)) {
        next = p;
        DEBUG("kmt_schedule %d next is %s\n", cpu_current(), p->name);
        break;
      }
    }
  }
  Context * new_context = context;
  if(next) {
    if(current->status == TASK_STATUS_RUNNING)
      current->status = TASK_STATUS_READY;
    current->cpu = -1;
    next->status = TASK_STATUS_RUNNING;
    next->cpu = cpu_current();
    new_context = &next->context;
  } else if(!next) {
    // schedule current again
    assert(current->status == TASK_STATUS_RUNNING);
  } else {
    panic("no task");
    // no task
  }
  DEBUG("kmt_schedule cpu %d context %p <<<<<<<<<<<<<<<<<<<<<<<\n", cpu_current(), new_context);
  return new_context;
}

int do_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg, int cpu, enum task_status status) {
  printf("do_create task:%s, arg:%p\n", name, arg);
  task->name = name;
  task->id = next_thread_id ++;
  task->status = status;
  task->cpu = cpu;
  task->next = NULL;
  task->entry = entry;
  Area stack  = (Area) { task->stack, task->stack + THREAD_STACK_SIZE};
  task->context = *kcontext(stack, task->entry, arg);
  append_task(task);
  printf("kernel task %d, context %p, stack (%p, %p)\n", task->id, task->context, task->stack, task->stack + THREAD_STACK_SIZE);
  print_tasks();
  return task->id;
}
static void kmt_init() {
  printf("kmt init\n");
  task_t * head = pmm->alloc(sizeof(task_t));
  do_create(head, "Head", NULL, NULL, -1, TASK_STATUS_NONE);
  for(int i = 0; i < cpu_count(); i ++) {
    do_create(pmm->alloc(sizeof(task_t)), "Idle", NULL, NULL, i, TASK_STATUS_READY);
  }
  os->on_irq(INT_MIN, EVENT_NULL, kmt_context_save);
  os->on_irq(INT_MAX, EVENT_NULL, kmt_schedule);
}
int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg) {
  printf("kmt_create task:%s\n", name);
  do_create(task, name, entry, arg, -1, TASK_STATUS_READY);
  return task->id;
}
void kmt_teardown(task_t *task) {
  while(task->status != TASK_STATUS_DEATH)
    // wait
    printf("wait %s\n", task->name);
    ; 
  // TODO: remove from task list
  // pmm->free(task);
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
void kmt_sem_init(sem_t *sem, const char *name, int value) {
  kmt_spin_init(&sem->lock, name);
  sem->count = value;
  sem->name = name;
  sem->wait_list = NULL;
}
void kmt_sem_wait(sem_t *sem) {
  DEBUG("kmt_sem_wait %s, at cpu %d\n", sem->name, cpu_current());
  kmt_spin_lock(&sem->lock);
  sem->count--;
  task_t * cur = NULL;
  if (sem->count < 0) {
    cur = get_current_task();
    cur->status = TASK_STATUS_BLOCK;
    task_node_t * node = pmm->alloc(sizeof(task_node_t));
    node->task = cur;
    node->next = NULL;
    if(!sem->wait_list) {
      sem->wait_list = node;
    } else {
      task_node_t * p = sem->wait_list;
      while(p->next)
        p = p->next;
      p->next = node;
    }
  }
  kmt_spin_unlock(&sem->lock);
  // cur may be set READY by the signal
  if (cur && cur->status == TASK_STATUS_BLOCK) {
    yield();
  }
}
void kmt_sem_signal(sem_t *sem) {
  DEBUG("kmt_sem_signal %s, at cpu %d\n", sem->name, cpu_current());
  kmt_spin_lock(&sem->lock);
  sem->count ++;
  if(sem->wait_list) {
    task_node_t * p = sem->wait_list;
    sem->wait_list = p->next;
    p->task->status = TASK_STATUS_READY;
    pmm->free(p);
  }
  kmt_spin_unlock(&sem->lock);
}

MODULE_DEF(kmt) = {
  .init  = kmt_init,
  .create = kmt_create,
  .teardown = kmt_teardown,
  .spin_init = kmt_spin_init,
  .spin_lock = kmt_spin_lock,
  .spin_unlock = kmt_spin_unlock,
  .sem_init = kmt_sem_init,
  .sem_wait = kmt_sem_wait,
  .sem_signal = kmt_sem_signal,
};
