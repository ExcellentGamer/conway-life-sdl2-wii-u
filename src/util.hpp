#include <SDL_rect.h>

bool checkRectCollision(const SDL_Rect& a, const SDL_Rect& b);
int signnum(int value); // [-1, 0, 1]
int mod(int a, int base);
int find_distance(int x1, int y1, int x2, int y2);
double clamp(double d, double min, double max);
SDL_Rect makeSDLRectfromFloat(float x, float y, int w, int h);
