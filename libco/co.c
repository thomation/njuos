#include "co.h"
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

#define STACK_SIZE 8 * 1024 * 1024
#define CO_LIST_SIZE 16
#define CO_MAIN 0

static int current = -1;
static struct co *co_list[CO_LIST_SIZE] = {NULL};
static int co_count = 0;


#define ENABLE_DEBUG_PRINT
#ifdef ENABLE_DEBUG_PRINT
  #define debug(...) printf(__VA_ARGS__)
#else
  #define debug()
#endif

enum CoStatus
{
  CO_NEW,
  CO_RUNNING,
  CO_WAITING,
  CO_DEAD,
};
typedef struct co
{
  char *name;
  void (*func)(void *);
  void *arg;
  enum CoStatus status;
  struct co *waiter;
  jmp_buf context;
  unsigned char stack[STACK_SIZE];
} Co;

void co_destroy(int index)
{
  if(co_list[index])
  {
    debug("Destory %s", co_list[index]->name);
    free(co_list[index]);
    co_list[index] = NULL;
  }
}
static Co *co_create(const char *name, void (*func)(void *), void *arg)
{
  Co *p = malloc(sizeof(struct co));
  if (!p)
  {
    printf("Create co fail %s\n", name);
    exit(-1);
  }
  p->name = (char *)name;
  p->func = func;
  p->arg = arg;
  p-> waiter = NULL;
  return p;
}
Co *co_start(const char *name, void (*func)(void *), void *arg)
{
  if(current == -1)
  {
    debug("Create main, current size:%d\n", co_count);
    co_destroy(CO_MAIN);
    Co * m = co_create("__MAIN__", NULL, NULL);
    m->status = CO_RUNNING;
    current = 0;
    co_list[current] = m;
    co_count ++;
    co_yield();
  }
  debug("Create co:%s, current size:%d\n", name, co_count);
  if (co_count >= CO_LIST_SIZE)
  {
    printf("Only support %d co\n", CO_LIST_SIZE);
    exit(-1);
  }
  Co * p = co_create(name, func, arg);
  p->status = CO_NEW;
  co_list[co_count++] = p;
  return p;
}

void co_wait(struct co *co)
{
  debug("wait %s\n", co->name);
  co->status = CO_RUNNING;
  co->waiter = co_list[current];
  // co_yield();
}
int find_next()
{
  int valid_co[CO_LIST_SIZE];
  int valid_count = 0;
  for (int i = 0; i < co_count; i++)
  {
    struct co *p = co_list[i];
    if (p->status != CO_DEAD && i != current)
    {
      valid_co[valid_count++] = i;
    }
  }
  if (valid_count <= 0)
    return current;
  return valid_co[0];
}
static inline void stack_switch_call(void *sp, void *entry, uintptr_t arg) {
  asm volatile (
#if __x86_64__
    "movq %0, %%rsp; movq %2, %%rdi; jmp *%1"
      : : "b"((uintptr_t)sp), "d"(entry), "a"(arg) : "memory"
#else
    "movl %0, %%esp; movl %2, 4(%0); jmp *%1"
      : : "b"((uintptr_t)sp - 8), "d"(entry), "a"(arg) : "memory"
#endif
  );
}
void run(int index)
{
  Co * p = co_list[index];
  stack_switch_call(p->stack, p->func, (uintptr_t)p->arg);
  current = index;
}
void co_yield ()
{
  Co * old = co_list[current];
  int val = setjmp(old->context);
  if (val == 0) {
    int next = find_next();
    Co * new = co_list[next];
    if(new->status == CO_NEW)
    {
      run(next);
    }
    else
    {
      // TODO: after new co is dead
      longjmp(new->context, next + 1);
    }
  } else {
    current = val - 1;
    Co * p = co_list[current];
  }

}
