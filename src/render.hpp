#include <SDL_render.h>

typedef struct {
    int rr;
    int gg;
    int bb;
    int aa;
} RenderColor;

const RenderColor BACKGROUND_OVERWORLD = {92, 148, 252, SDL_ALPHA_OPAQUE};
const RenderColor COLOR_CURSOR = {0, 0, 0, SDL_ALPHA_OPAQUE};
const RenderColor COLOR_WHITE = {245, 245, 245, SDL_ALPHA_OPAQUE};
const RenderColor COLOR_RED = {245, 25, 25, SDL_ALPHA_OPAQUE};
const RenderColor COLOR_GREEN = {25, 245, 25, SDL_ALPHA_OPAQUE};

void render_set_color(SDL_Renderer *renderer, RenderColor color);
void render_rectangle(SDL_Renderer *renderer, int xx, int yy, int ww, int hh, bool filled);
