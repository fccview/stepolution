#include "world/actor.h"

typedef struct {
  GSize frame;
  uint8_t idle[4];
  uint8_t walk[4];
  uint8_t walk_frames;
} SheetLayout;

static const SheetLayout s_layouts[] = {
  [ActorKindTrainer] = { { 16, 32 }, { 0, 1, 2, 2 }, { 3, 5, 7, 7 }, 2 },
  [ActorKindPokemon] = { { 32, 32 }, { 0, 2, 4, 4 }, { 1, 3, 5, 5 }, 1 },
};

void actor_load(Actor *actor, ActorKind kind, uint32_t resource) {
  actor->kind = kind;
  actor->sheet = gbitmap_create_with_resource(resource);
  GSize frame = s_layouts[kind].frame;
  actor->top = canvas_opaque_top(actor->sheet, GRect(0, 0, frame.w, frame.h)) - frame.h;
}

void actor_unload(Actor *actor) {
  gbitmap_destroy(actor->sheet);
  actor->sheet = NULL;
}

void actor_place(Actor *actor, GPoint cell, Direction facing) {
  actor->cell = cell;
  actor->from = cell;
  actor->facing = facing;
  actor->progress = STEP_FRAMES;
  actor->bob = false;
}

static Direction prv_direction(GPoint from, GPoint to, Direction fallback) {
  if (to.x > from.x) return DirectionRight;
  if (to.x < from.x) return DirectionLeft;
  if (to.y > from.y) return DirectionDown;
  if (to.y < from.y) return DirectionUp;
  return fallback;
}

void actor_step_to(Actor *actor, GPoint cell) {
  actor->from = actor->cell;
  actor->cell = cell;
  actor->facing = prv_direction(actor->from, cell, actor->facing);
  actor->progress = gpoint_equal(&actor->from, &cell) ? STEP_FRAMES : 0;
  actor->stride = !actor->stride;
}

void actor_face(Actor *actor, GPoint cell) {
  actor->facing = prv_direction(actor->cell, cell, actor->facing);
}

bool actor_is_moving(const Actor *actor) {
  return actor->progress < STEP_FRAMES;
}

void actor_advance(Actor *actor) {
  if (actor_is_moving(actor)) {
    actor->progress++;
  }
}

GPoint actor_world_position(const Actor *actor) {
  int16_t dx = (actor->cell.x - actor->from.x) * TILE * actor->progress / STEP_FRAMES;
  int16_t dy = (actor->cell.y - actor->from.y) * TILE * actor->progress / STEP_FRAMES;
  return GPoint(actor->from.x * TILE + dx, actor->from.y * TILE + dy);
}

static uint8_t prv_frame(const Actor *actor, const SheetLayout *layout) {
  bool mid_step = actor_is_moving(actor) && actor->progress < STEP_FRAMES / 2;
  bool bobbing = actor->bob && !actor_is_moving(actor);
  if (!mid_step && !bobbing) {
    return layout->idle[actor->facing];
  }
  uint8_t stride = layout->walk_frames > 1 && actor->stride ? 1 : 0;
  return layout->walk[actor->facing] + stride;
}

void actor_draw(const Actor *actor, const Scene *scene, Canvas *canvas) {
  const SheetLayout *layout = &s_layouts[actor->kind];
  GSize size = layout->frame;
  GPoint world = actor_world_position(actor);
  GPoint sprite = GPoint(world.x + (TILE - size.w) / 2, world.y + TILE - size.h);
  GRect source = GRect(prv_frame(actor, layout) * size.w, 0, size.w, size.h);
  canvas_blit(canvas, actor->sheet, source, scene_to_screen(scene, sprite), scene->zoom,
              actor->facing == DirectionRight);
}
