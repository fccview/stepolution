#pragma once

#include <pebble.h>

#define STAGE_COUNT 3

typedef struct {
  const char *names[STAGE_COUNT];
  uint32_t sprites[STAGE_COUNT];
  uint8_t evolve_levels[STAGE_COUNT - 1];
} Starter;

typedef struct {
  uint8_t stage;
  uint8_t level;
  uint8_t exp_percent;
} Growth;

uint8_t pokemon_count(void);
const Starter *pokemon_starter(uint8_t index);
Growth pokemon_growth(const Starter *starter, int32_t steps, int32_t goal);
