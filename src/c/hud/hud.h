#pragma once

#include "gfx/canvas.h"
#include "game/pokemon.h"

typedef struct {
  struct tm time;
  BatteryChargeState battery;
  int32_t steps;
  Growth growth;
  bool glitched;
} HudState;

void hud_load(uint8_t frame_style);
void hud_unload(void);
void hud_draw(Canvas *canvas, const HudState *state);
