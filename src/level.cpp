#include <SDL2/SDL.h>
#include <SDL_image.h>

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <map>

#include "level.hpp"

extern float mario_x;
extern float mario_y;
extern SDL_Color backgroundColor;

int level[MAX_HEIGHT][MAX_WIDTH];
std::string current_tileset = "overworld"; // fallback default

bool loadLevel(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open level file!\n";
        return false;
    }

    std::string line;
    int y = 0;
    bool marioSpawnSet = false;
    bool bgColorSet = false;

    while (std::getline(file, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.empty()) continue;

        // Tileset metadata
        if (line.rfind("#tileset", 0) == 0) {
            std::string raw = line.substr(9);
            raw.erase(0, raw.find_first_not_of(" \t"));
            raw.erase(raw.find_last_not_of(" \t") + 1);
            current_tileset = raw;
            continue;
        }

        // Background color
        if (!bgColorSet && line.rfind("BACKGROUND_COLOR", 0) == 0) {
            std::istringstream ss(line);
            std::string token;
            int r, g, b;
            ss >> token >> r >> g >> b;
            backgroundColor = { (Uint8)r, (Uint8)g, (Uint8)b };
            bgColorSet = true;
            continue;
        }

        if (line[0] == '#') continue;  // Skip other comments

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
                int value = 0;
                std::istringstream parser(token);
                if (token.find_first_not_of("0123456789") == std::string::npos) {
                    parser >> value; // Decimal
                } else {
                    parser >> std::hex >> value; // Hex
                }
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
    // Hard wall at screen left edge (x = 0)
    if (x < 0.0f) {
        return true;
    }

    int tile_x = static_cast<int>(x) / TILE_WIDTH;
    int tile_y = static_cast<int>(y) / TILE_HEIGHT;

    if (tile_x < 0 || tile_x >= MAX_WIDTH || tile_y < 0 || tile_y >= MAX_HEIGHT) {
        return false;
    }

    int tile_id = level[tile_y][tile_x];
    return tile_id >= 1 && tile_id <= 29;
}
