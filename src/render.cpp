#include "render.hpp"
#include "level.hpp"

const RenderColor BACKGROUND_OVERWORLD = {92, 148, 252, SDL_ALPHA_OPAQUE};
const RenderColor COLOR_CURSOR = {0, 0, 0, SDL_ALPHA_OPAQUE};
const RenderColor COLOR_WHITE = {245, 245, 245, SDL_ALPHA_OPAQUE};
const RenderColor COLOR_RED = {245, 25, 25, SDL_ALPHA_OPAQUE};
const RenderColor COLOR_GREEN = {25, 245, 25, SDL_ALPHA_OPAQUE};

void render_set_color(SDL_Renderer *renderer, RenderColor color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

void render_rectangle(SDL_Renderer *renderer, int x, int y, int w, int h, bool fill) {
    SDL_Rect rect = {x, y, w, h};
    if (fill)
        SDL_RenderFillRect(renderer, &rect);
    else
        SDL_RenderDrawRect(renderer, &rect);
}

void render_level(SDL_Renderer *renderer) {
    for (int y = 0; y < MAX_HEIGHT; ++y) {
        for (int x = 0; x < MAX_WIDTH; ++x) {
            if (level[y][x] == TILE_SOLID) {
                render_set_color(renderer, {0, 0, 255, 255}); // Blue solid block
                render_rectangle(renderer, x * TILE_WIDTH, y * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT, true);
            }
        }
    }
}
