
#include <stdlib.h>
#include <stdio.h>
#include "process.h"

struct ProcessLink * create_process_link()
{
    struct ProcessLink *link = malloc(sizeof(struct ProcessLink));
    if(link)
    {
        link->head = 0;
        link->tail = 0;
    }
    return link;
}
void destroy_process_link(struct ProcessLink * link)
{
    // TODO
}
void print_process_link(struct ProcessLink * link)
{
    struct ProcessInfo *info = link->head;
    while(info)
    {
        print_process_info(info);
        info = info->next;
    }
}
struct ProcessInfo * create_process_info(struct ProcessLink * link)
{
    struct ProcessInfo * info = malloc(sizeof(struct ProcessInfo));
    if(info)
    {
        info->id = -1;
        info->parent_id = -1;
        info->state = 0;
        info->next = 0;
        if(!link->head)
        {
            link->head = info;
            link->tail = info;
        }
        else
        {
            link->tail->next = info;
            link->tail = info;
        }
    }
    return info;
}
void print_process_info(struct ProcessInfo * info)
{
    printf("id:%d, comm:%s, state:%c, ppid:%d\n", info->id, info->comm, info->state, info->parent_id);
}