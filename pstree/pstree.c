#include <stdio.h>
#include <assert.h>
#include <dirent.h>
#include <string.h>
#include "process.h"

enum Option
{
  NONE = 0x0,
  OPTION_N = 0x1,
  OPTION_P = 0x2,
};

enum Option parse_option(char * option)
{
    if(strcmp(option,"-p"))
      return OPTION_P;
    if(strcmp(option, "-n"))
      return OPTION_N;
    printf("unsupport option %sn", option);
    return NONE;
}
int is_proc_dir(char * dir)
{
  char * p = dir;
  while(*p != '\0')
  {
    if(*p < '0' || *p >'9')
      return 0;
    p ++;
  }
  return 1;
}
int get_process_info(char * proc_dir, struct ProcessInfo * info)
{
  char filename[200];
  strncpy(filename, proc_dir, 200);
  strncat(filename,"/comm", 100);
  FILE *fp = fopen(filename, "r");
  if (fp) {
    fscanf(fp, "%s", info->comm);
    fclose(fp);
  } else {
    printf("Cannot open file: %s\n", filename);
    return -1;
  }
  return 0;
}
void travel_proc(unsigned int option, struct ProcessLink * process_link)
{
  printf("travel_proc with option:%u\n", option);
  DIR *proc = opendir("/proc");
  assert(proc);
  struct dirent * item = NULL;
  while((item = readdir(proc)))
  {
    if(is_proc_dir(item->d_name) && item->d_type == 4)
    {
      struct ProcessInfo *info = create_process_info(process_link);
      sscanf(item->d_name, "%u", &info->id);
      char path[100] = "/proc/";
      strncat(path, item->d_name, 90);
      get_process_info(path, info);
      print_process_info(info);
    }
  }
  closedir(proc);
}
int main(int argc, char *argv[]) {
  unsigned int option = 0;
  for (int i = 0; i < argc; i++) {
    assert(argv[i]);
    option |= parse_option(argv[i]);
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  assert(!argv[argc]);
  struct ProcessLink *process_link = create_process_link();
  travel_proc(option, process_link);
  destroy_process_link(process_link);
  return 0;
}
