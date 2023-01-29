#include <game.h>

// Operating system is a C program!
int main(const char *args) {
  ioe_init();

  puts("mainargs = \"");
  puts(args); // make run mainargs=xxx
  puts("\"\n");

  splash();
  int width = io_read(AM_GPU_CONFIG).width;
  int height = io_read(AM_GPU_CONFIG).height;
  printf("screen size = (%d, %d) %s\n", -width, height, "pixel");
  puts("Press any key to see its key code...\n");
  while (1) {
    if(!print_key())
      break;
  }
  return 0;
}
