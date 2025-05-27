#pragma once

#include <SDL_render.h>

typedef struct {
    Uint8 r, g, b, a;
} RenderColor;

extern const RenderColor BACKGROUND_OVERWORLD;
extern const RenderColor COLOR_CURSOR;
extern const RenderColor COLOR_WHITE;
extern const RenderColor COLOR_RED;
extern const RenderColor COLOR_GREEN;

void render_set_color(SDL_Renderer *renderer, RenderColor color);
void render_rectangle(SDL_Renderer *renderer, int x, int y, int w, int h, bool fill);
void render_level(SDL_Renderer *renderer);
