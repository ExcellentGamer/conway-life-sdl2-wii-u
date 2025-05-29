#include "level.hpp"
#include <fstream>
#include <string>
#include <iostream>
#include <map>

extern float mario_x;
extern float mario_y;

int level[MAX_HEIGHT][MAX_WIDTH];
std::string current_tileset = "overworld"; // fallback default

bool loadLevel(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open level file!\n";
        return false;
    }

    std::string line;
    int y = 0;
    bool marioSpawnSet = false;

    while (std::getline(file, line)) {
        if (line.rfind("#tileset", 0) == 0) {
            // This is the tileset declaration
            current_tileset = line.substr(9); // Skip "#tileset "
            continue;
        }

        if (y >= MAX_HEIGHT) break;

        for (size_t x = 0; x < line.length() && x < MAX_WIDTH; ++x) {
            char tile = line[x];

            if (tile == 'm' && !marioSpawnSet) {
                mario_x = x * TILE_WIDTH;
                mario_y = y * TILE_HEIGHT;
                marioSpawnSet = true;
                level[y][x] = 0;
            } else if (tile >= '0' && tile <= '9') {
                level[y][x] = tile - '0';
            } else {
                level[y][x] = 0;
            }
        }

        ++y;
    }

    file.close();
    return true;
}

bool is_solid_at(float x, float y) {
    int tile_x = static_cast<int>(x) / TILE_WIDTH;
    int tile_y = static_cast<int>(y) / TILE_HEIGHT;

    if (tile_x < 0 || tile_x >= MAX_WIDTH || tile_y < 0 || tile_y >= MAX_HEIGHT) {
        return false;
    }

    int tile_id = level[tile_y][tile_x];
    return tile_id == 1; // Treat tile index 1 as solid (you can expand this)
}
