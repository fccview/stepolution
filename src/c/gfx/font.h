#pragma once

#include "gfx/canvas.h"

typedef enum {
  FontStyleShadow,
  FontStylePlain,
} FontStyle;

void font_load(void);
void font_unload(void);
int16_t font_width(const char *text, uint8_t scale);
void font_draw(Canvas *canvas, const char *text, GPoint origin, uint8_t scale, FontStyle style);
void font_draw_right(Canvas *canvas, const char *text, GPoint right_top, uint8_t scale, FontStyle style);
