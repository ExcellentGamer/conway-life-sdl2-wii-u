#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <map>

#include "level.hpp"

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
            current_tileset = line.substr(9);
            continue;
        }

        if (y >= MAX_HEIGHT) break;

        std::istringstream iss(line);
        std::string token;
        int x = 0;

        while (iss >> token && x < MAX_WIDTH) {
            if (token == "mm" && !marioSpawnSet) {
                mario_x = x * TILE_WIDTH;
                mario_y = y * TILE_HEIGHT;
                marioSpawnSet = true;
                level[y][x] = 0;
            } else {
                int value;
                std::istringstream hexstream(token);
                hexstream >> std::hex >> value;
                level[y][x] = value;
            }
            ++x;
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
    return tile_id >= 1 && tile_id <= 29;
}
