#pragma once

#include "gfx/canvas.h"

typedef enum {
  CameoStyleEmerge,
  CameoStyleApparition,
} CameoStyle;

typedef struct {
  GBitmap *image;
  uint8_t walkable[CROP_ROWS][CROP_COLS];
  GPoint view;
  uint8_t zoom;
  uint32_t prop_sprite;
  GPoint prop_cell;
  uint32_t cameo_sprite;
  CameoStyle cameo_style;
  GPoint cameo_cell;
  bool outdoor;
} Scene;

uint8_t scene_count(void);
void scene_load(Scene *scene, uint8_t index, uint8_t zoom);
void scene_unload(Scene *scene);
void scene_draw(const Scene *scene, Canvas *canvas);
bool scene_can_stand(const Scene *scene, GPoint cell);
GPoint scene_to_screen(const Scene *scene, GPoint world);
