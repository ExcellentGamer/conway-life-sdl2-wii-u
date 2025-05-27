#pragma once

const int TILE_WIDTH = 16;
const int TILE_HEIGHT = 16;
const int MAX_WIDTH = 128;
const int MAX_HEIGHT = 64;

enum TileType {
    TILE_AIR,
    TILE_SOLID
};

extern TileType level[MAX_HEIGHT][MAX_WIDTH];

bool loadLevel(const char* filename);
bool is_solid_at(float x, float y);
