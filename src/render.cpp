#include <SDL.h>
#include <SDL_image.h>

#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "render.hpp"
#include "config.hpp"
#include "level.hpp"

std::map<std::string, SDL_Texture*> tilesets;

extern int level[MAX_HEIGHT][MAX_WIDTH];
extern std::string current_tileset;

// Set draw color and clear screen
void render_set_color(SDL_Renderer* renderer, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_RenderClear(renderer);
}

// Rectangle render helper
void render_rectangle(SDL_Renderer* renderer, int x, int y, int w, int h, bool filled) {
    SDL_Rect rect = { x, y, w, h };
    if (filled)
        SDL_RenderFillRect(renderer, &rect);
    else
        SDL_RenderDrawRect(renderer, &rect);
}

// Render visible level section
void render_level(SDL_Renderer* renderer, SDL_Texture* tileset, float camera_x) {
    SDL_Rect src, dst;
    const int tiles_per_row = 4;

    src.w = dst.w = TILE_WIDTH;
    src.h = dst.h = TILE_HEIGHT;

    int tile_start_x = (int)(camera_x / TILE_WIDTH);
    int tile_end_x = (int)((camera_x + WINDOW_WIDTH) / TILE_WIDTH) + 1;
    if (tile_end_x > MAX_WIDTH) tile_end_x = MAX_WIDTH;

    for (int y = 0; y < MAX_HEIGHT; ++y) {
        for (int x = tile_start_x; x < tile_end_x; ++x) {
            int tile_id = level[y][x];
            if (tile_id == 0) continue;

            dst.x = x * TILE_WIDTH - (int)camera_x;
            dst.y = y * TILE_HEIGHT;

            src.x = (tile_id % tiles_per_row) * TILE_WIDTH;
            src.y = (tile_id / tiles_per_row) * TILE_HEIGHT;

            SDL_RenderCopy(renderer, tileset, &src, &dst);
        }
    }
}

// Load all tilesets
void load_tilesets(SDL_Renderer* renderer) {
    tilesets["overworld"] = IMG_LoadTexture(renderer, "/vol/external01/smb/assets/tilesets/overworld.png");
    tilesets["underground"] = IMG_LoadTexture(renderer, "/vol/external01/smb/assets/tilesets/underground.png");
}

// Get active tileset
SDL_Texture* get_current_tileset() {
    auto it = tilesets.find(current_tileset);
    if (it != tilesets.end()) return it->second;
    return tilesets["overworld"]; // fallback
}
