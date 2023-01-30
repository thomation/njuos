#include <game.h>

#define FPS 30

GamePlay g_game;

void init_game()
{
  g_game.pos_x = 0.0;
  g_game.pos_y = 0.0;
}
void update(int key)
{
  printf("update from key: %d\n", key);
  switch (key)
  {
  case AM_KEY_A:
    g_game.pos_x -= 0.1;
    if (g_game.pos_x < 0.0)
      g_game.pos_x = 0.0;
    break;
  case AM_KEY_D:
    g_game.pos_x += 0.1;
    if (g_game.pos_x > 1.0)
      g_game.pos_x = 1.0;
    break;
  case AM_KEY_W:
    g_game.pos_y -= 0.1;
    if (g_game.pos_y < 0.0)
      g_game.pos_y = 0.0;
    break;
  case AM_KEY_S:
    g_game.pos_y += 0.1;
    if (g_game.pos_y > 1.0)
      g_game.pos_y = 1.0;
    break;
  default:
    break;
  }
}
// Operating system is a C program!
int main(const char *args)
{
  ioe_init();

  puts("mainargs = \"");
  puts(args); // make run mainargs=xxx
  puts("\"\n");

  splash();
  int width = io_read(AM_GPU_CONFIG).width;
  int height = io_read(AM_GPU_CONFIG).height;
  printf("screen size = (%d, %d) %s\n", -width, height, "pixel");

  unsigned long last = 0;
  while (1)
  {
    unsigned long upt = io_read(AM_TIMER_UPTIME).us / 1000;
    if (upt - last > 1000 / FPS)
    {
      int key = handle_input();
      if (key == AM_KEY_ESCAPE)
        break;
      if (key)
        update(key);
      redraw();
      last = upt;
    }
  }
  return 0;
}
