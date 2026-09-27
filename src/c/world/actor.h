#pragma once

#include "world/scene.h"

#define STEP_FRAMES 8
#define STEP_MS 45

typedef void (*RedrawHandler)(void);

typedef enum {
  DirectionDown,
  DirectionUp,
  DirectionLeft,
  DirectionRight,
} Direction;

typedef enum {
  ActorKindTrainer,
  ActorKindPokemon,
} ActorKind;

typedef struct {
  ActorKind kind;
  GBitmap *sheet;
  GPoint cell;
  GPoint from;
  Direction facing;
  int16_t top;
  uint8_t progress;
  bool stride;
  bool bob;
} Actor;

void actor_load(Actor *actor, ActorKind kind, uint32_t resource);
void actor_unload(Actor *actor);
void actor_place(Actor *actor, GPoint cell, Direction facing);
void actor_step_to(Actor *actor, GPoint cell);
void actor_face(Actor *actor, GPoint cell);
bool actor_is_moving(const Actor *actor);
void actor_advance(Actor *actor);
GPoint actor_world_position(const Actor *actor);
void actor_draw(const Actor *actor, const Scene *scene, Canvas *canvas);
