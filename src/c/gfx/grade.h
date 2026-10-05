#pragma once

#include <pebble.h>

typedef enum {
  GradeOff,
  GradeSoft,
  GradeStrong,
} GradeMode;

void grade_load(void);
const uint8_t *grade_table(GradeMode mode);
