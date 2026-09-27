#pragma once

#include <pebble.h>

#define DAILY_RANDOM UINT8_MAX

typedef struct {
  uint8_t starter;
  uint8_t scene;
} DailyPick;

void daily_init(const struct tm *now);
bool daily_roll_over(const struct tm *now);
void daily_reroll_starter(void);
void daily_reroll_scene(void);
DailyPick daily_resolve(uint8_t starter, uint8_t scene);
