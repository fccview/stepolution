#include "game/pokemon.h"

#define START_LEVEL 5
#define MAX_LEVEL 100
#define LEVELS_PER_EXTRA_GOAL 50
#define PERMILLE 1000
#define HALF_GOAL (PERMILLE / 2)

static const Starter s_starters[] = {
  { { "BULBASAUR", "IVYSAUR", "VENUSAUR" },
    { RESOURCE_ID_BULBASAUR, RESOURCE_ID_IVYSAUR, RESOURCE_ID_VENUSAUR }, { 16, 32 } },
  { { "CHARMANDER", "CHARMELEON", "CHARIZARD" },
    { RESOURCE_ID_CHARMANDER, RESOURCE_ID_CHARMELEON, RESOURCE_ID_CHARIZARD }, { 16, 36 } },
  { { "SQUIRTLE", "WARTORTLE", "BLASTOISE" },
    { RESOURCE_ID_SQUIRTLE, RESOURCE_ID_WARTORTLE, RESOURCE_ID_BLASTOISE }, { 16, 36 } },
  { { "CATERPIE", "METAPOD", "BUTTERFREE" },
    { RESOURCE_ID_CATERPIE, RESOURCE_ID_METAPOD, RESOURCE_ID_BUTTERFREE }, { 7, 10 } },
  { { "WEEDLE", "KAKUNA", "BEEDRILL" },
    { RESOURCE_ID_WEEDLE, RESOURCE_ID_KAKUNA, RESOURCE_ID_BEEDRILL }, { 7, 10 } },
  { { "PIDGEY", "PIDGEOTTO", "PIDGEOT" },
    { RESOURCE_ID_PIDGEY, RESOURCE_ID_PIDGEOTTO, RESOURCE_ID_PIDGEOT }, { 18, 36 } },
  { { "NIDORAN", "NIDORINA", "NIDOQUEEN" },
    { RESOURCE_ID_NIDORAN_F, RESOURCE_ID_NIDORINA, RESOURCE_ID_NIDOQUEEN }, { 16, 36 } },
  { { "NIDORAN", "NIDORINO", "NIDOKING" },
    { RESOURCE_ID_NIDORAN_M, RESOURCE_ID_NIDORINO, RESOURCE_ID_NIDOKING }, { 16, 36 } },
  { { "ODDISH", "GLOOM", "VILEPLUME" },
    { RESOURCE_ID_ODDISH, RESOURCE_ID_GLOOM, RESOURCE_ID_VILEPLUME }, { 21, 36 } },
  { { "POLIWAG", "POLIWHIRL", "POLIWRATH" },
    { RESOURCE_ID_POLIWAG, RESOURCE_ID_POLIWHIRL, RESOURCE_ID_POLIWRATH }, { 25, 36 } },
  { { "ABRA", "KADABRA", "ALAKAZAM" },
    { RESOURCE_ID_ABRA, RESOURCE_ID_KADABRA, RESOURCE_ID_ALAKAZAM }, { 16, 36 } },
  { { "MACHOP", "MACHOKE", "MACHAMP" },
    { RESOURCE_ID_MACHOP, RESOURCE_ID_MACHOKE, RESOURCE_ID_MACHAMP }, { 28, 40 } },
  { { "BELLSPROUT", "WEEPINBELL", "VICTREEBEL" },
    { RESOURCE_ID_BELLSPROUT, RESOURCE_ID_WEEPINBELL, RESOURCE_ID_VICTREEBEL }, { 21, 36 } },
  { { "GEODUDE", "GRAVELER", "GOLEM" },
    { RESOURCE_ID_GEODUDE, RESOURCE_ID_GRAVELER, RESOURCE_ID_GOLEM }, { 25, 40 } },
  { { "GASTLY", "HAUNTER", "GENGAR" },
    { RESOURCE_ID_GASTLY, RESOURCE_ID_HAUNTER, RESOURCE_ID_GENGAR }, { 25, 40 } },
  { { "DRATINI", "DRAGONAIR", "DRAGONITE" },
    { RESOURCE_ID_DRATINI, RESOURCE_ID_DRAGONAIR, RESOURCE_ID_DRAGONITE }, { 30, 55 } },
  { { "PICHU", "PIKACHU", "RAICHU" },
    { RESOURCE_ID_PICHU, RESOURCE_ID_PIKACHU, RESOURCE_ID_RAICHU }, { 12, 30 } },
};

uint8_t pokemon_count(void) {
  return ARRAY_LENGTH(s_starters);
}

const Starter *pokemon_starter(uint8_t index) {
  return &s_starters[index % ARRAY_LENGTH(s_starters)];
}

static int32_t prv_lerp(int32_t from, int32_t to, int32_t t, int32_t span) {
  return from + (to - from) * t / span;
}

Growth pokemon_growth(const Starter *starter, int32_t steps, int32_t goal) {
  int32_t progress = steps * PERMILLE / (goal > 0 ? goal : 1);
  int32_t first = starter->evolve_levels[0] * 100;
  int32_t second = starter->evolve_levels[1] * 100;
  int32_t level_x100;
  uint8_t stage;

  if (progress < HALF_GOAL) {
    stage = 0;
    level_x100 = prv_lerp(START_LEVEL * 100, first, progress, HALF_GOAL);
  } else if (progress < PERMILLE) {
    stage = 1;
    level_x100 = prv_lerp(first, second, progress - HALF_GOAL, HALF_GOAL);
  } else {
    stage = 2;
    level_x100 = second + (progress - PERMILLE) * LEVELS_PER_EXTRA_GOAL * 100 / PERMILLE;
  }

  if (level_x100 >= MAX_LEVEL * 100) {
    return (Growth) { .stage = stage, .level = MAX_LEVEL, .exp_percent = 100 };
  }
  return (Growth) { .stage = stage, .level = level_x100 / 100, .exp_percent = level_x100 % 100 };
}
