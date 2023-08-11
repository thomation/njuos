#include <os.h>
// #define __DEBUG
#ifdef __DEBUG
#define DEBUG(format,...) printf("File: "__FILE__", Line: %05d: "format"/n", __LINE__, ##__VA_ARGS__)  
#else
#define DEBUG(...)
#endif
#define INT_MIN	(-INT_MAX - 1)
#define INT_MAX	32767
static int next_thread_id;
task_t * task_list_head;
task_t * task_list_tail;
static task_t * get_current_task() {
  int cpu = cpu_current();
  for(task_t * p = task_list_head; p; p = p->next) {
    if(p->cpu != cpu) continue;
      return p;
  }
  return NULL;
}
static Context *kmt_context_save(Event ev, Context *context) {
  DEBUG("kmt_context_save cpu %d context %p >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", cpu_current(), context);
  task_t * p = get_current_task();
  if(p) {
      p->context = context;
      DEBUG("kmt_context_save %d current is %s\n", cpu_current(), p->name);
  }
  DEBUG("kmt_context_save %d <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n", cpu_current());
  return NULL;
}
static Context *kmt_schedule(Event ev, Context *context) {
  DEBUG("kmt_schedule cpu %d, context %p >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", cpu_current(), context);
  task_t * current = get_current_task();
  task_t * next = NULL;
  // search from current
  if(current) {
    for(task_t * p = current; p; p = p->next) {
      if(p->status == TASK_STATUS_READY) {
        next = p;
        DEBUG("kmt_schedule %d next is %s\n", cpu_current(), p->name);
        break;
      }
    }
  }
  // search from head
  if(!next) {
    for(task_t * p = task_list_head; p; p = p->next) {
      if(p->status == TASK_STATUS_READY) {
        next = p;
        DEBUG("kmt_schedule %d next is %s\n", cpu_current(), p->name);
        break;
      }
    }
  }
  Context * new_context = context;
  if(current && next) {
    current->status = TASK_STATUS_READY;
    current->cpu = -1;
    next->status = TASK_STATUS_RUNNING;
    next->cpu = cpu_current();
    new_context = next->context;
  } else if(current && !next) {
    // schedule current again
  } else if(!current && next) {
    next->status = TASK_STATUS_RUNNING;
    next->cpu = cpu_current();
    new_context = next->context;
  } else {
    // no task
  }
  DEBUG("kmt_schedule cpu %d context %p <<<<<<<<<<<<<<<<<<<<<<<\n", cpu_current(), new_context);
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

void kmt_teardown(task_t *task) {
  while(task->status != TASK_STATUS_DEATH)
    // wait
    printf("wait %s\n", task->name);
    ; 
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
