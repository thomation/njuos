#include <common.h>
#define THREAD_STACK_SIZE 4096 * 3
enum task_status {
  TASK_STATUS_NONE = 0,
  TASK_STATUS_RUNNING = 1,
  TASK_STATUS_READY = 2,
  TASK_STATUS_BLOCK = 3,
  TASK_STATUS_DEATH = 4,
};
struct cpu {
  int ncli;
  bool enable;
};
struct cpu *mycpu();
typedef struct task_node {
  task_t * task;
  struct task_node * next;
} task_node_t;
struct task {
  union {
    struct {
      const char *name;
      int id;
      enum task_status status;
      int cpu;
      struct task *next;
      void    (*entry)(void *);
      Context  context;
      AddrSpace as;
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
  spinlock_t lock;
  int count;
  const char *name;
  task_node_t * wait_list;
};

