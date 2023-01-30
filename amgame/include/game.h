#ifndef __GAME_H__
#define __GAME_H__
#include <am.h>
#include <amdev.h>
#include <klib.h>
#include <klib-macros.h>

typedef struct _GamePlay
{
  float pos_x, pos_y;
} GamePlay;

extern GamePlay g_game;

void splash();
int handle_input();
void redraw();
static inline void puts(const char *s) {
  for (; *s; s++) putch(*s);
}
#endif