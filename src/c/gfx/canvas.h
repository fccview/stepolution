#pragma once

#include <pebble.h>
#include "layout.h"

typedef struct {
  GBitmap *frame_buffer;
  GBitmapDataRowInfo rows[SCREEN_H];
  const uint8_t *tint;
  const uint8_t *grade;
} Canvas;

void canvas_begin(Canvas *canvas, GContext *ctx);
void canvas_end(Canvas *canvas, GContext *ctx);
void canvas_set_tint(Canvas *canvas, const uint8_t *tint);
void canvas_set_grade(Canvas *canvas, const uint8_t *grade);
void canvas_fill(Canvas *canvas, GRect rect, GColor color);
int16_t canvas_opaque_top(const GBitmap *source, GRect source_rect);
void canvas_blit(Canvas *canvas, const GBitmap *source, GRect source_rect, GPoint origin, uint8_t scale, bool flip);
void canvas_blit_except(Canvas *canvas, const GBitmap *source, GRect source_rect, GPoint origin, uint8_t scale, GColor skip);
void canvas_tile(Canvas *canvas, const GBitmap *source, GRect tile, GRect area, uint8_t scale);
