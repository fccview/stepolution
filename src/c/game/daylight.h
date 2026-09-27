#pragma once

#include <pebble.h>

typedef enum {
  DaylightCycle,
  DaylightDay,
  DaylightDusk,
  DaylightNight,
} DaylightMode;

void daylight_load(void);
const uint8_t *daylight_tint(DaylightMode mode, const struct tm *time);
