#include <stdio.h>
#include <assert.h>
#include <dirent.h>

void travel_proc()
{
  DIR *proc = opendir("/proc");
  assert(proc);
  struct dirent * item = NULL;
  while(item = readdir(dir))
  {
    printf("name:%s\n", item->d_name);
  }
}
int main(int argc, char *argv[]) {
  for (int i = 0; i < argc; i++) {
    assert(argv[i]);
    printf("argv[%d] = %s\n", i, argv[i]);
    travel_proc();
  }
  assert(!argv[argc]);
  return 0;
}
