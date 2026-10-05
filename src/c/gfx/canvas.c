#include "gfx/canvas.h"

static uint8_t prv_bits_per_pixel(GBitmapFormat format) {
  switch (format) {
    case GBitmapFormat1BitPalette: return 1;
    case GBitmapFormat2BitPalette: return 2;
    case GBitmapFormat4BitPalette: return 4;
    default: return 8;
  }
}

static uint8_t prv_read(const uint8_t *row, int16_t x, uint8_t bpp, const GColor *palette) {
  if (!palette) {
    return row[x];
  }
  uint8_t per_byte = 8 / bpp;
  uint8_t shift = 8 - bpp * (x % per_byte + 1);
  uint8_t index = (row[x / per_byte] >> shift) & ((1 << bpp) - 1);
  return palette[index].argb;
}

static void prv_put(Canvas *canvas, int16_t x, int16_t y, uint8_t scale, uint8_t argb) {
  for (int16_t py = y; py < y + scale; py++) {
    if (py < 0 || py >= SCREEN_H) {
      continue;
    }
    GBitmapDataRowInfo *row = &canvas->rows[py];
    int16_t from = x < row->min_x ? row->min_x : x;
    int16_t to = x + scale - 1 > row->max_x ? row->max_x : x + scale - 1;
    for (int16_t px = from; px <= to; px++) {
      row->data[px] = argb;
    }
  }
}

static uint8_t prv_color(const Canvas *canvas, uint8_t argb) {
  uint8_t color = canvas->tint ? canvas->tint[argb & 0x3F] : argb;
  return canvas->grade ? canvas->grade[color & 0x3F] : color;
}

void canvas_begin(Canvas *canvas, GContext *ctx) {
  canvas->frame_buffer = graphics_capture_frame_buffer(ctx);
  canvas->tint = NULL;
  for (int16_t y = 0; y < SCREEN_H; y++) {
    canvas->rows[y] = gbitmap_get_data_row_info(canvas->frame_buffer, y);
  }
}

void canvas_end(Canvas *canvas, GContext *ctx) {
  graphics_release_frame_buffer(ctx, canvas->frame_buffer);
  canvas->frame_buffer = NULL;
}

void canvas_set_tint(Canvas *canvas, const uint8_t *tint) {
  canvas->tint = tint;
}

void canvas_set_grade(Canvas *canvas, const uint8_t *grade) {
  canvas->grade = grade;
}

void canvas_fill(Canvas *canvas, GRect rect, GColor color) {
  for (int16_t y = rect.origin.y; y < rect.origin.y + rect.size.h; y++) {
    for (int16_t x = rect.origin.x; x < rect.origin.x + rect.size.w; x++) {
      prv_put(canvas, x, y, 1, prv_color(canvas, color.argb));
    }
  }
}

static void prv_blit(Canvas *canvas, const GBitmap *source, GRect source_rect, GPoint origin, uint8_t scale, bool flip, uint8_t skip) {
  GBitmapFormat format = gbitmap_get_format(source);
  uint8_t bpp = prv_bits_per_pixel(format);
  const GColor *palette = format == GBitmapFormat8Bit ? NULL : gbitmap_get_palette(source);
  const uint8_t *data = gbitmap_get_data(source);
  uint16_t stride = gbitmap_get_bytes_per_row(source);

  for (int16_t sy = 0; sy < source_rect.size.h; sy++) {
    int16_t y = origin.y + sy * scale;
    if (y + scale <= 0 || y >= SCREEN_H) {
      continue;
    }
    const uint8_t *row = data + (source_rect.origin.y + sy) * stride;
    for (int16_t sx = 0; sx < source_rect.size.w; sx++) {
      int16_t column = flip ? source_rect.size.w - 1 - sx : sx;
      uint8_t argb = prv_read(row, source_rect.origin.x + column, bpp, palette);
      if (argb >> 6 && argb != skip) {
        prv_put(canvas, origin.x + sx * scale, y, scale, prv_color(canvas, argb));
      }
    }
  }
}

int16_t canvas_opaque_top(const GBitmap *source, GRect source_rect) {
  GBitmapFormat format = gbitmap_get_format(source);
  uint8_t bpp = prv_bits_per_pixel(format);
  const GColor *palette = format == GBitmapFormat8Bit ? NULL : gbitmap_get_palette(source);
  const uint8_t *data = gbitmap_get_data(source);
  uint16_t stride = gbitmap_get_bytes_per_row(source);
  for (int16_t sy = 0; sy < source_rect.size.h; sy++) {
    const uint8_t *row = data + (source_rect.origin.y + sy) * stride;
    for (int16_t sx = 0; sx < source_rect.size.w; sx++) {
      if (prv_read(row, source_rect.origin.x + sx, bpp, palette) >> 6) {
        return sy;
      }
    }
  }
  return source_rect.size.h;
}

void canvas_blit(Canvas *canvas, const GBitmap *source, GRect source_rect, GPoint origin, uint8_t scale, bool flip) {
  prv_blit(canvas, source, source_rect, origin, scale, flip, GColorClear.argb);
}

void canvas_blit_except(Canvas *canvas, const GBitmap *source, GRect source_rect, GPoint origin, uint8_t scale, GColor skip) {
  prv_blit(canvas, source, source_rect, origin, scale, false, skip.argb);
}

void canvas_tile(Canvas *canvas, const GBitmap *source, GRect tile, GRect area, uint8_t scale) {
  int16_t step_w = tile.size.w * scale;
  int16_t step_h = tile.size.h * scale;
  for (int16_t y = 0; y < area.size.h; y += step_h) {
    for (int16_t x = 0; x < area.size.w; x += step_w) {
      int16_t w = (area.size.w - x) / scale;
      int16_t h = (area.size.h - y) / scale;
      GRect part = GRect(tile.origin.x, tile.origin.y, w < tile.size.w ? w : tile.size.w, h < tile.size.h ? h : tile.size.h);
      canvas_blit(canvas, source, part, GPoint(area.origin.x + x, area.origin.y + y), scale, false);
    }
  }
}
