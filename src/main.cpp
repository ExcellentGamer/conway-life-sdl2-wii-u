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

// Constants
const int WINDOW_WIDTH = 256;
const int WINDOW_HEIGHT = 240;
const float ACCELERATION = 0.1f;
const float GRAVITY = 0.5f;
const float WALK_SPEED = 2.5f;
const float RUN_SPEED = WALK_SPEED * 2;
const int MARIO_WIDTH = 16;
const int MARIO_HEIGHT = 16;

// Global Variables
SDL_Window* main_window;
SDL_Renderer* main_renderer;
SDL_Event event;

float horizontal_speed = 0.0f;
float vertical_speed = 0.0f;
float mario_x = WINDOW_WIDTH / 2.0f;
float mario_y = WINDOW_HEIGHT / 2.0f;
bool quit = false;
bool grounded = false;
bool jump_cut = false;

SDL_Rect mario_rect = { (int)mario_x, (int)mario_y, MARIO_WIDTH, MARIO_HEIGHT };

// Helpers
bool is_solid_box(float x, float y, int w, int h) {
    const int step = 4;

    for (int dy = 0; dy <= h; dy += step) {
        for (int dx = 0; dx <= w; dx += step) {
            float sample_x = x + ((dx >= w) ? (w - 1) : dx);
            float sample_y = y + ((dy >= h) ? (h - 1) : dy);
            if (is_solid_at(sample_x, sample_y)) return true;
        }
    }
    return false;
}

int initialise() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        OSReport("SDL_Init failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    main_window = SDL_CreateWindow("Super Mario Bros.",
                                   SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                   WINDOW_WIDTH, WINDOW_HEIGHT, 0);

    if (!main_window) {
        OSReport("SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    main_renderer = SDL_CreateRenderer(main_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    return EXIT_SUCCESS;
}

void shutdown() {
    SDL_DestroyRenderer(main_renderer);
    SDL_DestroyWindow(main_window);
    SDL_Quit();
}

void input(Input& input) {
    int direction = 0;
    float max_speed = WALK_SPEED;
    bool jump_pressed = (input.data.buttons_d & Input::BUTTON_B);

    bool right = input.data.buttons_h & Input::STICK_L_RIGHT;
    bool left  = input.data.buttons_h & Input::STICK_L_LEFT;

    if (right && !left) direction = 1;
    else if (!right && left) direction = -1;

    if ((input.data.buttons_h & Input::BUTTON_Y) || (input.data.buttons_h & Input::BUTTON_X))
        max_speed = RUN_SPEED;

    // Horizontal movement
    if (direction == 1) {
        horizontal_speed += ACCELERATION;
        if (horizontal_speed > max_speed) horizontal_speed = max_speed;
    } else if (direction == -1) {
        horizontal_speed -= ACCELERATION;
        if (horizontal_speed < -max_speed) horizontal_speed = -max_speed;
    } else {
        if (horizontal_speed > 0) {
            horizontal_speed -= ACCELERATION;
            if (horizontal_speed < 0) horizontal_speed = 0;
        } else if (horizontal_speed < 0) {
            horizontal_speed += ACCELERATION;
            if (horizontal_speed > 0) horizontal_speed = 0;
        }
    }

    // Horizontal collision
    float new_x = mario_x + horizontal_speed;
    if (!is_solid_box(new_x, mario_y, MARIO_WIDTH, MARIO_HEIGHT)) {
        mario_x = new_x;
    } else {
        while (!is_solid_box(mario_x + (horizontal_speed > 0 ? 1 : -1), mario_y, MARIO_WIDTH, MARIO_HEIGHT)) {
            mario_x += (horizontal_speed > 0 ? 1 : -1);
        }
        horizontal_speed = 0;
    }

    // Apply gravity
    vertical_speed += GRAVITY;
    float new_y = mario_y + vertical_speed;

    if (!is_solid_box(mario_x, new_y, MARIO_WIDTH, MARIO_HEIGHT)) {
        mario_y = new_y;
        grounded = false;
    } else {
        while (!is_solid_box(mario_x, mario_y + (vertical_speed > 0 ? 1 : -1), MARIO_WIDTH, MARIO_HEIGHT)) {
            mario_y += (vertical_speed > 0 ? 1 : -1);
        }

        if (vertical_speed > 0) grounded = true;
        if (grounded) jump_cut = false;
        vertical_speed = 0;
    }

    // Start jump
    if (jump_pressed && grounded) {
        vertical_speed = -8.5f;
        grounded = false;
        jump_cut = false;
    }

    // Short jump if button is released early
    bool jump_held = (input.data.buttons_h & Input::BUTTON_B);
    if (!jump_held && vertical_speed < 0 && !grounded && !jump_cut) {
        vertical_speed *= 0.5f; // cut upward speed
        jump_cut = true;
    }

    if (mario_y > WINDOW_HEIGHT + 64) {
        mario_y = 0;
        vertical_speed = 0;
    }

    switch (event.type) {
        case SDL_QUIT:
            quit = true;
            break;
    }
}

void update() {
    render_set_color(main_renderer, BACKGROUND_OVERWORLD);
    SDL_RenderClear(main_renderer);

    render_set_color(main_renderer, COLOR_CURSOR);
    render_rectangle(main_renderer, (int)roundf(mario_x), (int)roundf(mario_y), MARIO_WIDTH, MARIO_HEIGHT, true);

    render_level(main_renderer);

    SDL_RenderPresent(main_renderer);
}

inline bool RunningFromMiiMaker() {
    return (OSGetTitleID() & 0xFFFFFFFFFFFFF0FFull) == 0x000500101004A000ull;
}

int main(int argc, char const* argv[]) {
    if (initialise() != EXIT_SUCCESS) {
        shutdown();
        return EXIT_FAILURE;
    }

    WHBProcInit();
    loadLevel("/vol/external01/smb/levels/level1.txt");

    AXInit();
    AXQuit();

    KPADInit();
    WPADEnableURCC(TRUE);

    CombinedInput baseInput;
    VPadInput vpadInput;
    WPADInput wpadInputs[4] = {
        WPAD_CHAN_0, WPAD_CHAN_1, WPAD_CHAN_2, WPAD_CHAN_3
    };

    while (WHBProcIsRunning()) {
        baseInput.reset();
        if (vpadInput.update(1280, 720)) baseInput.combine(vpadInput);
        for (auto& wpadInput : wpadInputs) {
            if (wpadInput.update(1280, 720)) baseInput.combine(wpadInput);
        }
        baseInput.process();

        while (SDL_PollEvent(&event)) {}
        input(baseInput);

        if (quit) {
            if (RunningFromMiiMaker()) break;
            else SYSLaunchMenu();
        }

        update();
    }

    shutdown();
    WHBProcShutdown();
    return EXIT_SUCCESS;
}
