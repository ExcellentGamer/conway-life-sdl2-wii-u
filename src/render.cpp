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
void render_level(SDL_Renderer* renderer, SDL_Texture* ground_texture) {
    for (int y = 0; y < MAX_HEIGHT; ++y) {
        for (int x = 0; x < MAX_WIDTH; ++x) {
            if (level[y][x] == TILE_SOLID && ground_texture) {
                SDL_Rect dest = {
                    x * TILE_WIDTH,
                    y * TILE_HEIGHT,
                    TILE_WIDTH,
                    TILE_HEIGHT
                };
                SDL_RenderCopy(renderer, ground_texture, NULL, &dest);
            }
        }
    }
}
