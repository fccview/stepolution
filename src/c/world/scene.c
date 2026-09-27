#include "world/scene.h"

#define GRID_WALKABLE 1
#define GRID_PROP 2
#define GRID_CAMEO 3

#define SCENE_OUTDOOR 1

typedef struct {
  uint32_t image;
  uint32_t grid;
  uint32_t prop;
  uint32_t cameo;
  CameoStyle cameo_style;
} SceneResources;

static const SceneResources s_scenes[] = {
  { .image = RESOURCE_ID_SCENE_PALLET, .grid = RESOURCE_ID_SCENE_PALLET_GRID },
  { .image = RESOURCE_ID_SCENE_BEDROOM, .grid = RESOURCE_ID_SCENE_BEDROOM_GRID },
  { .image = RESOURCE_ID_SCENE_LAKE, .grid = RESOURCE_ID_SCENE_LAKE_GRID },
  { .image = RESOURCE_ID_SCENE_FOREST, .grid = RESOURCE_ID_SCENE_FOREST_GRID },
  { .image = RESOURCE_ID_SCENE_LAVENDER, .grid = RESOURCE_ID_SCENE_LAVENDER_GRID },
  { .image = RESOURCE_ID_SCENE_TOWER, .grid = RESOURCE_ID_SCENE_TOWER_GRID },
  { .image = RESOURCE_ID_SCENE_CAVE, .grid = RESOURCE_ID_SCENE_CAVE_GRID, .prop = RESOURCE_ID_MEWTWO },
  { .image = RESOURCE_ID_SCENE_DOCK, .grid = RESOURCE_ID_SCENE_DOCK_GRID,
    .cameo = RESOURCE_ID_MEW, .cameo_style = CameoStyleEmerge },
  { .image = RESOURCE_ID_SCENE_SAFARI, .grid = RESOURCE_ID_SCENE_SAFARI_GRID },
  { .image = RESOURCE_ID_SCENE_GLITCH, .grid = RESOURCE_ID_SCENE_GLITCH_GRID,
    .cameo = RESOURCE_ID_MISSINGNO, .cameo_style = CameoStyleApparition },
};

static int16_t prv_mod(int16_t value, int16_t divisor) {
  int16_t result = value % divisor;
  return result < 0 ? result + divisor : result;
}

static GPoint prv_view_origin(uint8_t zoom) {
  int16_t view_w = SCREEN_W / zoom;
  int16_t view_h = SCREEN_H / zoom;
  int16_t crop_w = CROP_COLS * TILE;
  int16_t crop_h = CROP_ROWS * TILE;
  int16_t phase = (TOP_BOX_H + SPRITE_H * zoom) % (TILE * zoom);
  int16_t target_y = (crop_h - view_h) / 2;
  int16_t y = target_y - prv_mod(target_y + phase / zoom, TILE);
  return GPoint((crop_w - view_w) / 2, y);
}

uint8_t scene_count(void) {
  return ARRAY_LENGTH(s_scenes);
}

void scene_load(Scene *scene, uint8_t index, uint8_t zoom) {
  const SceneResources *resources = &s_scenes[index % ARRAY_LENGTH(s_scenes)];
  scene->image = gbitmap_create_with_resource(resources->image);
  ResHandle grid = resource_get_handle(resources->grid);
  uint8_t flags = 0;
  resource_load(grid, &scene->walkable[0][0], sizeof(scene->walkable));
  resource_load_byte_range(grid, sizeof(scene->walkable), &flags, 1);
  scene->outdoor = flags & SCENE_OUTDOOR;
  scene->zoom = zoom;
  scene->view = prv_view_origin(zoom);
  scene->prop_sprite = resources->prop;
  scene->cameo_sprite = resources->cameo;
  scene->cameo_style = resources->cameo_style;
  for (int16_t y = 0; y < CROP_ROWS; y++) {
    for (int16_t x = 0; x < CROP_COLS; x++) {
      if (scene->walkable[y][x] == GRID_PROP) {
        scene->prop_cell = GPoint(x, y);
      } else if (scene->walkable[y][x] == GRID_CAMEO) {
        scene->cameo_cell = GPoint(x, y);
      }
    }
  }
}

void scene_unload(Scene *scene) {
  gbitmap_destroy(scene->image);
  scene->image = NULL;
}

void scene_draw(const Scene *scene, Canvas *canvas) {
  GRect source = GRect(scene->view.x, scene->view.y, SCREEN_W / scene->zoom, SCREEN_H / scene->zoom);
  canvas_blit(canvas, scene->image, source, GPoint(0, 0), scene->zoom, false);
}

GPoint scene_to_screen(const Scene *scene, GPoint world) {
  return GPoint((world.x - scene->view.x) * scene->zoom, (world.y - scene->view.y) * scene->zoom);
}

bool scene_can_stand(const Scene *scene, GPoint cell) {
  if (cell.x < 0 || cell.y < 0 || cell.x >= CROP_COLS || cell.y >= CROP_ROWS || scene->walkable[cell.y][cell.x] != GRID_WALKABLE) {
    return false;
  }
  int16_t size = TILE * scene->zoom;
  GPoint top_left = scene_to_screen(scene, GPoint(cell.x * TILE, cell.y * TILE));
  return top_left.x >= 0 && top_left.x + size <= SCREEN_W
      && top_left.y + size - SPRITE_H * scene->zoom >= TOP_BOX_H
      && top_left.y + size <= BOTTOM_BOX_Y;
}
