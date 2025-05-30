#include <SDL.h>
#include <SDL_image.h>
#include <coreinit/debug.h>
#include <coreinit/title.h>
#include <padscore/kpad.h>
#include <sndcore2/core.h>
#include <sysapp/launch.h>
#include <whb/proc.h>
#include <map>

#include "input/CombinedInput.h"
#include "input/VPADInput.h"
#include "input/WPADInput.h"

#include "animation.hpp"
#include "render.hpp"
#include "util.hpp"
#include "level.hpp"
#include "config.hpp"
#include "font.hpp"

const int MARIO_WIDTH = 44;
const int MARIO_HEIGHT = 48;

const int GRAVITY = 1;
const int WALK_SPEED = 4;
const int RUN_SPEED = WALK_SPEED * 2;
const float ACCELERATION = 0.3f;

// Global Variables
int currentLevelIndex = 0;
float horizontal_speed = 0;
float vertical_speed = 0;
float mario_x = WINDOW_WIDTH / 2;
float mario_y = WINDOW_HEIGHT / 2;
float mario_spawn_x;
float mario_spawn_y;
float camera_x = 0;

// Global Booleans
bool quit = false;
bool paused = false;
bool grounded = false;
bool jump_cut = false;
bool mario_facing_right = true;

SDL_Window* main_window;
SDL_Renderer* main_renderer;
SDL_Event event;
BitmapFont font;

// Background Color
SDL_Color backgroundColor = { 0, 0, 0 };

// Textures
SDL_Texture* mario_texture = NULL;
AnimationPlayer animationPlayer;

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

SDL_Texture* load_texture(const char* path, SDL_Renderer* renderer) {
    SDL_RWops* rw = SDL_RWFromFile(path, "rb");
    if (!rw) {
        OSReport("SDL_RWFromFile failed: %s\n", SDL_GetError());
        return NULL;
    }

    SDL_Surface* surface = IMG_Load_RW(rw, 1);
    if (!surface) {
        OSReport("IMG_Load_RW failed: %s\n", IMG_GetError());
        return NULL;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!texture) {
        OSReport("SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
    }

    return texture;
}

int getDeltaTime() {
    static Uint32 lastTime = SDL_GetTicks();
    Uint32 currentTime = SDL_GetTicks();
    Uint32 delta = currentTime - lastTime;
    lastTime = currentTime;
    return delta;
}

std::vector<std::string> levelList = {
    "1-1", "1-1_1", "1-2", "1-2_1", "1-2_2", "1-3", "1-4",
    "2-1", "2-2", "2-3", "2-4",
    "3-1", "3-2", "3-3", "3-4",
    "4-1", "4-2", "4-3", "4-4",
    "5-1", "5-2", "5-3", "54",
    "6-1", "6-2", "6-3", "6-4",
    "7-1", "7-2", "7-3", "7-4",
    "8-1", "8-2", "8-3", "8-4",
};

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

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        OSReport("Failed to initialise SDL_image for PNG files: %s\n", IMG_GetError());
    }

    main_renderer = SDL_CreateRenderer(main_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!main_renderer) {
        OSReport("SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    mario_texture = load_texture("/vol/external01/smb/assets/mario_small.png", main_renderer);
    if (!mario_texture) {
        OSReport("Failed to load Mario texture\n");
    }
    font.load(main_renderer, "/vol/external01/smb/assets/font.png", 30, 30, 19);

    return EXIT_SUCCESS;
}

void shutdown() {
    if (mario_texture) {
        SDL_DestroyTexture(mario_texture);
        mario_texture = NULL;
    }

    SDL_DestroyRenderer(main_renderer);
    SDL_DestroyWindow(main_window);
    IMG_Quit();
    SDL_Quit();
}

void input(Input& input) {
    int direction = 0;
    int max_speed = WALK_SPEED;
    bool jump_pressed = ((input.data.buttons_d & Input::BUTTON_B) || (input.data.buttons_d & Input::BUTTON_A));

    bool right = input.data.buttons_h & Input::STICK_L_RIGHT;
    bool left  = input.data.buttons_h & Input::STICK_L_LEFT;
    bool down  = input.data.buttons_h & Input::STICK_L_DOWN;

    if (input.data.buttons_d & Input::BUTTON_PLUS) {
        paused = !paused;
        return;
    }

    if (input.data.buttons_d & Input::BUTTON_MINUS) {
        currentLevelIndex = (currentLevelIndex + 1) % levelList.size();
        std::string nextLevelPath = "/vol/external01/smb/levels/" + levelList[currentLevelIndex] + ".txt";

        unloadLevel();

        mario_x = mario_spawn_x = 0;
        mario_y = mario_spawn_y = 0;
        camera_x = 0;
        horizontal_speed = 0;
        vertical_speed = 0;
        grounded = false;
        jump_cut = false;
        paused = false;

        loadLevel(nextLevelPath);
    }

    // If grounded and down is held, cancel horizontal input
    if (!paused) {
        if (!(grounded && down)) {
            if (right && !left) direction = 1;
            else if (!right && left) direction = -1;
        }

        if ((input.data.buttons_h & Input::BUTTON_Y) || (input.data.buttons_h & Input::BUTTON_X))
            max_speed = RUN_SPEED;

        // Animations
        if (input.data.buttons_h & Input::STICK_L_DOWN) {
            animationPlayer.play(&crouchAnim);
        } else if (grounded) {
            if (horizontal_speed != 0) {
                animationPlayer.play(&runAnim);
            } else {
                animationPlayer.play(&idleAnim);
            }
        } else {
            animationPlayer.play(&jumpAnim);
        }

        if (horizontal_speed > 0) mario_facing_right = true;
        else if (horizontal_speed < 0) mario_facing_right = false;

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
        int new_x = mario_x + horizontal_speed;
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
        int new_y = mario_y + vertical_speed;

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
            vertical_speed = -21;
            grounded = false;
            jump_cut = false;
        }

        // Short jump
        bool jump_held = ((input.data.buttons_h & Input::BUTTON_B) || (input.data.buttons_h & Input::BUTTON_A));
        if (!jump_held && vertical_speed < 0 && !grounded && !jump_cut) {
            vertical_speed *= 0.5f;
            jump_cut = true;
        }

        if (mario_y > WINDOW_HEIGHT + MARIO_HEIGHT) {
            // Handle falling off screen (e.g. reset level or death)
        }
    }

    switch (event.type) {
        case SDL_QUIT:
            quit = true;
            break;
    }
}

void update() {
    float maxSpeed = 5.0f;
    float speed = fabsf(horizontal_speed);
    float speedMultiplier = ((speed / maxSpeed) * 1.5f);
    if (speedMultiplier < 0.1f) speedMultiplier = 0.1f;

    render_set_color(main_renderer, backgroundColor);
    SDL_RenderClear(main_renderer);

    render_level(main_renderer, get_current_tileset(), camera_x);

    SDL_Rect dst = { (int)roundf(mario_x - camera_x), (int)roundf(mario_y), 48, 48 };
    SDL_Rect src = animationPlayer.currentFrame();
    SDL_RendererFlip flip = mario_facing_right ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
    SDL_RenderCopyEx(main_renderer, mario_texture, &src, &dst, 0.0, NULL, flip);

    // Center the camera on Mario (X axis only)
    camera_x = mario_x - WINDOW_WIDTH / 2;

    // Clamp to level bounds
    int level_width_pixels = get_level_width_tiles() * TILE_WIDTH;
    if (camera_x < 0) camera_x = 0;
    if (camera_x > level_width_pixels - WINDOW_WIDTH)
        camera_x = level_width_pixels - WINDOW_WIDTH;

    // You can clamp to the max level width too, e.g.
    // if (camera_x > LEVEL_WIDTH_PIXELS - WINDOW_WIDTH) camera_x = LEVEL_WIDTH_PIXELS - WINDOW_WIDTH;
    if (camera_x < 0) camera_x = 0;

    std::string displayName = levelList[currentLevelIndex];
    size_t underscorePos = displayName.find('_');
    if (underscorePos != std::string::npos) {
        displayName = displayName.substr(0, underscorePos);
    }

    int delta = getDeltaTime();
    animationPlayer.update(delta, paused ? 0.0f : speedMultiplier);

    font.renderText(main_renderer, "MARIO", 50, 25, TextAlign::LEFT, -4);
    font.renderText(main_renderer, "000000", 50, 50, TextAlign::LEFT, -4); // Replace with a score that can actually be updated
    font.renderText(main_renderer, "00", ((WINDOW_WIDTH / 2) - 200), 50, TextAlign::LEFT, -4); // Replace with a coin count that can actually be updated
    font.renderText(main_renderer, "WORLD", ((WINDOW_WIDTH / 2) + 200), 25, TextAlign::LEFT, -4);
    font.renderText(main_renderer, displayName.c_str(), ((WINDOW_WIDTH / 2) + 265), 50, TextAlign::CENTER, -4);
    font.renderText(main_renderer, "TIME", 1230, 25, TextAlign::RIGHT, -4);
    font.renderText(main_renderer, "400", 1190, 50, TextAlign::CENTER, -4);

    if (paused) {
        SDL_SetRenderDrawBlendMode(main_renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(main_renderer, 0, 0, 0, 128);
        SDL_Rect overlay = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
        SDL_RenderFillRect(main_renderer, &overlay);

        font.renderText(main_renderer, "PAUSED", WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, TextAlign::CENTER, 0);
    }

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
    load_tilesets(main_renderer);
    loadLevel("/vol/external01/smb/levels/1-1.txt");

    AXInit();
    AXQuit();

    KPADInit();
    WPADEnableURCC(TRUE);

    loadAnimations();

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
