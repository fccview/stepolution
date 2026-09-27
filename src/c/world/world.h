#pragma once

#include "world/actor.h"

typedef struct {
  uint8_t scene;
  uint8_t zoom;
  uint32_t trainer_sprite;
  uint32_t pokemon_sprite;
  uint16_t wander_seconds;
  bool idle_animation;
} WorldConfig;

void world_load(const WorldConfig *config, RedrawHandler redraw);
void world_unload(void);
void world_set_pokemon(uint32_t sprite);
void world_set_awake(bool awake);
void world_draw(Canvas *canvas, const uint8_t *tint);
bool world_is_glitching(void);
