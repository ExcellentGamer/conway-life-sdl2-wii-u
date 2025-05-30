#include <SDL_image.h>

#include "render.hpp"
#include "config.hpp"
#include "level.hpp"
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
void render_level(SDL_Renderer* renderer, SDL_Texture* tileset, float camera_x) {
    SDL_Rect src, dst;
    const int tiles_per_row = 4; // depends on your tileset layout

    src.w = dst.w = TILE_WIDTH;
    src.h = dst.h = TILE_HEIGHT;

    // Calculate visible tile range on screen (start and end tile columns)
    int tile_start_x = (int)(camera_x / TILE_WIDTH);
    int tile_end_x = (int)((camera_x + WINDOW_WIDTH) / TILE_WIDTH) + 1;

    // Clamp to level bounds
    if (tile_end_x > MAX_WIDTH) tile_end_x = MAX_WIDTH;

    for (int y = 0; y < MAX_HEIGHT; ++y) {
        for (int x = tile_start_x; x < tile_end_x; ++x) {
            int tile_id = level[y][x];
            if (tile_id == 0) continue; // Skip air

            dst.x = x * TILE_WIDTH - (int)camera_x; // Camera offset
            dst.y = y * TILE_HEIGHT;

            src.x = (tile_id % tiles_per_row) * TILE_WIDTH;
            src.y = (tile_id / tiles_per_row) * TILE_HEIGHT;

            SDL_RenderCopy(renderer, tileset, &src, &dst);
        }
    }
}
