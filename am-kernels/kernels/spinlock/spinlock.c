#include <am.h>
#include <klib.h>
struct cpu {
  int ncli;
  bool enable;
};
struct cpu cpus[8];
struct cpu *mycpu() {
  return &cpus[cpu_current()];
}
typedef struct spinlock{
  int locked;
  struct cpu *cpu;
} spinlock_t;
static spinlock_t mylock;
void initlock(spinlock_t * lk) {
  lk->locked = 0;
  lk->cpu = NULL;
}
void lock(spinlock_t *lk) {
  bool enable = ienabled();
  iset(false);
  if(mycpu()->ncli == 0)
    mycpu()->enable = enable;
  mycpu()->ncli ++;
  while (atomic_xchg(&(lk->locked), 1));
  lk->cpu = mycpu();
}

void unlock(spinlock_t *lk) {
  atomic_xchg(&lk->locked, 0);
  lk->cpu = NULL;
  mycpu()->ncli --;
  if(mycpu()->ncli == 0 && mycpu()->enable)
    iset(mycpu()->enable);
}

Context *on_interrupt(Event ev, Context *ctx) {
  switch (ev.event) {
    case EVENT_IRQ_TIMER:
      lock(&mylock);
      printf("Interrupt %d\n", cpu_current());
      unlock(&mylock);
      break;
    default:
      break;
  }
  assert(!ienabled());  // Interrupt disabled in on_interrupt()
  return ctx;
}

void os_main() {
  int c = cpu_current();
  cpus[c].ncli = 0;
  iset(true);
  while (1) {
    lock(&mylock);
    printf("Hello from CPU %d, ienabled %d\n", c, ienabled());
    unlock(&mylock);
    for(volatile int i = 0; i < 10000000; i ++ )
      ;
  }
}

int main() {
  initlock(&mylock);
  cte_init(on_interrupt);
  mpe_init(os_main);
}