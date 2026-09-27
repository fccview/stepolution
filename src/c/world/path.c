#include "world/path.h"
#include "layout.h"

#define CELL_COUNT (CROP_COLS * CROP_ROWS)
#define UNVISITED 0xFF

static const GPoint s_neighbors[] = { { 0, 1 }, { 0, -1 }, { -1, 0 }, { 1, 0 } };

static int16_t prv_index(GPoint cell) {
  return cell.y * CROP_COLS + cell.x;
}

static GPoint prv_cell(int16_t index) {
  return GPoint(index % CROP_COLS, index / CROP_COLS);
}

static bool prv_in_bounds(GPoint cell) {
  return cell.x >= 0 && cell.y >= 0 && cell.x < CROP_COLS && cell.y < CROP_ROWS;
}

uint8_t path_random(GPoint start, uint8_t max_steps, PathOpenHandler is_open, GPoint *out) {
  static int16_t parent[CELL_COUNT];
  static uint8_t distance[CELL_COUNT];
  static int16_t queue[CELL_COUNT];
  memset(distance, UNVISITED, sizeof(distance));

  int16_t head = 0, tail = 0;
  distance[prv_index(start)] = 0;
  queue[tail++] = prv_index(start);

  while (head < tail) {
    int16_t current = queue[head++];
    if (distance[current] >= max_steps) {
      continue;
    }
    GPoint cell = prv_cell(current);
    for (uint8_t i = 0; i < ARRAY_LENGTH(s_neighbors); i++) {
      GPoint next = GPoint(cell.x + s_neighbors[i].x, cell.y + s_neighbors[i].y);
      if (!prv_in_bounds(next) || distance[prv_index(next)] != UNVISITED || !is_open(next)) {
        continue;
      }
      int16_t index = prv_index(next);
      distance[index] = distance[current] + 1;
      parent[index] = current;
      queue[tail++] = index;
    }
  }

  if (tail <= 1) {
    return 0;
  }
  int16_t target = queue[1 + rand() % (tail - 1)];
  uint8_t length = distance[target];
  for (int16_t step = length - 1, index = target; step >= 0; step--, index = parent[index]) {
    out[step] = prv_cell(index);
  }
  return length;
}
