#include <SDL.h>
#include <SDL_ttf.h>
#include <math.h>
#include "util.hpp"

bool checkRectCollision(const SDL_Rect& a, const SDL_Rect& b) {
    return a.x < b.x + b.w && // Collision on the Left of a and the Right of b
           a.x + a.w > b.x && // Collision on the Right of a and the Left of b
           a.y < b.y + b.h && // Collision on the Bottom of a and the Top of b
           a.y + a.h > b.y;   // Collision on the Top of a and the Bottom of b
}

int signnum(int value) {
    return (0 < value) - (value < 0);
}

int mod(int a, int base) {
    return ((a % base) + base) % base;
}

int find_distance(int x1, int y1, int x2, int y2) {
    return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
}

double clamp(double d, double min, double max) {
  const double t = d < min ? min : d;
  return t > max ? max : t;
}

void render_text(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text, color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dst = { x, y, surface->w, surface->h };

    SDL_FreeSurface(surface);
    if (!texture) return;

    SDL_RenderCopy(renderer, texture, NULL, &dst);
    SDL_DestroyTexture(texture);
}

SDL_Rect makeSDLRectfromFloat(float x, float y, int w, int h) {
    return SDL_Rect{ (int)x, (int)y, w, h };
}
