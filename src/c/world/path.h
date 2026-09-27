#pragma once

#include <pebble.h>

typedef bool (*PathOpenHandler)(GPoint cell);

uint8_t path_random(GPoint start, uint8_t max_steps, PathOpenHandler is_open, GPoint *out);
