#include "gfx/font.h"

#define FIRST_CHAR 32
#define CHAR_COUNT 96
#define CELL 16
#define CELLS_PER_ROW 16
#define SHADOW_COLOR GColorLightGray

static GBitmap *s_glyphs;
static uint8_t s_widths[CHAR_COUNT];

static int16_t prv_index(char c) {
  int16_t index = (uint8_t)c - FIRST_CHAR;
  return index >= 0 && index < CHAR_COUNT ? index : -1;
}

void font_load(void) {
  s_glyphs = gbitmap_create_with_resource(RESOURCE_ID_FONT);
  resource_load(resource_get_handle(RESOURCE_ID_FONT_WIDTHS), s_widths, CHAR_COUNT);
}

void font_unload(void) {
  gbitmap_destroy(s_glyphs);
}

int16_t font_width(const char *text, uint8_t scale) {
  int16_t width = 0;
  for (const char *c = text; *c; c++) {
    int16_t index = prv_index(*c);
    width += index < 0 ? 0 : s_widths[index];
  }
  return width * scale;
}

void font_draw(Canvas *canvas, const char *text, GPoint origin, uint8_t scale, FontStyle style) {
  int16_t x = origin.x;
  for (const char *c = text; *c; c++) {
    int16_t index = prv_index(*c);
    if (index < 0 || !s_widths[index]) {
      continue;
    }
    GRect cell = GRect((index % CELLS_PER_ROW) * CELL, (index / CELLS_PER_ROW) * CELL, s_widths[index], CELL);
    GPoint at = GPoint(x, origin.y);
    if (style == FontStylePlain) {
      canvas_blit_except(canvas, s_glyphs, cell, at, scale, SHADOW_COLOR);
    } else {
      canvas_blit(canvas, s_glyphs, cell, at, scale, false);
    }
    x += s_widths[index] * scale;
  }
}

void font_draw_right(Canvas *canvas, const char *text, GPoint right_top, uint8_t scale, FontStyle style) {
  font_draw(canvas, text, GPoint(right_top.x - font_width(text, scale), right_top.y), scale, style);
}
