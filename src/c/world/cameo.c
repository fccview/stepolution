#include "world/cameo.h"

#define MAX_STEPS 3
#define LINGER_MS 2500
#define STAY_MS 15000
#define FLICKER_MS 110
#define FLICKER_TICKS 9

typedef enum {
  CameoHidden,
  CameoOut,
  CameoLinger,
  CameoBack,
  CameoFlickerIn,
  CameoStay,
  CameoFlickerOut,
} CameoPhase;

typedef struct {
  uint32_t min_delay_ms;
  uint32_t jitter_ms;
} CameoTiming;

static const CameoTiming s_timings[] = {
  [CameoStyleEmerge] = { 10 * 60 * 1000, 20 * 60 * 1000 },
  [CameoStyleApparition] = { 20 * 60 * 1000, 40 * 60 * 1000 },
};

static Actor s_actor;
static CameoStyle s_style;
static GPoint s_home;
static RedrawHandler s_redraw;
static PathOpenHandler s_open;
static AppTimer *s_timer;
static CameoPhase s_phase;
static GPoint s_path[MAX_STEPS];
static uint8_t s_length;
static uint8_t s_step;
static AppTimerCallback s_callback;
static uint64_t s_deadline;
static uint32_t s_remaining;
static bool s_awake;
static bool s_due;
static bool s_paused;

static uint64_t prv_now_ms(void) {
  time_t seconds;
  uint16_t millis;
  time_ms(&seconds, &millis);
  return (uint64_t)seconds * 1000 + millis;
}

static void prv_schedule(uint32_t delay, AppTimerCallback callback) {
  s_callback = callback;
  s_deadline = prv_now_ms() + delay;
  s_timer = app_timer_register(delay, callback, NULL);
}

static void prv_appear(void *context);

static void prv_hide(void) {
  s_phase = CameoHidden;
  actor_place(&s_actor, s_home, DirectionDown);
  const CameoTiming *timing = &s_timings[s_style];
  prv_schedule(timing->min_delay_ms + rand() % timing->jitter_ms, prv_appear);
}

static GPoint prv_target(uint8_t step) {
  if (s_phase == CameoOut) {
    return s_path[step];
  }
  return step + 1 < s_length ? s_path[s_length - 2 - step] : s_home;
}

static void prv_walk(void *context) {
  s_timer = NULL;
  actor_advance(&s_actor);
  s_redraw();
  if (actor_is_moving(&s_actor)) {
    prv_schedule(STEP_MS, prv_walk);
    return;
  }
  if (++s_step < s_length) {
    actor_step_to(&s_actor, prv_target(s_step));
    prv_schedule(STEP_MS, prv_walk);
  } else if (s_phase == CameoOut) {
    s_phase = CameoLinger;
    prv_schedule(LINGER_MS, prv_walk);
  } else if (s_phase == CameoLinger) {
    s_phase = CameoBack;
    s_step = 0;
    actor_step_to(&s_actor, prv_target(0));
    prv_schedule(STEP_MS, prv_walk);
  } else {
    prv_hide();
    s_redraw();
  }
}

static void prv_flicker(void *context) {
  s_timer = NULL;
  s_step++;
  s_redraw();
  if (s_step < FLICKER_TICKS) {
    prv_schedule(FLICKER_MS, prv_flicker);
  } else if (s_phase == CameoFlickerIn) {
    s_phase = CameoStay;
    prv_schedule(STAY_MS, prv_flicker);
  } else if (s_phase == CameoStay) {
    s_phase = CameoFlickerOut;
    s_step = 0;
    prv_schedule(FLICKER_MS, prv_flicker);
  } else {
    prv_hide();
  }
}

static bool prv_emerge(void) {
  s_length = path_random(s_home, MAX_STEPS, s_open, s_path);
  if (!s_length) {
    return false;
  }
  s_phase = CameoOut;
  s_step = 0;
  actor_step_to(&s_actor, s_path[0]);
  prv_schedule(STEP_MS, prv_walk);
  return true;
}

static bool prv_materialize(void) {
  static GPoint spots[CROP_COLS * CROP_ROWS];
  uint16_t count = 0;
  for (int16_t y = 0; y < CROP_ROWS; y++) {
    for (int16_t x = 0; x < CROP_COLS; x++) {
      if (s_open(GPoint(x, y))) {
        spots[count++] = GPoint(x, y);
      }
    }
  }
  if (!count) {
    return false;
  }
  actor_place(&s_actor, spots[rand() % count], DirectionDown);
  s_phase = CameoFlickerIn;
  s_step = 0;
  prv_schedule(FLICKER_MS, prv_flicker);
  return true;
}

static void prv_appear(void *context) {
  s_timer = NULL;
  if (!s_awake) {
    s_due = true;
    return;
  }
  bool shown = s_style == CameoStyleEmerge ? prv_emerge() : prv_materialize();
  if (!shown) {
    prv_hide();
  }
}

void cameo_load(const Scene *scene, RedrawHandler redraw, PathOpenHandler is_open) {
  if (!scene->cameo_sprite) {
    return;
  }
  s_redraw = redraw;
  s_open = is_open;
  s_style = scene->cameo_style;
  s_awake = true;
  s_due = false;
  s_paused = false;
  s_home = scene->cameo_cell;
  actor_load(&s_actor, ActorKindPokemon, scene->cameo_sprite);
  prv_hide();
}

void cameo_unload(void) {
  if (s_timer) {
    app_timer_cancel(s_timer);
    s_timer = NULL;
  }
  if (s_actor.sheet) {
    actor_unload(&s_actor);
  }
  s_phase = CameoHidden;
}

void cameo_set_awake(bool awake) {
  if (!s_actor.sheet) {
    return;
  }
  s_awake = awake;
  if (!awake) {
    if (s_phase != CameoHidden && s_timer) {
      uint64_t now = prv_now_ms();
      s_remaining = s_deadline > now ? s_deadline - now : 0;
      app_timer_cancel(s_timer);
      s_timer = NULL;
      s_paused = true;
    }
    return;
  }
  if (s_paused) {
    s_paused = false;
    prv_schedule(s_remaining, s_callback);
  } else if (s_due) {
    s_due = false;
    prv_appear(NULL);
  }
}

const Actor *cameo_actor(void) {
  bool flickering = s_phase == CameoFlickerIn || s_phase == CameoFlickerOut;
  if (s_phase == CameoHidden || (flickering && s_step % 2)) {
    return NULL;
  }
  return &s_actor;
}

bool cameo_occupies(GPoint cell) {
  return s_phase != CameoHidden && (gpoint_equal(&cell, &s_actor.cell) || gpoint_equal(&cell, &s_actor.from));
}

bool cameo_is_glitching(void) {
  return s_style == CameoStyleApparition && s_phase != CameoHidden;
}
