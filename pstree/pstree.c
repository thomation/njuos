#include <stdio.h>
#include <assert.h>
#include <dirent.h>

void travel_proc()
{
  DIR *proc = opendir("/proc");
  assert(proc);
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
