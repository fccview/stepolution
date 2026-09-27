#include "hud/hud.h"
#include "gfx/font.h"
#include <ctype.h>

#define FRAME_TILE 8
#define FRAME_SCALE 1
#define EDGE (FRAME_TILE * FRAME_SCALE)
#define TIME_SCALE (UI_SCALE * 2)
#define RIGHT_EDGE 188

static const uint32_t s_frames[] = {
  RESOURCE_ID_FRAME_0, RESOURCE_ID_FRAME_1, RESOURCE_ID_FRAME_2, RESOURCE_ID_FRAME_3,
  RESOURCE_ID_FRAME_4, RESOURCE_ID_FRAME_5, RESOURCE_ID_FRAME_6, RESOURCE_ID_FRAME_7,
  RESOURCE_ID_FRAME_8, RESOURCE_ID_FRAME_9, RESOURCE_ID_FRAME_10, RESOURCE_ID_FRAME_11,
  RESOURCE_ID_FRAME_12, RESOURCE_ID_FRAME_13, RESOURCE_ID_FRAME_14, RESOURCE_ID_FRAME_15,
  RESOURCE_ID_FRAME_16, RESOURCE_ID_FRAME_17, RESOURCE_ID_FRAME_18, RESOURCE_ID_FRAME_19,
  RESOURCE_ID_FRAME_20, RESOURCE_ID_FRAME_21, RESOURCE_ID_FRAME_22, RESOURCE_ID_FRAME_23,
  RESOURCE_ID_FRAME_24, RESOURCE_ID_FRAME_25, RESOURCE_ID_FRAME_26, RESOURCE_ID_FRAME_27,
};

static GBitmap *s_frame;

void hud_load(uint8_t frame_style) {
  s_frame = gbitmap_create_with_resource(s_frames[frame_style % ARRAY_LENGTH(s_frames)]);
  font_load();
}

void hud_unload(void) {
  font_unload();
  gbitmap_destroy(s_frame);
}

static GRect prv_tile(int16_t col, int16_t row) {
  return GRect(col * FRAME_TILE, row * FRAME_TILE, FRAME_TILE, FRAME_TILE);
}

static void prv_box(Canvas *canvas, GRect box) {
  int16_t left = box.origin.x, top = box.origin.y;
  int16_t right = left + box.size.w - EDGE, bottom = top + box.size.h - EDGE;
  int16_t inner_w = box.size.w - 2 * EDGE, inner_h = box.size.h - 2 * EDGE;

  canvas_tile(canvas, s_frame, prv_tile(1, 1), GRect(left + EDGE, top + EDGE, inner_w, inner_h), FRAME_SCALE);
  canvas_tile(canvas, s_frame, prv_tile(1, 0), GRect(left + EDGE, top, inner_w, EDGE), FRAME_SCALE);
  canvas_tile(canvas, s_frame, prv_tile(1, 2), GRect(left + EDGE, bottom, inner_w, EDGE), FRAME_SCALE);
  canvas_tile(canvas, s_frame, prv_tile(0, 1), GRect(left, top + EDGE, EDGE, inner_h), FRAME_SCALE);
  canvas_tile(canvas, s_frame, prv_tile(2, 1), GRect(right, top + EDGE, EDGE, inner_h), FRAME_SCALE);
  canvas_blit(canvas, s_frame, prv_tile(0, 0), GPoint(left, top), FRAME_SCALE, false);
  canvas_blit(canvas, s_frame, prv_tile(2, 0), GPoint(right, top), FRAME_SCALE, false);
  canvas_blit(canvas, s_frame, prv_tile(0, 2), GPoint(left, bottom), FRAME_SCALE, false);
  canvas_blit(canvas, s_frame, prv_tile(2, 2), GPoint(right, bottom), FRAME_SCALE, false);
}

static void prv_meter(Canvas *canvas, GRect rect, uint8_t percent, GColor fill) {
  GRect inner = grect_inset(rect, GEdgeInsets(UI_SCALE));
  canvas_fill(canvas, rect, GColorDarkGray);
  canvas_fill(canvas, inner, GColorLightGray);
  int16_t filled = inner.size.w * percent / 100 / UI_SCALE * UI_SCALE;
  canvas_fill(canvas, GRect(inner.origin.x, inner.origin.y, filled, inner.size.h), fill);
}

static GColor prv_battery_color(const BatteryChargeState *battery) {
  if (battery->is_charging) return GColorVividCerulean;
  if (battery->charge_percent > 50) return GColorMalachite;
  if (battery->charge_percent > 20) return GColorChromeYellow;
  return GColorRed;
}

static void prv_battery(Canvas *canvas, GPoint origin, const BatteryChargeState *battery) {
  GRect body = GRect(origin.x, origin.y, 24, 14);
  prv_meter(canvas, body, battery->charge_percent, prv_battery_color(battery));
  canvas_fill(canvas, GRect(body.origin.x + body.size.w, body.origin.y + 4, UI_SCALE, 6), GColorDarkGray);
}

static void prv_uppercase(char *text) {
  for (; *text; text++) {
    *text = toupper((unsigned char)*text);
  }
}

static void prv_draw_time(Canvas *canvas, const struct tm *time) {
  static char clock[8];
  strftime(clock, sizeof(clock), clock_is_24h_style() ? "%H:%M" : "%I:%M", time);
  const char *text = !clock_is_24h_style() && clock[0] == '0' ? clock + 1 : clock;
  font_draw(canvas, text, GPoint(12, 6), TIME_SCALE, FontStylePlain);
}

static void prv_draw_date(Canvas *canvas, const struct tm *time) {
  static char weekday[8];
  static char day[4];
  strftime(weekday, sizeof(weekday), "%a", time);
  prv_uppercase(weekday);
  snprintf(day, sizeof(day), "%d", time->tm_mday);
  font_draw_right(canvas, weekday, GPoint(RIGHT_EDGE, 6), UI_SCALE, FontStyleShadow);
  font_draw_right(canvas, day, GPoint(RIGHT_EDGE, 28), UI_SCALE, FontStyleShadow);
}

static void prv_glitch_text(char *text, size_t size) {
  static const char glyphs[] = "?!%&/#-.,:;";
  for (size_t i = 0; i + 1 < size; i++) {
    text[i] = glyphs[rand() % (sizeof(glyphs) - 1)];
  }
  text[size - 1] = '\0';
}

static void prv_draw_status(Canvas *canvas, const HudState *state) {
  static char level[8];
  static char steps[8];
  snprintf(level, sizeof(level), "Lv%d", state->growth.level);
  snprintf(steps, sizeof(steps), "%ld", (long)state->steps);
  if (state->glitched) {
    prv_glitch_text(steps, 6);
  }
  int16_t y = BOTTOM_BOX_Y + 8;
  font_draw(canvas, level, GPoint(12, y), UI_SCALE, FontStyleShadow);
  prv_meter(canvas, GRect(78, BOTTOM_BOX_Y + 18, 44, 8), state->growth.exp_percent, GColorPictonBlue);
  font_draw_right(canvas, steps, GPoint(RIGHT_EDGE, y), UI_SCALE, FontStyleShadow);
}

void hud_draw(Canvas *canvas, const HudState *state) {
  prv_box(canvas, GRect(0, 0, SCREEN_W, TOP_BOX_H));
  prv_box(canvas, GRect(0, BOTTOM_BOX_Y, SCREEN_W, BOTTOM_BOX_H));
  prv_draw_time(canvas, &state->time);
  prv_draw_date(canvas, &state->time);
  prv_battery(canvas, GPoint(130, 36), &state->battery);
  prv_draw_status(canvas, state);
}
