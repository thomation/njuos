#include <common.h>
#define THREAD_STACK_SIZE 4096 * 3
enum task_status {
  TASK_STATUS_NONE = 0,
  TASK_STATUS_RUNNING = 1,
  TASK_STATUS_READY = 2,
  TASK_STATUS_BLOCK = 3,
};
struct cpu {
  int ncli;
  bool enable;
};
struct cpu *mycpu();
struct task {
  union {
    struct {
      const char *name;
      int id;
      enum task_status status;
      int cpu;
      struct task *next;
      void      (*entry)(void *);
      Context    *context;
    };
    uint8_t stack[THREAD_STACK_SIZE];
  };
};

struct spinlock {
  int locked;
  struct cpu *cpu;
  const char * name;
};

struct semaphore {
  // TODO
};

