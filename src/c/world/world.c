#include "world/world.h"
#include "world/cameo.h"
#include "world/emote.h"
#include "world/path.h"
#include "world/scene.h"

#define MIN_WANDER_STEPS 3
#define MAX_WANDER_STEPS 16
#define SECONDS_PER_STEP 6
#define BOB_MS 500
#define GREET_MIN_MS (4 * 60 * 1000)
#define GREET_JITTER_MS (16 * 60 * 1000)

static Scene s_scene;
static Actor s_trainer;
static Actor s_follower;
static Actor s_prop;
static RedrawHandler s_redraw;
static uint16_t s_wander_seconds;
static AppTimer *s_wander_timer;
static AppTimer *s_step_timer;
static AppTimer *s_greet_timer;
static AppTimer *s_bob_timer;
static GPoint s_path[MAX_WANDER_STEPS];
static uint8_t s_path_length;
static uint8_t s_step;
static bool s_interact_pending;
static bool s_idle_animation;
static bool s_awake;
static bool s_wander_due;
static bool s_greet_due;

static const GPoint s_neighbors[] = { { 0, 1 }, { 0, -1 }, { -1, 0 }, { 1, 0 } };
static const Direction s_facing_home[] = { DirectionDown, DirectionUp, DirectionLeft, DirectionRight };

static bool prv_trainer_open(GPoint cell) {
  return scene_can_stand(&s_scene, cell) && !gpoint_equal(&cell, &s_follower.cell) && !cameo_occupies(cell);
}

static bool prv_cameo_open(GPoint cell) {
  return scene_can_stand(&s_scene, cell)
      && !gpoint_equal(&cell, &s_trainer.cell) && !gpoint_equal(&cell, &s_trainer.from)
      && !gpoint_equal(&cell, &s_follower.cell) && !gpoint_equal(&cell, &s_follower.from);
}

static void prv_schedule_wander(uint32_t pause_ms);
static void prv_greet(void);
static void prv_schedule_greet(void);
static void prv_start_bob(void);

static void prv_start_step(void) {
  GPoint previous = s_trainer.cell;
  actor_step_to(&s_trainer, s_path[s_step]);
  actor_step_to(&s_follower, previous);
}

static void prv_step_tick(void *context) {
  s_step_timer = NULL;
  actor_advance(&s_trainer);
  actor_advance(&s_follower);
  if (!actor_is_moving(&s_trainer)) {
    if (++s_step >= s_path_length) {
      s_redraw();
      if (s_interact_pending) {
        prv_greet();
      } else {
        prv_schedule_wander(0);
      }
      return;
    }
    prv_start_step();
  }
  s_redraw();
  s_step_timer = app_timer_register(STEP_MS, prv_step_tick, NULL);
}

static uint8_t prv_wander_steps(void) {
  uint16_t steps = MIN_WANDER_STEPS + s_wander_seconds / SECONDS_PER_STEP;
  return steps > MAX_WANDER_STEPS ? MAX_WANDER_STEPS : steps;
}

static void prv_wander(void *context) {
  s_wander_timer = NULL;
  if (!s_awake) {
    s_wander_due = true;
    return;
  }
  s_path_length = path_random(s_trainer.cell, prv_wander_steps(), prv_trainer_open, s_path);
  if (!s_path_length) {
    prv_schedule_wander(0);
    return;
  }
  s_step = 0;
  prv_start_step();
  s_step_timer = app_timer_register(STEP_MS, prv_step_tick, NULL);
}

static void prv_schedule_wander(uint32_t pause_ms) {
  if (!s_wander_seconds) {
    return;
  }
  uint32_t jitter = rand() % (s_wander_seconds * 500 + 1);
  s_wander_timer = app_timer_register(pause_ms + s_wander_seconds * 1000 + jitter, prv_wander, NULL);
}

static void prv_greet(void) {
  s_interact_pending = false;
  actor_face(&s_trainer, s_follower.cell);
  actor_face(&s_follower, s_trainer.cell);
  emote_show();
  prv_schedule_wander(EMOTE_MS);
}

static void prv_place_actors(void) {
  static int16_t spots[CROP_COLS * CROP_ROWS];
  int16_t count = 0;
  for (int16_t i = 0; i < CROP_COLS * CROP_ROWS; i++) {
    if (scene_can_stand(&s_scene, GPoint(i % CROP_COLS, i / CROP_COLS))) {
      spots[count++] = i;
    }
  }
  int16_t home_index = count ? spots[rand() % count] : CROP_COLS * CROP_ROWS / 2;
  GPoint home = GPoint(home_index % CROP_COLS, home_index / CROP_COLS);
  actor_place(&s_trainer, home, DirectionDown);
  actor_place(&s_follower, home, DirectionDown);

  for (uint8_t i = 0; i < ARRAY_LENGTH(s_neighbors); i++) {
    GPoint next = GPoint(home.x - s_neighbors[i].x, home.y - s_neighbors[i].y);
    if (scene_can_stand(&s_scene, next)) {
      actor_place(&s_follower, next, s_facing_home[i]);
      return;
    }
  }
}

void world_load(const WorldConfig *config, RedrawHandler redraw) {
  s_redraw = redraw;
  s_wander_seconds = config->wander_seconds;
  scene_load(&s_scene, config->scene, config->zoom);
  actor_load(&s_trainer, ActorKindTrainer, config->trainer_sprite);
  actor_load(&s_follower, ActorKindPokemon, config->pokemon_sprite);
  if (s_scene.prop_sprite) {
    actor_load(&s_prop, ActorKindPokemon, s_scene.prop_sprite);
    actor_place(&s_prop, s_scene.prop_cell, DirectionDown);
  }
  prv_place_actors();
  cameo_load(&s_scene, redraw, prv_cameo_open);
  emote_load(redraw);
  s_interact_pending = false;
  s_idle_animation = config->idle_animation;
  s_awake = true;
  s_wander_due = false;
  s_greet_due = false;
  prv_schedule_wander(0);
  prv_schedule_greet();
  prv_start_bob();
}

void world_unload(void) {
  if (s_wander_timer) {
    app_timer_cancel(s_wander_timer);
    s_wander_timer = NULL;
  }
  if (s_step_timer) {
    app_timer_cancel(s_step_timer);
    s_step_timer = NULL;
  }
  if (s_greet_timer) {
    app_timer_cancel(s_greet_timer);
    s_greet_timer = NULL;
  }
  if (s_bob_timer) {
    app_timer_cancel(s_bob_timer);
    s_bob_timer = NULL;
  }
  emote_unload();
  cameo_unload();
  if (s_prop.sheet) {
    actor_unload(&s_prop);
  }
  actor_unload(&s_follower);
  actor_unload(&s_trainer);
  scene_unload(&s_scene);
}

void world_set_pokemon(uint32_t sprite) {
  actor_unload(&s_follower);
  actor_load(&s_follower, ActorKindPokemon, sprite);
}

static void prv_interact(void) {
  if (s_step_timer) {
    s_path_length = s_step + 1;
    s_interact_pending = true;
    return;
  }
  if (s_wander_timer) {
    app_timer_cancel(s_wander_timer);
    s_wander_timer = NULL;
  }
  prv_greet();
}

static void prv_greet_due(void *context) {
  s_greet_timer = NULL;
  prv_schedule_greet();
  if (!s_awake) {
    s_greet_due = true;
    return;
  }
  prv_interact();
}

static void prv_schedule_greet(void) {
  s_greet_timer = app_timer_register(GREET_MIN_MS + rand() % GREET_JITTER_MS, prv_greet_due, NULL);
}

static void prv_bob(void *context) {
  s_follower.bob = !s_follower.bob;
  s_prop.bob = s_follower.bob;
  s_redraw();
  s_bob_timer = app_timer_register(BOB_MS, prv_bob, NULL);
}

static void prv_start_bob(void) {
  if (s_idle_animation && !s_bob_timer) {
    s_bob_timer = app_timer_register(BOB_MS, prv_bob, NULL);
  }
}

static void prv_stop_bob(void) {
  if (s_bob_timer) {
    app_timer_cancel(s_bob_timer);
    s_bob_timer = NULL;
  }
  s_follower.bob = false;
  s_prop.bob = false;
}

void world_set_awake(bool awake) {
  if (awake == s_awake) {
    return;
  }
  s_awake = awake;
  cameo_set_awake(awake);
  if (!awake) {
    if (s_step_timer) {
      s_path_length = s_step + 1;
    }
    emote_hide();
    prv_stop_bob();
    s_redraw();
    return;
  }
  prv_start_bob();
  if (s_greet_due) {
    s_greet_due = false;
    s_wander_due = false;
    prv_interact();
  } else if (s_wander_due) {
    s_wander_due = false;
    prv_wander(NULL);
  }
}

void world_draw(Canvas *canvas, const uint8_t *tint) {
  const Actor *candidates[] = { &s_prop, cameo_actor(), &s_follower, &s_trainer };
  const Actor *actors[ARRAY_LENGTH(candidates)];
  uint8_t count = 0;
  for (uint8_t i = 0; i < ARRAY_LENGTH(candidates); i++) {
    if (candidates[i] && candidates[i]->sheet) {
      actors[count++] = candidates[i];
    }
  }
  for (uint8_t i = 1; i < count; i++) {
    for (uint8_t j = i; j > 0 && actor_world_position(actors[j - 1]).y > actor_world_position(actors[j]).y; j--) {
      const Actor *swap = actors[j];
      actors[j] = actors[j - 1];
      actors[j - 1] = swap;
    }
  }
  canvas_set_tint(canvas, s_scene.outdoor ? tint : NULL);
  scene_draw(&s_scene, canvas);
  for (uint8_t i = 0; i < count; i++) {
    actor_draw(actors[i], &s_scene, canvas);
  }
  canvas_set_tint(canvas, NULL);
  emote_draw(canvas, &s_scene, &s_follower);
}

bool world_is_glitching(void) {
  return cameo_is_glitching();
}
