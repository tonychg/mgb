#ifndef _RENDER_H
#define _RENDER_H

#include "platform/types.h"
#include <stdbool.h>

void render_pixel(int x, int y, int color, int scale);
void render_init(int width, int height, int scale);
bool render_is_running(void);
void render_handle_inputs(u8 *joypad);
void render_debug(int dots, int frames);
void render_begin(void);
void render_end(void);
void render_release(void);

#endif
