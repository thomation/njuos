#include "co.h"
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

#define CO_LIST_SIZE 16
#define CO_MAIN 0

static int current = -1;
static struct co *co_list[CO_LIST_SIZE] = {NULL};
static int co_count = 0;

// #define ENABLE_DEBUG_PRINT
#ifdef ENABLE_DEBUG_PRINT
#define debug(...) printf(__VA_ARGS__)
#else
#define debug(...)
#endif

enum CoStatus
{
  CO_NEW,
  CO_RUNNING,
  CO_SCHEDULABLE = CO_RUNNING,
  CO_WAITING,
  CO_DEAD,
};
typedef struct _Stack
{
  uint8_t stack[8192];
} Stack;
typedef struct co
{
  char *name;
  void (*func)(void *);
  void *arg;
  enum CoStatus status;
  struct co *waiter;
  jmp_buf context;
  Stack stack;
} Co;

void co_destroy(Co * co)
{
  if (co)
  {
    debug("Destory %s\n", co->name);
    free(co);
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
  p->waiter = NULL;
  return p;
}
Co *co_start(const char *name, void (*func)(void *), void *arg)
{
  if (current == -1)
  {
    debug("Create main, current size:%d\n", co_count);
    Co *m = co_create("__MAIN__", NULL, NULL);
    m->status = CO_RUNNING;
    current = 0;
    co_list[current] = m;
    co_count++;
  }
  debug("Create co:%s, current size:%d\n", name, co_count);
  if (co_count >= CO_LIST_SIZE)
  {
    printf("Only support %d co\n", CO_LIST_SIZE);
    exit(-1);
  }
  Co *p = co_create(name, func, arg);
  p->status = CO_NEW;
  co_list[co_count++] = p;
  return p;
}

void co_wait(struct co *co)
{
  debug("wait %s\n", co->name);
  co->waiter = co_list[current];
  co->waiter->status = CO_WAITING;
  co_yield ();
  if(co->status == CO_DEAD)
  {
    co_destroy(co);
  }
}
int find_next()
{
  int valid_co[CO_LIST_SIZE];
  int valid_count = 0;
  for (int i = 0; i < co_count; i++)
  {
    struct co *p = co_list[i];
    if (p->status <= CO_SCHEDULABLE && i != current)
    {
      valid_co[valid_count++] = i;
    }
  }
  debug("valid count:%d\n", valid_count);
  if (valid_count <= 0)
    return current;
  int r = rand() % valid_count;
  return valid_co[r];
}
static void exit_co_func()
{
  debug("exit co func\n");
  int i = current;
  Co *p = co_list[current];
  p->status = CO_DEAD;
  longjmp(co_list[CO_MAIN]->context, 1);
}
static inline void stack_switch_call(void *sp, void *entry, uintptr_t arg, void *exit)
{
  asm volatile(
#if __x86_64__
      "movq %0, %%rsp; movq %3, (%%rsp); movq %2, %%rdi; jmp *%1"
      :
      : "b"((uintptr_t)sp), "d"(entry), "a"(arg), "r"((uintptr_t)exit)
#else
      "movl %0, %%esp; movl %2, 4(%0); jmp *%1"
      :
      : "b"((uintptr_t)sp - 8), "d"(entry), "a"(arg)
#endif
  );
  debug("stack_switch_call end\n");
}
static inline void *stack_top(Stack *s)
{
  return s->stack + sizeof(s->stack) - 8; // Simulate ret address to fix segment fault.
}
void run(int index)
{
  Co *p = co_list[index];
  debug("run:%s\n", p->name);
  p->status = CO_RUNNING;
  current = index;
  stack_switch_call(stack_top(&p->stack), p->func, (uintptr_t)p->arg, exit_co_func);
}
void co_yield ()
{
  Co *old = co_list[current];
  debug("co yield current:%d, %s\n", current, old->name);
  int val = setjmp(old->context);
  if (val == 0)
  {
    debug("set jump value:%d\n", val);
    int next = find_next();
    Co *new = co_list[next];
    debug("co yield to next:%d, %s\n", next, new->name);
    if (new->status == CO_NEW)
    {
      debug("co yield to next new:%d, %s, go_new\n", next, new->name);
      run(next);
    }
    else
    {
      // TODO: after new co is dead
      debug("co yield to next old:%d, %s, %d\n", next, new->name, new->status);
      longjmp(new->context, next + 1);
    }
  }
  else
  {
    debug("set jump value:%d\n", val);
    current = val - 1;
  }
}
