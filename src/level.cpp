#include "level.hpp"
#include <fstream>
#include <string>
#include <iostream>

TileType level[MAX_HEIGHT][MAX_WIDTH];

bool loadLevel(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open level file!\n";
        return false;
    }

    std::string line;
    int y = 0;
    while (std::getline(file, line) && y < MAX_HEIGHT) {
        for (size_t x = 0; x < line.length() && x < MAX_WIDTH; ++x) {
            if (line[x] == '#') {
                level[y][x] = TILE_SOLID;
            } else {
                level[y][x] = TILE_AIR;
            }
        }
        ++y;
    }

    file.close();
    return true;
}

bool is_solid_at(float x, float y) {
    int tile_x = (int)(x) / TILE_WIDTH;
    int tile_y = (int)(y) / TILE_HEIGHT;

    if (tile_x < 0 || tile_x >= MAX_WIDTH || tile_y < 0 || tile_y >= MAX_HEIGHT) {
        return false; // Out of bounds is considered empty
    }

    return level[tile_y][tile_x] == TILE_SOLID;
}
