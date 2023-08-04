#include <common.h>
#define THREAD_STACK_SIZE 4096 * 3
struct task {
  union {
    struct {
      const char *name;
      int id;
      struct task *next;
      void      (*entry)(void *);
      Context    *context;
    };
    uint8_t stack[THREAD_STACK_SIZE];
  };
};

struct spinlock {
  // TODO
};

struct semaphore {
  // TODO
};
