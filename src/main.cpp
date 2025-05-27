#include <coreinit/debug.h>
#include <coreinit/title.h>
#include <padscore/kpad.h>
#include <sndcore2/core.h>
#include <sysapp/launch.h>
#include <whb/proc.h>
#include <SDL.h>

#include "input/CombinedInput.h"
#include "input/VPADInput.h"
#include "input/WPADInput.h"

#include "render.hpp"
#include "util.hpp"
#include "level.hpp"
#include "render.hpp"

// Global data
SDL_Window *main_window;
SDL_Renderer *main_renderer;
SDL_Event event;

// Constants
const int WINDOW_WIDTH = 256;
const int WINDOW_HEIGHT = 240;

const float ACCELERATION = 0.1;
const float GRAVITY = 0.5;
const float WALK_SPEED = 2.5;
const int RUN_SPEED = WALK_SPEED * 2;
const int MARIO_WIDTH = 16;
const int MARIO_HEIGHT = 16;

// Global Player Variables
float horizontal_speed = 0.0;
float vertical_speed = 0.0;
int mario_x = WINDOW_WIDTH / 2.0;
int mario_y = WINDOW_HEIGHT / 2.0;

// Global control
bool quit = false;

// Global SDL Rects
SDL_Rect mario_rect = { mario_x, mario_y, 16, 16 };
SDL_Rect wall_rect = { 300, 350, 100, 32 };  // Example wall/platform

// Setup for The Game Loop
int initialise() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        OSReport("SDL_Init failed with error: ", SDL_GetError(), "\n");
        return EXIT_FAILURE;
    }

    // Handle window creation
    main_window = SDL_CreateWindow(
        "Super Mario Bros.",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        0);

    if (main_window == nullptr) {
        OSReport("SDL_CreateWindow failed with error: ", SDL_GetError(), "\n");
        SDL_Quit();
        return EXIT_FAILURE;
    }

    // Handle renderer creation
    main_renderer = SDL_CreateRenderer(main_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    return EXIT_SUCCESS;
}

// Clear out File for Shutdown
void shutdown() {
    SDL_DestroyWindow(main_window);
    SDL_DestroyRenderer(main_renderer);
    SDL_Quit();
}

// Player Physics (Input) Script
void input(Input &input) {
    int direction = 0;
    int max_speed = WALK_SPEED;
    bool grounded = false;

    bool right = (input.data.buttons_h & Input::STICK_L_RIGHT);
    bool left  = (input.data.buttons_h & Input::STICK_L_LEFT);
    bool jump_pressed = (input.data.buttons_d & Input::BUTTON_B);

    if (right && !left) {
        direction = 1;
    } else if (!right && left) {
        direction = -1;
    }

    // Running modifier
    if ((input.data.buttons_h & Input::BUTTON_Y) || (input.data.buttons_h & Input::BUTTON_X)) {
        max_speed = RUN_SPEED;
    }

    // Horizontal movement
    if (direction == 1) {
        horizontal_speed += ACCELERATION;
        if (horizontal_speed > max_speed) horizontal_speed = max_speed;
    } else if (direction == -1) {
        horizontal_speed -= ACCELERATION;
        if (horizontal_speed < -max_speed) horizontal_speed = -max_speed;
    } else {
        // Decelerate to stop
        if (horizontal_speed > 0) {
            horizontal_speed -= ACCELERATION;
            if (horizontal_speed < 0) horizontal_speed = 0;
        } else if (horizontal_speed < 0) {
            horizontal_speed += ACCELERATION;
            if (horizontal_speed > 0) horizontal_speed = 0;
        }
    }

    // Try moving horizontally
    SDL_Rect test_rect_x = makeSDLRectFromFloat(mario_x + horizontal_speed, mario_y, MARIO_WIDTH, MARIO_HEIGHT);
    if (!is_solid_at(mario_x + horizontal_speed, mario_y + MARIO_HEIGHT / 2)) {
        mario_x += horizontal_speed;
    } else {
        horizontal_speed = 0;
    }

    // Apply gravity
    vertical_speed += GRAVITY;

    if (!is_solid_at(mario_x + MARIO_WIDTH / 2, mario_y + vertical_speed + MARIO_HEIGHT)) {
        mario_y += vertical_speed;
        grounded = false;
    } else {
        if (vertical_speed > 0) {
            mario_y = ((int)(mario_y + vertical_speed + MARIO_HEIGHT) / TILE_HEIGHT) * TILE_HEIGHT - MARIO_HEIGHT;
            grounded = true;
        } else if (vertical_speed < 0) {
            mario_y = ((int)(mario_y + vertical_speed) / TILE_HEIGHT + 1) * TILE_HEIGHT;
        }
        vertical_speed = 0;
    }

    // Jumping
    if (jump_pressed && grounded) {
        vertical_speed = -8; // jump strength
    }

    // Fall off screen reset
    if (mario_y > WINDOW_HEIGHT + 64) {
        mario_y = 0;
        vertical_speed = 0;
    }

    // Quit event
    switch (event.type) {
        case SDL_QUIT:
            quit = true;
            break;
        default:
            break;
    }
}

// Update Function
void update() {
    // Background
    render_set_color(main_renderer, BACKGROUND_OVERWORLD);
    SDL_RenderClear(main_renderer);

    // Spawn area
    render_set_color(main_renderer, COLOR_CURSOR);
    render_rectangle(main_renderer,
                    mario_x, mario_y, 16, 16, true);

    // Draw Level
    render_level(main_renderer);

    SDL_Rect mario_rect = makeSDLRectFromFloat(mario_x, mario_y, MARIO_WIDTH, MARIO_HEIGHT);
    render_set_color(main_renderer, COLOR_CURSOR);
    render_rectangle(main_renderer, mario_rect.x, mario_rect.y, mario_rect.w, mario_rect.h, true);

    // Commit render
    SDL_RenderPresent(main_renderer);
}

inline bool RunningFromMiiMaker() {
    return (OSGetTitleID() & 0xFFFFFFFFFFFFF0FFull) == 0x000500101004A000ull;
}

// Main Function
int main(int argc, char const *argv[]) {
    if (initialise() != EXIT_SUCCESS) {
        shutdown();
    }

    WHBProcInit();
    loadLevel("/vol/external01/smb/levels/level1.txt");

    // Call AXInit to stop already playing sounds
    AXInit();
    AXQuit();

    KPADInit();
    WPADEnableURCC(TRUE);

    CombinedInput baseInput;
    VPadInput vpadInput;
    WPADInput wpadInputs[4] = {
            WPAD_CHAN_0,
            WPAD_CHAN_1,
            WPAD_CHAN_2,
            WPAD_CHAN_3};

    while (WHBProcIsRunning()) {
        baseInput.reset();
        if (vpadInput.update(1280, 720)) {
            baseInput.combine(vpadInput);
        }
        for (auto &wpadInput : wpadInputs) {
            if (wpadInput.update(1280, 720)) {
                baseInput.combine(wpadInput);
            }
        }
        baseInput.process();

        input(baseInput);

        if (quit) {
           if (RunningFromMiiMaker()) {
                // Legacy way, just quit
                break;
            } else {
                // Launch menu otherwise
                SYSLaunchMenu();
            }
        }
        update();
    }

    shutdown();

    WHBProcShutdown();
    return EXIT_SUCCESS;
}
