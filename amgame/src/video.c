#include <game.h>

#define SIDE 16
static int w, h;

static void init()
{
  AM_GPU_CONFIG_T info = {0};
  ioe_read(AM_GPU_CONFIG, &info);
  w = info.width;
  h = info.height;
}

static void draw_tile(int x, int y, int w, int h, uint32_t color)
{
  uint32_t pixels[w * h]; // WARNING: large stack-allocated memory
  AM_GPU_FBDRAW_T event = {
      .x = x,
      .y = y,
      .w = w,
      .h = h,
      .sync = 0,
      .pixels = pixels,
  };
  for (int i = 0; i < w * h; i++)
  {
    pixels[i] = color;
  }
  ioe_write(AM_GPU_FBDRAW, &event);
}

void splash()
{
  init();
  for (int x = 0; x * SIDE <= w; x++)
  {
    for (int y = 0; y * SIDE <= h; y++)
    {
      if ((x & 1) ^ (y & 1))
      {
        draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, 0xffffff); // white
      }
    }
  }
  io_write(AM_GPU_FBDRAW, 0, 0, NULL, 0, 0, true);
}
void redraw()
{
  init();
  for (int x = 0; x * SIDE <= w; x++)
  {
    for (int y = 0; y * SIDE <= h; y++)
    {
      float x1 = x * SIDE * 1.0 / w;
      float y1 = y * SIDE * 1.0 / h;
      if (x1 > g_game.pos_x && x1 < g_game.pos_x + 0.1 && y1 > g_game.pos_y && y1 < g_game.pos_y + 0.1)
      {
        draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, 0xffffff); // white
      }
      else
      {
        draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, 0x0ff000); // white
      }
    }
    io_write(AM_GPU_FBDRAW, 0, 0, NULL, 0, 0, true);
  }
}