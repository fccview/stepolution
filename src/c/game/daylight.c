#include "game/daylight.h"

#define TINT_SIZE 64
#define DAWN_HOUR 6
#define MORNING_HOUR 7
#define DUSK_HOUR 18
#define NIGHT_HOUR 20

static uint8_t s_dusk[TINT_SIZE];
static uint8_t s_night[TINT_SIZE];

void daylight_load(void) {
  resource_load(resource_get_handle(RESOURCE_ID_TINT_DUSK), s_dusk, TINT_SIZE);
  resource_load(resource_get_handle(RESOURCE_ID_TINT_NIGHT), s_night, TINT_SIZE);
}

static DaylightMode prv_mode_at(const struct tm *time) {
  int hour = time->tm_hour;
  if (hour >= NIGHT_HOUR || hour < DAWN_HOUR) {
    return DaylightNight;
  }
  if (hour >= DUSK_HOUR || hour < MORNING_HOUR) {
    return DaylightDusk;
  }
  return DaylightDay;
}

const uint8_t *daylight_tint(DaylightMode mode, const struct tm *time) {
  switch (mode == DaylightCycle ? prv_mode_at(time) : mode) {
    case DaylightDusk: return s_dusk;
    case DaylightNight: return s_night;
    default: return NULL;
  }
}
