#ifndef __PROCESS_H__
#define __PROCESS_H__
struct ProcessInfo
{
  int id;
  char comm[100];
  char state;
  int parent_id;
  struct ProcessInfo * next;
};
struct ProcessLink
{
  struct ProcessInfo * head;
  struct ProcessInfo * tail;
};

struct ProcessLink* create_process_link();
void destroy_process_link(struct ProcessLink * link);
void print_process_link(struct ProcessLink * link);
struct ProcessInfo * create_process_info(struct ProcessLink* link);
void print_process_info(struct ProcessInfo * info);
#endif