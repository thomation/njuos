#include <common.h>
#include <os.h>

static task_t * task_list_head;

task_t * get_task_list_head() {
    return task_list_head;
}
void append_task(task_t * task) { 
  if(task_list_head == NULL) {
    task_list_head = task;
    task_list_head->next = NULL;
  }
  task_t * p = task_list_head;
  while(p->next != NULL)
    p = p->next;
  p -> next = task;
  task->next = NULL;
}
void print_tasks() {
  for(task_t * p = task_list_head; p; p = p->next) {
    printf("task: id=%d, name=%s, status=%d, cpu=%d, context=%p\n", p->id, p->name, p->status, p->cpu, &p->context);
  }
}

task_t * get_current_task() {
  int cpu = cpu_current();
  for(task_t * p = task_list_head; p; p = p->next) {
    if(p->cpu != cpu) continue;
      return p;
  }
  return NULL;
}