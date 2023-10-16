#include <common.h>
#include <os.h>

typedef struct _event_handler_node_t {
  int seq;
  int event;
  handler_t handler;
  struct _event_handler_node_t * next; 
} event_handler_node_t;

static event_handler_node_t *event_handler_head;
static struct cpu cpus[8];
struct cpu *mycpu() {
  return &cpus[cpu_current()];
}
spinlock_t trap_lock;
static void os_init() {
  printf("cpu count:%d\n", cpu_count());
  pmm->init();
  kmt->init();
  kmt->spin_init(&trap_lock, "trap");
  // dev->init();
  uproc->init();
}
#if TEST_ALLOC
#define TEST_SIZE 100
static void simple_test(int cpu) {
  void * addrs[TEST_SIZE];
  for(int i = 0; i < TEST_SIZE; i ++) {
    addrs[i] = pmm->alloc(i + 1 + cpu * 1000);
  }
  for(int i = 2; i >= 0; i --) {
    if(addrs[i])
      pmm->free(addrs[i]);
  }
  for(int i = 3; i < TEST_SIZE; i ++) {
    if(addrs[i])
      pmm->free(addrs[i]);
  }
}
#endif
static void os_run() {
  for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
    putch(*s == '*' ? '0' + cpu_current() : *s);
  }
  #if TEST_ALLOC
  simple_test(cpu_current());
  simple_test(cpu_current());
  #endif
  int c = cpu_current();
  cpus[c].ncli = 0;
  iset(true);
  while(1){
    // printf("Idle on cpu %d\n", cpu_current());
  }
}
static Context *os_trap(Event ev, Context *ctx) {
  printf("os trap:%d %s\n", ev.event, ev.msg);
  panic_on(ev.event == EVENT_ERROR, ev.msg);
  Context *next = NULL;
  kmt->spin_lock(&trap_lock);
  switch(ev.event) {
    case EVENT_PAGEFAULT:
    printf("page fault\n");
    break;
    case EVENT_SYSCALL:
    break;
    default:
    
  }
  for(event_handler_node_t *h = event_handler_head; h; h = h->next) {
    if (h->event == EVENT_NULL || h->event == ev.event) {
      Context *r = h->handler(ev, ctx);
      panic_on(r && next, "returning multiple contexts");
      if (r) next = r;
    }
  }
  kmt->spin_unlock(&trap_lock);
  panic_on(!next, "returning NULL context");
  return next;
}
static void os_on_irq(int seq, int event, handler_t handler) {
  // printf("os_on_irq %d \n", seq);
  event_handler_node_t * new_node = pmm->alloc(sizeof(event_handler_node_t));
  assert(new_node);
  new_node->seq = seq;
  new_node->event = event;
  new_node->handler = handler;
  new_node->next = NULL;
  if(event_handler_head == NULL) {
    // printf("os_on_irq set head\n");
    event_handler_head = new_node;
  } else {
    event_handler_node_t * p = event_handler_head;
    while(p && p->seq < new_node->seq)
      p = p->next;
    // printf("os_on_irq find node %p of seq >= %d \n", p, seq);
    if(p) {
      new_node->next = p->next;
      p->next = new_node;
    } else {
      // printf("os_on_irq append to the tail\n");
      event_handler_node_t * q = event_handler_head;
      while(q->next)
        q = q->next;
      q->next = new_node;
    }
  }
}
MODULE_DEF(os) = {
  .init = os_init,
  .run  = os_run,
  .trap = os_trap,
  .on_irq = os_on_irq,
};
