#include <pebble.h>
#include "gfx/canvas.h"
#include "gfx/grade.h"
#include "hud/hud.h"
#include "game/daily.h"
#include "game/daylight.h"
#include "game/pokemon.h"
#include "game/settings.h"
#include "world/world.h"

#define DEMO_SECONDS_TO_GOAL 60
#define DEMO_CYCLE_PERCENT 125

static const uint32_t s_trainers[] = { RESOURCE_ID_TRAINER_RED, RESOURCE_ID_TRAINER_LEAF };

static Window *s_window;
static Layer *s_layer;
static Canvas s_canvas;
static HudState s_hud;
static int32_t s_demo_steps;

static DailyPick prv_pick(void) {
  const Settings *settings = settings_get();
  return daily_resolve(settings->starter, settings->scene);
}

static const Starter *prv_starter(void) {
  return pokemon_starter(prv_pick().starter);
}

static bool prv_uses_random(void) {
  const Settings *settings = settings_get();
  return settings->starter == DAILY_RANDOM || settings->scene == DAILY_RANDOM;
}

static int32_t prv_steps(void) {
  if (settings_get()->demo) {
    return s_demo_steps;
  }
  HealthServiceAccessibilityMask access = health_service_metric_accessible(
      HealthMetricStepCount, time_start_of_today(), time(NULL));
  return access & HealthServiceAccessibilityMaskAvailable ? health_service_sum_today(HealthMetricStepCount) : 0;
}

static void prv_refresh_growth(void) {
  const Settings *settings = settings_get();
  uint8_t previous_stage = s_hud.growth.stage;
  s_hud.steps = prv_steps();
  s_hud.growth = pokemon_growth(prv_starter(), s_hud.steps, settings->step_goal);
  if (s_hud.growth.stage != previous_stage) {
    world_set_pokemon(prv_starter()->sprites[s_hud.growth.stage]);
  }
  layer_mark_dirty(s_layer);
}

static void prv_update_demo(void) {
  int32_t goal = settings_get()->step_goal;
  s_demo_steps = (s_demo_steps + goal / DEMO_SECONDS_TO_GOAL) % (goal * DEMO_CYCLE_PERCENT / 100);
}

static void prv_reload_face(void);

static void prv_tick(struct tm *tick_time, TimeUnits units_changed) {
  s_hud.time = *tick_time;
  if (daily_roll_over(tick_time) && prv_uses_random()) {
    prv_reload_face();
    return;
  }
  if (settings_get()->demo) {
    prv_update_demo();
  }
  prv_refresh_growth();
}

static void prv_battery(BatteryChargeState state) {
  s_hud.battery = state;
  layer_mark_dirty(s_layer);
}

static void prv_health(HealthEventType event, void *context) {
  if (event == HealthEventMovementUpdate || event == HealthEventSignificantUpdate) {
    prv_refresh_growth();
  }
}

static void prv_backlight(bool on) {
  world_set_awake(on);
}

static void prv_redraw(void) {
  layer_mark_dirty(s_layer);
}

static void prv_draw(Layer *layer, GContext *ctx) {
  const uint8_t *tint = daylight_tint(settings_get()->daylight, &s_hud.time);
  s_hud.glitched = world_is_glitching();
  canvas_begin(&s_canvas, ctx);
  canvas_set_grade(&s_canvas, grade_table(settings_get()->grade));
  world_draw(&s_canvas, tint);
  hud_draw(&s_canvas, &s_hud);
  canvas_end(&s_canvas, ctx);
}

static void prv_load_face(void) {
  const Settings *settings = settings_get();
  s_hud.steps = prv_steps();
  s_hud.growth = pokemon_growth(prv_starter(), s_hud.steps, settings->step_goal);
  WorldConfig config = {
    .scene = prv_pick().scene,
    .zoom = settings->zoom,
    .trainer_sprite = s_trainers[settings->trainer % ARRAY_LENGTH(s_trainers)],
    .pokemon_sprite = prv_starter()->sprites[s_hud.growth.stage],
    .wander_seconds = settings->wander_seconds,
    .idle_animation = settings->idle_animation,
  };
  world_load(&config, prv_redraw);
  if (settings->active_only) {
    backlight_service_subscribe(prv_backlight);
    world_set_awake(light_is_on());
  }
  hud_load(settings->frame);
  tick_timer_service_subscribe(settings->demo ? SECOND_UNIT : MINUTE_UNIT, prv_tick);
}

static void prv_unload_face(void) {
  tick_timer_service_unsubscribe();
  backlight_service_unsubscribe();
  hud_unload();
  world_unload();
}

static void prv_reload_face(void) {
  prv_unload_face();
  prv_load_face();
  layer_mark_dirty(s_layer);
}

static void prv_settings_changed(const Settings *previous) {
  const Settings *settings = settings_get();
  if (settings->starter == DAILY_RANDOM && previous->starter != DAILY_RANDOM) {
    daily_reroll_starter();
  }
  if (settings->scene == DAILY_RANDOM && previous->scene != DAILY_RANDOM) {
    daily_reroll_scene();
  }
  s_demo_steps = 0;
  prv_reload_face();
}

static void prv_window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  s_layer = layer_create(layer_get_bounds(root));
  layer_set_update_proc(s_layer, prv_draw);
  layer_add_child(root, s_layer);

  time_t now = time(NULL);
  s_hud.time = *localtime(&now);
  s_hud.battery = battery_state_service_peek();
  daily_init(&s_hud.time);
  prv_load_face();
}

static void prv_window_unload(Window *window) {
  prv_unload_face();
  layer_destroy(s_layer);
}

static void prv_init(void) {
  srand(time(NULL));
  settings_init(prv_settings_changed);
  daylight_load();
  grade_load();
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  window_stack_push(s_window, false);
  battery_state_service_subscribe(prv_battery);
  health_service_events_subscribe(prv_health, NULL);
}

static void prv_deinit(void) {
  health_service_events_unsubscribe();
  battery_state_service_unsubscribe();
  settings_deinit();
  window_destroy(s_window);
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
