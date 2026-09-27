#pragma once

#include "world/actor.h"
#include "world/path.h"

void cameo_load(const Scene *scene, RedrawHandler redraw, PathOpenHandler is_open);
void cameo_unload(void);
void cameo_set_awake(bool awake);
const Actor *cameo_actor(void);
bool cameo_occupies(GPoint cell);
bool cameo_is_glitching(void);
