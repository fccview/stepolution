#include "gfx/grade.h"

#define GRADE_SIZE 64

static uint8_t s_soft[GRADE_SIZE];
static uint8_t s_strong[GRADE_SIZE];

void grade_load(void) {
  resource_load(resource_get_handle(RESOURCE_ID_GRADE_SOFT), s_soft, GRADE_SIZE);
  resource_load(resource_get_handle(RESOURCE_ID_GRADE_STRONG), s_strong, GRADE_SIZE);
}

const uint8_t *grade_table(GradeMode mode) {
  switch (mode) {
    case GradeSoft: return s_soft;
    case GradeStrong: return s_strong;
    default: return NULL;
  }
}
