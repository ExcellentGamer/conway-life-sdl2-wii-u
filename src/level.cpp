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
extern float mario_spawn_x;
extern float mario_spawn_y;
extern SDL_Color backgroundColor;

int level[MAX_HEIGHT][MAX_WIDTH];
int level_width_tiles = 0;
std::string current_tileset = "overworld"; // fallback default

int get_level_width_tiles() {
    return level_width_tiles;
}

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

    level_width_tiles = 0; // Reset before loading new level

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
            if (token == "mm" || (token.length() > 1 && token.back() == 'M')) {
                std::string numPart = token.substr(0, token.length() - 1);
                int value = 0;

                if (numPart.find_first_not_of("0123456789") == std::string::npos) {
                    std::istringstream(numPart) >> value; // Decimal
                } else {
                    std::istringstream(numPart) >> std::hex >> value; // Hex
                }

                level[y][x] = value;

                if (!marioSpawnSet) {
                    marioSpawnSet = true;
                    mario_x = mario_spawn_x = x * TILE_WIDTH;
                    mario_y = mario_spawn_y = y * TILE_HEIGHT;
                }
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

        // Track the widest row
        if (x > level_width_tiles) {
            level_width_tiles = x;
        }

        ++y;
    }

    file.close();
    return true;
}

void unloadLevel() {
    // Clear all tile IDs in the level grid
    for (int y = 0; y < MAX_HEIGHT; ++y) {
        for (int x = 0; x < MAX_WIDTH; ++x) {
            level[y][x] = 0;
        }
    }

    // Reset level width
    level_width_tiles = 0;
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
    if (current_tileset != "overworld") {
        if (tile_id == 38) {
            return true;
        }
    }

    return tile_id >= 1 && tile_id <= 29;
}
