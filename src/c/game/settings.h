#pragma once

#include <pebble.h>

typedef struct {
  uint8_t trainer;
  uint8_t starter;
  uint8_t scene;
  uint8_t frame;
  uint8_t zoom;
  uint16_t wander_seconds;
  int32_t step_goal;
  bool demo;
  uint8_t daylight;
  bool idle_animation;
  bool active_only;
} Settings;

typedef void (*SettingsChangedHandler)(const Settings *previous);

const Settings *settings_get(void);
void settings_init(SettingsChangedHandler on_change);
void settings_deinit(void);
