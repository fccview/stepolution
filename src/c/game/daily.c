#include "game/daily.h"
#include "game/pokemon.h"
#include "world/scene.h"

#define STORAGE_KEY 2

typedef struct {
  int32_t day;
  DailyPick pick;
} DailyState;

static DailyState s_state;

static int32_t prv_day(const struct tm *now) {
  return (now->tm_year + 1900) * 1000 + now->tm_yday;
}

static void prv_save(void) {
  persist_write_data(STORAGE_KEY, &s_state, sizeof(s_state));
}

void daily_reroll_starter(void) {
  s_state.pick.starter = rand() % pokemon_count();
  prv_save();
}

void daily_reroll_scene(void) {
  s_state.pick.scene = rand() % scene_count();
  prv_save();
}

bool daily_roll_over(const struct tm *now) {
  int32_t day = prv_day(now);
  if (day == s_state.day) {
    return false;
  }
  s_state.day = day;
  daily_reroll_starter();
  daily_reroll_scene();
  return true;
}

void daily_init(const struct tm *now) {
  if (persist_get_size(STORAGE_KEY) == sizeof(s_state)) {
    persist_read_data(STORAGE_KEY, &s_state, sizeof(s_state));
  }
  daily_roll_over(now);
}

DailyPick daily_resolve(uint8_t starter, uint8_t scene) {
  return (DailyPick) {
    .starter = starter == DAILY_RANDOM ? s_state.pick.starter : starter,
    .scene = scene == DAILY_RANDOM ? s_state.pick.scene : scene,
  };
}
