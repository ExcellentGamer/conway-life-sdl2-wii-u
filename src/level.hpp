#pragma once
#include <string>

const int TILE_WIDTH = 16*3;
const int TILE_HEIGHT = 16*3;
const int MAX_WIDTH = 256*3;
const int MAX_HEIGHT = 15*3;

extern int level[MAX_HEIGHT][MAX_WIDTH];
extern std::string current_tileset;

bool loadLevel(const char* filename);
bool is_solid_at(float x, float y);
