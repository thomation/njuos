#ifndef __TASK_H__
#define __TASK_H__
void init_task_list();
void append_task(task_t * task); 
void print_tasks(); 
task_t * get_current_task(); 
task_t * get_task_list_head();
#endif