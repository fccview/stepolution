#include "game/settings.h"
#include "game/daily.h"
#include <stdlib.h>

#define STORAGE_KEY 1

static Settings s_settings;
static SettingsChangedHandler s_on_change;

static void prv_defaults(void) {
  s_settings = (Settings) {
    .trainer = 0,
    .starter = DAILY_RANDOM,
    .scene = DAILY_RANDOM,
    .frame = 0,
    .zoom = 1,
    .wander_seconds = 60,
    .step_goal = 8000,
    .demo = false,
    .daylight = 0,
    .idle_animation = false,
    .active_only = true,
  };
}

static int32_t prv_tuple_int(const Tuple *tuple) {
  return tuple->type == TUPLE_CSTRING ? atoi(tuple->value->cstring) : tuple->value->int32;
}

static int32_t prv_read(DictionaryIterator *iter, uint32_t key, int32_t min, int32_t max, int32_t fallback) {
  Tuple *tuple = dict_find(iter, key);
  if (!tuple) {
    return fallback;
  }
  int32_t value = prv_tuple_int(tuple);
  return value < min ? min : value > max ? max : value;
}

static void prv_inbox_received(DictionaryIterator *iter, void *context) {
  Settings previous = s_settings;
  Settings *s = &s_settings;
  s->trainer = prv_read(iter, MESSAGE_KEY_Trainer, 0, 1, s->trainer);
  s->starter = prv_read(iter, MESSAGE_KEY_Starter, 0, UINT8_MAX, s->starter);
  s->scene = prv_read(iter, MESSAGE_KEY_Scene, 0, UINT8_MAX, s->scene);
  s->frame = prv_read(iter, MESSAGE_KEY_Frame, 0, UINT8_MAX, s->frame);
  s->zoom = prv_read(iter, MESSAGE_KEY_Zoom, 1, 2, s->zoom);
  s->wander_seconds = prv_read(iter, MESSAGE_KEY_WanderSeconds, 0, 600, s->wander_seconds);
  s->step_goal = prv_read(iter, MESSAGE_KEY_StepGoal, 1000, 100000, s->step_goal);
  s->demo = prv_read(iter, MESSAGE_KEY_Demo, 0, 1, s->demo);
  s->daylight = prv_read(iter, MESSAGE_KEY_Daylight, 0, 3, s->daylight);
  s->idle_animation = prv_read(iter, MESSAGE_KEY_IdleAnimation, 0, 1, s->idle_animation);
  s->active_only = prv_read(iter, MESSAGE_KEY_ActiveOnly, 0, 1, s->active_only);
  persist_write_data(STORAGE_KEY, &s_settings, sizeof(s_settings));
  if (s_on_change) {
    s_on_change(&previous);
  }
}

const Settings *settings_get(void) {
  return &s_settings;
}

void settings_init(SettingsChangedHandler on_change) {
  s_on_change = on_change;
  prv_defaults();
  if (persist_get_size(STORAGE_KEY) == sizeof(s_settings)) {
    persist_read_data(STORAGE_KEY, &s_settings, sizeof(s_settings));
  }
  app_message_register_inbox_received(prv_inbox_received);
  app_message_open(256, 64);
}

void settings_deinit(void) {
  app_message_deregister_callbacks();
}
