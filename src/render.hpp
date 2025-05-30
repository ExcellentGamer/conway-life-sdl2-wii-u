#pragma once

#include <map>
#include <string>
#include <SDL_render.h>

typedef struct {
    Uint8 r, g, b, a;
} RenderColor;


extern std::map<std::string, SDL_Texture*> tilesets;

// Tileset Related
void load_tilesets(SDL_Renderer* renderer);
SDL_Texture* get_current_tileset();

// Drawing helpers
void render_set_color(SDL_Renderer* renderer, SDL_Color color);
void render_rectangle(SDL_Renderer* renderer, int x, int y, int w, int h, bool filled);
SDL_Texture* load_texture(const char* path, SDL_Renderer* renderer);

// Updated level rendering function
void render_level(SDL_Renderer* renderer, SDL_Texture* tileset, float camera_x);
