#include "render.hpp"
#include "level.hpp"
#include <SDL_image.h>
#include <iostream>

// Set draw color
void render_set_color(SDL_Renderer* renderer, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

// Draw rectangle
void render_rectangle(SDL_Renderer* renderer, int x, int y, int w, int h, bool filled) {
    SDL_Rect rect = { x, y, w, h };
    if (filled) {
        SDL_RenderFillRect(renderer, &rect);
    } else {
        SDL_RenderDrawRect(renderer, &rect);
    }
}

// Render level with ground tiles
void render_level(SDL_Renderer* renderer, SDL_Texture* tileset) {
    SDL_Rect src, dst;
    const int tiles_per_row = 8; // depends on your tileset.png layout

    src.w = dst.w = TILE_WIDTH;
    src.h = dst.h = TILE_HEIGHT;

    for (int y = 0; y < MAX_HEIGHT; ++y) {
        for (int x = 0; x < MAX_WIDTH; ++x) {
            int tile_id = level[y][x];
            if (tile_id == 0) continue; // Skip empty/air

            dst.x = x * TILE_WIDTH;
            dst.y = y * TILE_HEIGHT;

            src.x = (tile_id % tiles_per_row) * TILE_WIDTH;
            src.y = (tile_id / tiles_per_row) * TILE_HEIGHT;

            SDL_RenderCopy(renderer, tileset, &src, &dst);
        }
    }
}
