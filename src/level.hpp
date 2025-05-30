#pragma once
#include <string>

const int TILE_WIDTH = 48;
const int TILE_HEIGHT = 48;
const int MAX_WIDTH = 256*3;
const int MAX_HEIGHT = 15*3;

extern int level[MAX_HEIGHT][MAX_WIDTH];
extern std::string current_tileset;

int get_level_width_tiles();
bool loadLevel(const std::string& filename);
void unloadLevel();
bool is_solid_at(float x, float y);
