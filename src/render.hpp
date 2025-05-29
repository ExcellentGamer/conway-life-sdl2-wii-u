#pragma once

#include <SDL_render.h>

typedef struct {
    Uint8 r, g, b, a;
} RenderColor;

// Background Colors
const SDL_Color BACKGROUND_OVERWORLD = { 92, 148, 252, 255 };
const SDL_Color COLOR_CURSOR = { 255, 0, 0, 255 };
const SDL_Color COLOR_WHITE = { 245, 245, 245, 255 };
const SDL_Color COLOR_RED = { 245, 25, 25, 255 };
const SDL_Color COLOR_GREEN = { 25, 245, 25, 255 };

// Drawing helpers
void render_set_color(SDL_Renderer* renderer, SDL_Color color);
void render_rectangle(SDL_Renderer* renderer, int x, int y, int w, int h, bool filled);
SDL_Texture* load_texture(const char* path, SDL_Renderer* renderer);

// Updated level rendering function
void render_level(SDL_Renderer* renderer, SDL_Texture* tileset);