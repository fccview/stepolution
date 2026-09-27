#include "world/emote.h"

#define EMOTE_SIZE 16
#define EMOTE_KINDS 4
#define EMOTE_FRAMES 2
#define FRAME_MS 250
#define TICKS (EMOTE_MS / FRAME_MS)
#define LIFT 1

static GBitmap *s_sheet;
static RedrawHandler s_redraw;
static AppTimer *s_timer;
static uint8_t s_kind;
static uint8_t s_tick;
static bool s_showing;

static void prv_tick(void *context) {
  s_timer = NULL;
  if (++s_tick >= TICKS) {
    s_showing = false;
  } else {
    s_timer = app_timer_register(FRAME_MS, prv_tick, NULL);
  }
  s_redraw();
}

void emote_load(RedrawHandler redraw) {
  s_redraw = redraw;
  s_sheet = gbitmap_create_with_resource(RESOURCE_ID_EMOTES);
  s_kind = rand() % EMOTE_KINDS;
  s_showing = false;
}

void emote_unload(void) {
  if (s_timer) {
    app_timer_cancel(s_timer);
    s_timer = NULL;
  }
  gbitmap_destroy(s_sheet);
  s_sheet = NULL;
  s_showing = false;
}

void emote_show(void) {
  if (s_timer) {
    app_timer_cancel(s_timer);
  }
  s_kind = (s_kind + 1 + rand() % (EMOTE_KINDS - 1)) % EMOTE_KINDS;
  s_tick = 0;
  s_showing = true;
  s_timer = app_timer_register(FRAME_MS, prv_tick, NULL);
  s_redraw();
}

void emote_hide(void) {
  if (s_timer) {
    app_timer_cancel(s_timer);
    s_timer = NULL;
  }
  s_showing = false;
}

void emote_draw(Canvas *canvas, const Scene *scene, const Actor *anchor) {
  if (!s_showing) {
    return;
  }
  GPoint world = actor_world_position(anchor);
  GPoint origin = GPoint(world.x + (TILE - EMOTE_SIZE) / 2, world.y + TILE + anchor->top - EMOTE_SIZE + LIFT);
  uint8_t frame = s_kind * EMOTE_FRAMES + s_tick % EMOTE_FRAMES;
  GRect source = GRect(frame * EMOTE_SIZE, 0, EMOTE_SIZE, EMOTE_SIZE);
  canvas_blit(canvas, s_sheet, source, scene_to_screen(scene, origin), scene->zoom, false);
}
