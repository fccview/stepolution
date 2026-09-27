#pragma once

#include "world/actor.h"

#define EMOTE_MS 3000

void emote_load(RedrawHandler redraw);
void emote_unload(void);
void emote_show(void);
void emote_hide(void);
void emote_draw(Canvas *canvas, const Scene *scene, const Actor *anchor);
