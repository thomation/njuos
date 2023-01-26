
#include <stdlib.h>
#include <stdio.h>
#include "process.h"

struct ProcessLink * create_process_link()
{
    return malloc(sizeof(struct ProcessLink));
}
void destroy_process_link(struct ProcessLink * link)
{
    // TODO
}
struct ProcessInfo * create_process_info(struct ProcessLink * link)
{
    struct ProcessInfo * info = malloc(sizeof(struct ProcessInfo));
    if(info)
    {
        info->id = -1;
        info->parent_id = -1;
        info->state = 0;
        info->next = link->tail;
        link->tail = info;
    }
    return info;
}
void print_process_info(struct ProcessInfo * info)
{
    printf("id:%d, comm:%s, state:%c, ppid:%d\n", info->id, info->comm, info->state, info->parent_id);
}